#include <sourcemod>
#include <sdktools>
#include "MapData/mapdata.inc"

#define RAY_DRAW_LIFE 1.0
#define RAY_DRAW_WIDTH 0.4
#define RAY_DRAW_CYCLE 0.9
#define RAY_DRAW_BATCH 22
#define RAY_OFFSET_Z 32.0

int g_VoxelFlags[VOXEL_COUNT];
RayData g_Rays[RAY_COUNT];
bool g_RaysReady = false;

int g_BeamSprite = -1;
bool g_RayDrawEnabled = false;
int g_RayDrawClient = 0;
int g_RayDrawCycleStartTick = 0;

float g_RayStart[RAY_COUNT][3];
float g_RayEnd[RAY_COUNT][3];

static void CacheStaticProps()
{
    int count = MapData_CacheStaticProps();

    PrintToServer("[VoxelTest] Cached %d static props", count);
}

static void CacheTriggers()
{
    int count = MapData_CacheTriggers();

    PrintToServer("[VoxelTest] Cached %d triggers", count);
}

static void CacheMapGeometry()
{
    CacheStaticProps();
    CacheTriggers();
}

static void PrintRayLine(int client, int index)
{
    char label[8];

    if (index == RAY_LOOK_INDEX)
    {
        strcopy(label, sizeof(label), "look");
    }
    else
    {
        IntToString(index, label, sizeof(label));
    }

    char line[256];
    Format(
        line,
        sizeof(line),
        "[VoxelTest] ray %s flags=%d geom=%.1f n=(%.2f %.2f %.2f) ng=%.1f tp=%.1f",
        label,
        g_Rays[index].flags,
        g_Rays[index].geomDistance,
        g_Rays[index].geomNormal[0],
        g_Rays[index].geomNormal[1],
        g_Rays[index].geomNormal[2],
        g_Rays[index].noGrenadesDistance,
        g_Rays[index].teleportDistance
    );

    PrintToServer("%s", line);

    if (client > 0 && IsClientInGame(client))
    {
        PrintToChat(client, "%s", line);
        PrintToConsole(client, "%s", line);
    }
}

static void FibonacciWorldDir(int index, float yawDegrees, float dir[3])
{
    float y = 1.0 - ((float(index) + 0.5) / float(RAY_COUNT_OMNI)) * 2.0;
    float radiusSq = 1.0 - y * y;
    float radius = (radiusSq > 0.0) ? SquareRoot(radiusSq) : 0.0;
    float theta = float(index) * FLOAT_PI * (3.0 - SquareRoot(5.0));

    float lx = Cosine(theta) * radius;
    float ly = Sine(theta) * radius;
    float lz = y;

    float yaw = DegToRad(yawDegrees);
    float cy = Cosine(yaw);
    float sy = Sine(yaw);

    dir[0] = sy * lx + cy * ly;
    dir[1] = -cy * lx + sy * ly;
    dir[2] = lz;
}

static void CaptureRaySnapshot(
    const float origin[3],
    const float angles[3],
    const float eyeOrigin[3]
)
{
    float omniOrigin[3];
    omniOrigin[0] = origin[0];
    omniOrigin[1] = origin[1];
    omniOrigin[2] = origin[2] + RAY_OFFSET_Z;

    for (int i = 0; i < RAY_COUNT_OMNI; i++)
    {
        float dir[3];
        FibonacciWorldDir(i, angles[1], dir);

        g_RayStart[i][0] = omniOrigin[0];
        g_RayStart[i][1] = omniOrigin[1];
        g_RayStart[i][2] = omniOrigin[2];

        float length = g_Rays[i].geomDistance;

        g_RayEnd[i][0] = omniOrigin[0] + dir[0] * length;
        g_RayEnd[i][1] = omniOrigin[1] + dir[1] * length;
        g_RayEnd[i][2] = omniOrigin[2] + dir[2] * length;
    }

    float lookDir[3];
    GetAngleVectors(angles, lookDir, NULL_VECTOR, NULL_VECTOR);

    g_RayStart[RAY_LOOK_INDEX][0] = eyeOrigin[0];
    g_RayStart[RAY_LOOK_INDEX][1] = eyeOrigin[1];
    g_RayStart[RAY_LOOK_INDEX][2] = eyeOrigin[2];

    float lookLength = g_Rays[RAY_LOOK_INDEX].geomDistance;

    g_RayEnd[RAY_LOOK_INDEX][0] = eyeOrigin[0] + lookDir[0] * lookLength;
    g_RayEnd[RAY_LOOK_INDEX][1] = eyeOrigin[1] + lookDir[1] * lookLength;
    g_RayEnd[RAY_LOOK_INDEX][2] = eyeOrigin[2] + lookDir[2] * lookLength;
}

