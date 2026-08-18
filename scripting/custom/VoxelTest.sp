#include <sourcemod>
#include <sdktools>
#include "MapData/mapdata.inc"

int g_VoxelFlags[VOXEL_COUNT];
RayData g_Rays[RAY_COUNT];
bool g_RaysReady = false;

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

public void OnPluginStart()
{
    RegConsoleCmd("sm_voxeltest", Command_VoxelTest);
    RegConsoleCmd("sm_raytest", Command_RayTest);
    RegConsoleCmd("sm_printrays", Command_PrintRays);
    HookEvent("teamplay_round_start", Event_OnRoundStart, EventHookMode_PostNoCopy);
    PrintToServer("[VoxelTest] Loaded. Use sm_voxeltest, sm_raytest, sm_printrays.");
    // Map geometry is cached in OnMapStart. The map is not loaded here.
}

public void OnMapStart()
{
    g_RaysReady = false;
    CacheMapGeometry();
}

public void Event_OnRoundStart(Event event, const char[] name, bool dontBroadcast)
{
    CacheTriggers();
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
