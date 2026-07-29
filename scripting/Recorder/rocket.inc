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
    if (!IsValidEntity(entity))
    {
        return;
    }

    int slot = FindFreeRocketSlot();

    if (slot == -1)
    {
        return;
    }

    int owner = GetEntPropEnt(entity, Prop_Send, "m_hOwnerEntity");

    if (owner > 0 && owner <= MaxClients && IsClientInGame(owner))
    {
        g_RocketFired[owner] = true;
    }

    SDKHook(entity, SDKHook_StartTouch, RocketTouch);

    float spawnAngles[3];
    g_RocketSlots[slot].exists = true;
    g_RocketSlots[slot].entity = EntIndexToEntRef(entity);
    g_RocketSlots[slot].id = g_NextRocketID++;
    g_RocketSlots[slot].spawnTick = GetGameTickCount();
    GetEntPropVector(entity, Prop_Send, "m_vecOrigin", g_RocketSlots[slot].position);
    GetEntPropVector(entity, Prop_Data, "m_vecVelocity", g_RocketSlots[slot].velocity);
    GetEntPropVector(entity, Prop_Send, "m_angRotation", spawnAngles);
    g_RocketSlots[slot].spawnPitch = spawnAngles[0];
    g_RocketSlots[slot].spawnYaw = spawnAngles[1];
    g_RocketSlots[slot].exploded = false;
    g_RocketSlots[slot].explosionPosition[0] = 0.0;
    g_RocketSlots[slot].explosionPosition[1] = 0.0;
    g_RocketSlots[slot].explosionPosition[2] = 0.0;
    g_RocketSlots[slot].clearAfterRecord = false;
}

public Action RocketTouch(int rocket, int other)
{
    for (int i = 0; i < MAX_ROCKET_SLOTS; i++)
    {
        int entity = EntRefToEntIndex(g_RocketSlots[i].entity);

        if (g_RocketSlots[i].exists && entity == rocket)
        {
            GetEntPropVector(rocket, Prop_Send, "m_vecOrigin", g_RocketSlots[i].explosionPosition);
            g_RocketSlots[i].position = g_RocketSlots[i].explosionPosition;
            g_RocketSlots[i].velocity[0] = 0.0;
            g_RocketSlots[i].velocity[1] = 0.0;
            g_RocketSlots[i].velocity[2] = 0.0;
            g_RocketSlots[i].exploded = true;
            g_RocketSlots[i].clearAfterRecord = true;
            break;
        }
    }

    return Plugin_Continue;
}

int FindFreeRocketSlot()
{
    for (int i = 0; i < MAX_ROCKET_SLOTS; i++)
    {
        if (!g_RocketSlots[i].exists)
        {
            return i;
        }
    }

    return -1;
}

void UpdateRocketSlots()
{
    for (int i = 0; i < MAX_ROCKET_SLOTS; i++)
    {
        if (!g_RocketSlots[i].exists || g_RocketSlots[i].exploded)
        {
            continue;
        }

        int entity = EntRefToEntIndex(g_RocketSlots[i].entity);

        if (entity == INVALID_ENT_REFERENCE)
        {
            g_RocketSlots[i].exploded = true;
            g_RocketSlots[i].explosionPosition = g_RocketSlots[i].position;
            g_RocketSlots[i].velocity[0] = 0.0;
            g_RocketSlots[i].velocity[1] = 0.0;
            g_RocketSlots[i].velocity[2] = 0.0;
            g_RocketSlots[i].clearAfterRecord = true;
            continue;
        }

        GetEntPropVector(entity, Prop_Send, "m_vecOrigin", g_RocketSlots[i].position);
        GetEntPropVector(entity, Prop_Data, "m_vecVelocity", g_RocketSlots[i].velocity);
    }
}

void ClearFinishedRocketSlots()
{
    for (int i = 0; i < MAX_ROCKET_SLOTS; i++)
    {
        if (g_RocketSlots[i].exists && g_RocketSlots[i].clearAfterRecord)
        {
            ClearRocketSlot(i);
        }
    }
}

void ResetRocketSlots()
{
    for (int i = 0; i < MAX_ROCKET_SLOTS; i++)
    {
        ClearRocketSlot(i);
    }

    g_NextRocketID = 1;
}

void ClearRocketSlot(int slot)
{
    g_RocketSlots[slot].exists = false;
    g_RocketSlots[slot].entity = INVALID_ENT_REFERENCE;
    g_RocketSlots[slot].id = 0;
    g_RocketSlots[slot].spawnTick = 0;
    g_RocketSlots[slot].position[0] = 0.0;
    g_RocketSlots[slot].position[1] = 0.0;
    g_RocketSlots[slot].position[2] = 0.0;
    g_RocketSlots[slot].velocity[0] = 0.0;
    g_RocketSlots[slot].velocity[1] = 0.0;
    g_RocketSlots[slot].velocity[2] = 0.0;
    g_RocketSlots[slot].spawnPitch = 0.0;
    g_RocketSlots[slot].spawnYaw = 0.0;
    g_RocketSlots[slot].exploded = false;
    g_RocketSlots[slot].explosionPosition[0] = 0.0;
    g_RocketSlots[slot].explosionPosition[1] = 0.0;
    g_RocketSlots[slot].explosionPosition[2] = 0.0;
    g_RocketSlots[slot].clearAfterRecord = false;
}