static void DrawRayBeam(int client, int index)
{
    int color[4];

    if (index == RAY_LOOK_INDEX)
    {
        color[0] = 0;
        color[1] = 0;
        color[2] = 255;
        color[3] = 255;
    }
    else
    {
        color[0] = 0;
        color[1] = 255;
        color[2] = 0;
        color[3] = 255;
    }

    TE_SetupBeamPoints(
        g_RayStart[index],
        g_RayEnd[index],
        g_BeamSprite,
        0,
        0,
        0,
        RAY_DRAW_LIFE,
        RAY_DRAW_WIDTH,
        RAY_DRAW_WIDTH,
        0,
        0.0,
        color,
        0
    );

    TE_SendToClient(client);
}

static void DrawRayBatch(int client, int startIndex, int count)
{
    int endIndex = startIndex + count;

    if (endIndex > RAY_COUNT)
    {
        endIndex = RAY_COUNT;
    }

    for (int i = startIndex; i < endIndex; i++)
    {
        DrawRayBeam(client, i);
    }
}

public void OnPluginStart()
{
    RegConsoleCmd("sm_voxeltest", Command_VoxelTest);
    RegConsoleCmd("sm_raytest", Command_RayTest);
    RegConsoleCmd("sm_printrays", Command_PrintRays);
    RegConsoleCmd("sm_raysdraw", Command_RaysDraw);
    HookEvent("teamplay_round_start", Event_OnRoundStart, EventHookMode_PostNoCopy);
    PrintToServer("[VoxelTest] Loaded. Use sm_voxeltest, sm_raytest, sm_printrays, sm_raysdraw.");
    // Map geometry is cached in OnMapStart. The map is not loaded here.
}

public void OnMapStart()
{
    g_RaysReady = false;
    g_RayDrawEnabled = false;
    g_BeamSprite = PrecacheModel("materials/sprites/laserbeam.vmt");
    CacheMapGeometry();
}

public void Event_OnRoundStart(Event event, const char[] name, bool dontBroadcast)
{
    CacheTriggers();
}

public void OnGameFrame()
{
    if (!g_RayDrawEnabled)
    {
        return;
    }

    int client = g_RayDrawClient;

    if (client <= 0 || !IsClientInGame(client) || g_BeamSprite <= 0)
    {
        g_RayDrawEnabled = false;
        return;
    }

    int cycleTicks = RoundToNearest(RAY_DRAW_CYCLE / GetTickInterval());

    if (cycleTicks < 4)
    {
        cycleTicks = 4;
    }

    int phase = (GetGameTickCount() - g_RayDrawCycleStartTick) % cycleTicks;

    if (phase == 0)
    {
        DrawRayBatch(client, 0, RAY_DRAW_BATCH);
    }
    else if (phase == 1)
    {
        DrawRayBatch(client, RAY_DRAW_BATCH, RAY_DRAW_BATCH);
    }
    else if (phase == 2)
    {
        DrawRayBatch(client, RAY_DRAW_BATCH * 2, RAY_COUNT - RAY_DRAW_BATCH * 2);
    }
}

public Action Command_VoxelTest(int client, int args)
{
    if (client <= 0 || !IsClientInGame(client))
    {
        ReplyToCommand(client, "[VoxelTest] Must be used in-game.");
        return Plugin_Handled;
    }

    float origin[3];
    float angles[3];

    GetClientAbsOrigin(client, origin);
    GetClientEyeAngles(client, angles);

    bool success = MapData_BuildVoxels(
        origin,
        angles,
        g_VoxelFlags,
        sizeof(g_VoxelFlags)
    );

    int occupied = 0;
    int solid = 0;
    int playerClip = 0;
    int water = 0;
    int noGrenades = 0;
    int teleport = 0;

    for (int i = 0; i < VOXEL_COUNT; i++)
    {
        int flags = g_VoxelFlags[i];

        if (flags == 0)
        {
            continue;
        }

        occupied++;

        if (flags & GEOM_SOLID)
        {
            solid++;
        }

        if (flags & GEOM_PLAYERCLIP)
        {
            playerClip++;
        }

        if (flags & GEOM_WATER)
        {
            water++;
        }

        if (flags & GEOM_NOGRENADES)
        {
            noGrenades++;
        }

        if (flags & GEOM_TELEPORT)
        {
            teleport++;
        }
    }

    PrintToServer(
        "[VoxelTest] success=%d occupied=%d/%d solid=%d clip=%d water=%d nogrenades=%d teleport=%d yaw=%.1f",
        success,
        occupied,
        VOXEL_COUNT,
        solid,
        playerClip,
        water,
        noGrenades,
        teleport,
        angles[1]
    );

    PrintToChat(
        client,
        "[VoxelTest] occupied %d / %d (solid %d clip %d water %d ng %d tp %d)",
        occupied,
        VOXEL_COUNT,
        solid,
        playerClip,
        water,
        noGrenades,
        teleport
    );

    return Plugin_Handled;
}

