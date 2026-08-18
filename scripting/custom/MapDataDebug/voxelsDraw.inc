#if defined _mapdatadebug_voxelsdraw_included
    #endinput
#endif

#define _mapdatadebug_voxelsdraw_included

float g_VoxelDrawCenter[3];
float g_VoxelDrawRight[3];
float g_VoxelDrawForward[3];
float g_VoxelDrawUp[3];

void VoxelLocalToWorld(float localX, float localY, float localZ, float world[3])
{
    world[0] =
        g_VoxelDrawCenter[0]
        + g_VoxelDrawRight[0] * localX
        + g_VoxelDrawForward[0] * localY
        + g_VoxelDrawUp[0] * localZ;

    world[1] =
        g_VoxelDrawCenter[1]
        + g_VoxelDrawRight[1] * localX
        + g_VoxelDrawForward[1] * localY
        + g_VoxelDrawUp[1] * localZ;

    world[2] =
        g_VoxelDrawCenter[2]
        + g_VoxelDrawRight[2] * localX
        + g_VoxelDrawForward[2] * localY
        + g_VoxelDrawUp[2] * localZ;
}

void DrawVoxelWorldLine(const float start[3], const float end[3])
{
    TE_SetupBeamPoints(
        start,
        end,
        g_BeamSprite,
        0,
        0,
        0,
        VOXEL_BEAM_LIFE,
        VOXEL_BEAM_WIDTH,
        VOXEL_BEAM_WIDTH,
        0,
        0.0,
        {0, 255, 0, 255},
        0
    );

    TE_SendToAll();
}

void DrawVoxelOrientedBox(
    float minX,
    float minY,
    float minZ,
    float maxX,
    float maxY,
    float maxZ
)
{
    float c000[3];
    float c100[3];
    float c110[3];
    float c010[3];
    float c001[3];
    float c101[3];
    float c111[3];
    float c011[3];

    VoxelLocalToWorld(minX, minY, minZ, c000);
    VoxelLocalToWorld(maxX, minY, minZ, c100);
    VoxelLocalToWorld(maxX, maxY, minZ, c110);
    VoxelLocalToWorld(minX, maxY, minZ, c010);
    VoxelLocalToWorld(minX, minY, maxZ, c001);
    VoxelLocalToWorld(maxX, minY, maxZ, c101);
    VoxelLocalToWorld(maxX, maxY, maxZ, c111);
    VoxelLocalToWorld(minX, maxY, maxZ, c011);

    DrawVoxelWorldLine(c000, c100);
    DrawVoxelWorldLine(c100, c110);
    DrawVoxelWorldLine(c110, c010);
    DrawVoxelWorldLine(c010, c000);

    DrawVoxelWorldLine(c001, c101);
    DrawVoxelWorldLine(c101, c111);
    DrawVoxelWorldLine(c111, c011);
    DrawVoxelWorldLine(c011, c001);

    DrawVoxelWorldLine(c000, c001);
    DrawVoxelWorldLine(c100, c101);
    DrawVoxelWorldLine(c110, c111);
    DrawVoxelWorldLine(c010, c011);
}

void DrawVoxelOuterBox(int client)
{
    float origin[3];
    float angles[3];

    GetClientAbsOrigin(client, origin);
    GetClientEyeAngles(client, angles);

    g_VoxelDrawCenter[0] = origin[0];
    g_VoxelDrawCenter[1] = origin[1];
    g_VoxelDrawCenter[2] = origin[2] + VOXEL_BOX_OFFSET_Z;

    float yaw = DegToRad(angles[1]);
    float cy = Cosine(yaw);
    float sy = Sine(yaw);

    g_VoxelDrawForward[0] = cy;
    g_VoxelDrawForward[1] = sy;
    g_VoxelDrawForward[2] = 0.0;

    g_VoxelDrawRight[0] = sy;
    g_VoxelDrawRight[1] = -cy;
    g_VoxelDrawRight[2] = 0.0;

    g_VoxelDrawUp[0] = 0.0;
    g_VoxelDrawUp[1] = 0.0;
    g_VoxelDrawUp[2] = 1.0;

    float half = VOXEL_BOX_SIZE / 2.0;

    DrawVoxelOrientedBox(
        -half,
        -half,
        -half,
        half,
        half,
        half
    );
}

void TickVoxelDraw()
{
    if (!g_VoxelDrawEnabled)
    {
        return;
    }

    int client = g_VoxelDrawClient;

    if (client <= 0 || !IsClientInGame(client) || g_BeamSprite <= 0)
    {
        g_VoxelDrawEnabled = false;
        return;
    }

    g_VoxelDrawTick++;

    if (g_VoxelDrawTick < VOXEL_DRAW_TICKS)
    {
        return;
    }

    g_VoxelDrawTick = 0;
    DrawVoxelOuterBox(client);
}

Action Command_VoxelsDraw(int client)
{
    if (client <= 0 || !IsClientInGame(client))
    {
        ReplyToCommand(client, "[MapDataDebug] drawVoxels must be used in-game.");
        return Plugin_Handled;
    }

    if (g_VoxelDrawEnabled && g_VoxelDrawClient == client)
    {
        g_VoxelDrawEnabled = false;
        ReplyToCommand(client, "[MapDataDebug] Voxel box draw off.");
        return Plugin_Handled;
    }

    g_VoxelDrawClient = client;
    g_VoxelDrawEnabled = true;
    g_VoxelDrawTick = 0;
    ReplyToCommand(client, "[MapDataDebug] Voxel box draw on. 256 HU cube, +32 Z, yaw-aligned.");
    return Plugin_Handled;
}
