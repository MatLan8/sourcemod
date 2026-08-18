#if defined _mapdatadebug_profile_included
    #endinput
#endif

#define _mapdatadebug_profile_included

void ResetProfileStats()
{
    g_ProfileTickCount = 0;
    g_ProfileVoxelTimeTotal = 0.0;
    g_ProfileRayTimeTotal = 0.0;
    g_ProfileVoxelTimeMax = 0.0;
    g_ProfileRayTimeMax = 0.0;
}

void RecordProfileFrame()
{
    int target = ResolveMapDataTarget(g_ProfileClient);

    if (target == 0)
    {
        return;
    }

    float origin[3];
    float angles[3];
    float eyeOrigin[3];

    GetClientAbsOrigin(target, origin);
    GetClientEyeAngles(target, angles);
    GetClientEyePosition(target, eyeOrigin);

    StartNativeTimer();
    MapData_BuildVoxels(
        origin,
        angles,
        g_VoxelFlags,
        sizeof(g_VoxelFlags)
    );
    float voxelMs = StopNativeTimerMs();

    StartNativeTimer();
    MapData_TraceRays(
        origin,
        angles,
        eyeOrigin,
        g_Rays,
        sizeof(g_Rays) * sizeof(g_Rays[0])
    );
    float rayMs = StopNativeTimerMs();

    g_ProfileTickCount++;
    g_ProfileVoxelTimeTotal += voxelMs;
    g_ProfileRayTimeTotal += rayMs;

    if (voxelMs > g_ProfileVoxelTimeMax)
    {
        g_ProfileVoxelTimeMax = voxelMs;
    }

    if (rayMs > g_ProfileRayTimeMax)
    {
        g_ProfileRayTimeMax = rayMs;
    }
}

void PrintProfileData(int client)
{
    if (g_ProfileTickCount <= 0)
    {
        ReplyToCommand(client, "[MapDataDebug] Profile: no ticks recorded.");
        return;
    }

    float ticks = float(g_ProfileTickCount);
    float avgVoxels = g_ProfileVoxelTimeTotal / ticks;
    float avgRays = g_ProfileRayTimeTotal / ticks;
    float avgTogether = avgVoxels + avgRays;

    PrintToServer(
        "[MapDataDebug] profile ticks=%d avg voxels=%.4fms avg rays=%.4fms avg together=%.4fms max voxels=%.4fms max rays=%.4fms",
        g_ProfileTickCount,
        avgVoxels,
        avgRays,
        avgTogether,
        g_ProfileVoxelTimeMax,
        g_ProfileRayTimeMax
    );

    if (client > 0 && IsClientInGame(client))
    {
        PrintToChat(
            client,
            "[MapDataDebug] profile ticks=%d avg voxels=%.4fms avg rays=%.4fms avg together=%.4fms max voxels=%.4fms max rays=%.4fms",
            g_ProfileTickCount,
            avgVoxels,
            avgRays,
            avgTogether,
            g_ProfileVoxelTimeMax,
            g_ProfileRayTimeMax
        );
    }
}

Action Command_Profile(int client)
{
    if (g_ProfileEnabled)
    {
        g_ProfileEnabled = false;
        PrintProfileData(client);
        return Plugin_Handled;
    }

    int target = ResolveMapDataTarget(client);

    if (target == 0)
    {
        ReplyToCommand(client, "[MapDataDebug] No in-game player. Add a fake client or use this in-game.");
        return Plugin_Handled;
    }

    g_ProfileClient = target;
    ResetProfileStats();
    g_ProfileEnabled = true;

    ReplyToCommand(client, "[MapDataDebug] Profile on. Use sm_mapdata profile again to stop and print averages.");

    if (client > 0 && IsClientInGame(client))
    {
        PrintToChat(client, "[MapDataDebug] Profile on.");
    }

    return Plugin_Handled;
}
