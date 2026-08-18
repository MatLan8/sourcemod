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

int ResolveMapDataTarget(int client)
{
    if (client > 0 && IsClientInGame(client) && !IsClientSourceTV(client) && !IsClientReplay(client))
    {
        return client;
    }

    for (int i = 1; i <= MaxClients; i++)
    {
        if (IsClientInGame(i) && !IsClientSourceTV(i) && !IsClientReplay(i))
        {
            return i;
        }
    }

    return 0;
}
