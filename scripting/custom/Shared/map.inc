#if defined _map_included
    #endinput
#endif

#define _map_included


void GatherMapData(int client)
{
    if (client == 0)
    {
        return;
    }

    float origin[3];
    float angles[3];
    float eyeOrigin[3];

    GetClientAbsOrigin(client, origin);
    GetClientEyeAngles(client, angles);
    GetClientEyePosition(client, eyeOrigin);

    MapData_BuildVoxels(
        origin,
        angles,
        g_VoxelFlags,
        sizeof(g_VoxelFlags)
    );

    MapData_TraceRays(
        origin,
        angles,
        eyeOrigin,
        g_Rays,
        sizeof(g_Rays) * sizeof(g_Rays[0])
    );
}