public Action Command_RayTest(int client, int args)
{
    if (client <= 0 || !IsClientInGame(client))
    {
        ReplyToCommand(client, "[VoxelTest] Must be used in-game.");
        return Plugin_Handled;
    }

    float origin[3];
    float angles[3];
    float eyeOrigin[3];

    GetClientAbsOrigin(client, origin);
    GetClientEyeAngles(client, angles);
    GetClientEyePosition(client, eyeOrigin);

    float startTime = GetEngineTime();

    bool success = MapData_TraceRays(
        origin,
        angles,
        eyeOrigin,
        g_Rays,
        sizeof(g_Rays) * sizeof(g_Rays[0])
    );

    float elapsedMs = (GetEngineTime() - startTime) * 1000.0;

    g_RaysReady = success;

    if (success)
    {
        CaptureRaySnapshot(origin, angles, eyeOrigin);
    }

    int geomHits = 0;
    int clipHits = 0;
    int waterHits = 0;
    int noGrenadeHits = 0;
    int teleportHits = 0;
    float minGeom = RAY_MAX_RANGE;

    for (int i = 0; i < RAY_COUNT; i++)
    {
        int flags = g_Rays[i].flags;

        if (flags & GEOM_SOLID)
        {
            geomHits++;
        }

        if (flags & GEOM_PLAYERCLIP)
        {
            clipHits++;
        }

        if (flags & GEOM_WATER)
        {
            waterHits++;
        }

        if (flags & GEOM_NOGRENADES)
        {
            noGrenadeHits++;
        }

        if (flags & GEOM_TELEPORT)
        {
            teleportHits++;
        }

        if (g_Rays[i].geomDistance < minGeom)
        {
            minGeom = g_Rays[i].geomDistance;
        }
    }

    PrintToServer(
        "[VoxelTest] rays success=%d time=%.3fms geom=%d clip=%d water=%d ng=%d tp=%d minGeom=%.1f look flags=%d geom=%.1f ng=%.1f tp=%.1f",
        success,
        elapsedMs,
        geomHits,
        clipHits,
        waterHits,
        noGrenadeHits,
        teleportHits,
        minGeom,
        g_Rays[RAY_LOOK_INDEX].flags,
        g_Rays[RAY_LOOK_INDEX].geomDistance,
        g_Rays[RAY_LOOK_INDEX].noGrenadesDistance,
        g_Rays[RAY_LOOK_INDEX].teleportDistance
    );

    PrintToChat(
        client,
        "[VoxelTest] rays %dms geom %d clip %d water %d ng %d tp %d min %.0f look g=%.0f ng=%.0f tp=%.0f",
        RoundToNearest(elapsedMs),
        geomHits,
        clipHits,
        waterHits,
        noGrenadeHits,
        teleportHits,
        minGeom,
        g_Rays[RAY_LOOK_INDEX].geomDistance,
        g_Rays[RAY_LOOK_INDEX].noGrenadesDistance,
        g_Rays[RAY_LOOK_INDEX].teleportDistance
    );

    return Plugin_Handled;
}

public Action Command_PrintRays(int client, int args)
{
    if (!g_RaysReady)
    {
        ReplyToCommand(client, "[VoxelTest] No ray data. Use sm_raytest first.");
        return Plugin_Handled;
    }

    for (int i = 0; i < RAY_COUNT; i++)
    {
        PrintRayLine(client, i);
    }

    return Plugin_Handled;
}

public Action Command_RaysDraw(int client, int args)
{
    if (client <= 0 || !IsClientInGame(client))
    {
        ReplyToCommand(client, "[VoxelTest] Must be used in-game.");
        return Plugin_Handled;
    }

    if (g_RayDrawEnabled && g_RayDrawClient == client)
    {
        g_RayDrawEnabled = false;
        PrintToChat(client, "[VoxelTest] Ray draw off.");
        return Plugin_Handled;
    }

    if (!g_RaysReady)
    {
        ReplyToCommand(client, "[VoxelTest] No ray data. Use sm_raytest first.");
        return Plugin_Handled;
    }

    g_RayDrawClient = client;
    g_RayDrawEnabled = true;
    g_RayDrawCycleStartTick = GetGameTickCount();
    PrintToChat(client, "[VoxelTest] Ray draw on. Frozen at last sm_raytest.");
    return Plugin_Handled;
}
