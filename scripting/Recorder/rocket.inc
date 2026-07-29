#if defined _recorder_rocket_included
    #endinput
#endif
#define _recorder_rocket_included

void HandleRocketEntityCreated(int entity, const char[] classname)
{
    if (!g_Recording || !StrEqual(classname, "tf_projectile_rocket"))
    {
        return;
    }

    SDKHook(entity, SDKHook_SpawnPost, RocketSpawned);
}

public void RocketSpawned(int entity)
{
    if (!IsValidEntity(entity) || g_RocketCount >= 2048)
    {
        return;
    }

    int owner = GetEntPropEnt(entity, Prop_Send, "m_hOwnerEntity");

    if (owner > 0 && owner <= MaxClients && IsClientInGame(owner))
    {
        g_RocketFired[owner] = true;
    }

    SDKHook(entity, SDKHook_StartTouch, RocketTouch);

    g_Rockets[g_RocketCount].entity = EntIndexToEntRef(entity);
    g_Rockets[g_RocketCount].id = g_NextRocketID++;
    g_Rockets[g_RocketCount].owner = owner;
    g_Rockets[g_RocketCount].spawnTick = GetGameTickCount();
    g_Rockets[g_RocketCount].exploded = false;
    g_Rockets[g_RocketCount].explosionTick = 0;
    g_Rockets[g_RocketCount].explosionPos[0] = 0.0;
    g_Rockets[g_RocketCount].explosionPos[1] = 0.0;
    g_Rockets[g_RocketCount].explosionPos[2] = 0.0;

    g_RocketCount++;
}

public Action RocketTouch(int rocket, int other)
{
    for (int i = 0; i < g_RocketCount; i++)
    {
        int entity = EntRefToEntIndex(g_Rockets[i].entity);

        if (entity == rocket)
        {
            GetEntPropVector(rocket, Prop_Send, "m_vecOrigin", g_Rockets[i].explosionPos);
            g_Rockets[i].exploded = true;
            g_Rockets[i].explosionTick = GetGameTickCount();
            break;
        }
    }

    return Plugin_Continue;
}

public Action Timer_ProcessRocket(Handle timer, int ref)
{
    int entity = EntRefToEntIndex(ref);

    if (entity == INVALID_ENT_REFERENCE || g_RocketFile == INVALID_HANDLE)
    {
        return Plugin_Stop;
    }

    int owner = GetEntPropEnt(entity, Prop_Send, "m_hOwnerEntity");

    if (owner <= 0 || owner > MaxClients || !IsClientInGame(owner))
    {
        return Plugin_Stop;
    }

    float pos[3];
    float vel[3];
    GetEntPropVector(entity, Prop_Send, "m_vecOrigin", pos);
    GetEntPropVector(entity, Prop_Data, "m_vecVelocity", vel);

    WriteFileLine(g_RocketFile, "%d,%.2f,%.2f,%.2f,%.2f,%.2f,%.2f,%d", GetGameTickCount(), pos[0], pos[1], pos[2], vel[0], vel[1], vel[2], owner);
    FlushFile(g_RocketFile);

    g_RocketFired[owner] = true;
    return Plugin_Stop;
}

void RecordRockets()
{
    if (g_RocketFile == INVALID_HANDLE)
    {
        return;
    }

    int tick = GetGameTickCount();

    for (int i = 0; i < g_RocketCount; i++)
    {
        int entity = EntRefToEntIndex(g_Rockets[i].entity);

        if (entity == INVALID_ENT_REFERENCE)
        {
            if (!g_Rockets[i].exploded)
            {
                g_Rockets[i].exploded = true;
                g_Rockets[i].explosionTick = tick;
            }

            WriteExplosionFrame(i);
            RemoveRocket(i);
            i--;
            continue;
        }

        float pos[3];
        float vel[3];
        GetEntPropVector(entity, Prop_Send, "m_vecOrigin", pos);
        GetEntPropVector(entity, Prop_Data, "m_vecVelocity", vel);

        WriteFileLine(g_RocketFile, "%d,%d,%d,%d,%.3f,%.3f,%.3f,%.3f,%.3f,%.3f,%d,%d,%.3f,%.3f,%.3f", tick, g_Rockets[i].id, g_Rockets[i].spawnTick, g_Rockets[i].owner, pos[0], pos[1], pos[2], vel[0], vel[1], vel[2], 0, 0, 0.0, 0.0, 0.0);
    }

    FlushFile(g_RocketFile);
}

void RemoveRocket(int index)
{
    for (int i = index; i < g_RocketCount - 1; i++)
    {
        g_Rockets[i] = g_Rockets[i + 1];
    }

    g_RocketCount--;
}

void WriteExplosionFrame(int index)
{
    WriteFileLine(g_RocketFile, "%d,%d,%d,%d,0,0,0,0,0,0,%d,%d,%.3f,%.3f,%.3f", GetGameTickCount(), g_Rockets[index].id, g_Rockets[index].spawnTick, g_Rockets[index].owner, 1, g_Rockets[index].explosionTick, g_Rockets[index].explosionPos[0], g_Rockets[index].explosionPos[1], g_Rockets[index].explosionPos[2]);
}
