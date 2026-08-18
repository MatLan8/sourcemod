#if defined _mapdatadebug_voxels_included
    #endinput
#endif

#define _mapdatadebug_voxels_included

Action Command_VoxelTest(int client)
{
    int target = ResolveMapDataTarget(client);

    if (target == 0)
    {
        ReplyToCommand(client, "[MapDataDebug] No in-game player. Add a fake client or use this in-game.");
        return Plugin_Handled;
    }

    float origin[3];
    float angles[3];

    GetClientAbsOrigin(target, origin);
    GetClientEyeAngles(target, angles);

    StartNativeTimer();

    bool success = MapData_BuildVoxels(
        origin,
        angles,
        g_VoxelFlags,
        sizeof(g_VoxelFlags)
    );

    float elapsedMs = StopNativeTimerMs();

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
        "[MapDataDebug] voxels success=%d time=%.4fms occupied=%d/%d solid=%d clip=%d water=%d nogrenades=%d teleport=%d yaw=%.1f",
        success,
        elapsedMs,
        occupied,
        VOXEL_COUNT,
        solid,
        playerClip,
        water,
        noGrenades,
        teleport,
        angles[1]
    );

    ReplyToCommand(
        client,
        "[MapDataDebug] voxels %.4fms occupied %d / %d (solid %d clip %d water %d ng %d tp %d)",
        elapsedMs,
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
