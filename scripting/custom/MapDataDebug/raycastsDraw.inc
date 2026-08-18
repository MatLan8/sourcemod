#if defined _mapdatadebug_raycastsdraw_included
    #endinput
#endif

#define _mapdatadebug_raycastsdraw_included

void DrawRayBeam(int client, int index)
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

void DrawRayBatch(int client, int startIndex, int count)
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

void TickRayDraw()
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

Action Command_RaysDraw(int client)
{
    if (client <= 0 || !IsClientInGame(client))
    {
        ReplyToCommand(client, "[MapDataDebug] drawRays must be used in-game.");
        return Plugin_Handled;
    }

    if (g_RayDrawEnabled && g_RayDrawClient == client)
    {
        g_RayDrawEnabled = false;
        ReplyToCommand(client, "[MapDataDebug] Ray draw off.");
        return Plugin_Handled;
    }

    if (!g_RaysReady)
    {
        ReplyToCommand(client, "[MapDataDebug] No ray data. Use sm_mapdata rays first.");
        return Plugin_Handled;
    }

    g_RayDrawClient = client;
    g_RayDrawEnabled = true;
    g_RayDrawCycleStartTick = GetGameTickCount();
    ReplyToCommand(client, "[MapDataDebug] Ray draw on. Frozen at last sm_mapdata rays.");
    return Plugin_Handled;
}
