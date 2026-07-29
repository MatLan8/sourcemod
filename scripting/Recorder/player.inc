#if defined _recorder_player_included
    #endinput
#endif
#define _recorder_player_included

void GatherPlayerSnapshot(int client, PlayerSnapshot player)
{
    GetClientAbsOrigin(client, player.position);
    GetEntPropVector(client, Prop_Data, "m_vecVelocity", player.velocity);
    GetClientEyeAngles(client, player.viewAngles);

    player.grounded = IsPlayerGrounded(client);
    player.waterLevel = GetEntProp(client, Prop_Send, "m_nWaterLevel");
    player.ammo = 0;
    int weapon = GetEntPropEnt(client, Prop_Send, "m_hActiveWeapon");

    if (weapon != -1)
    {
        player.ammo = GetEntProp(weapon, Prop_Send, "m_iClip1");
    }
}

int GetRecordingPlayer()
{
    for (int i = 1; i <= MaxClients; i++)
    {
        if (IsClientInGame(i) && IsPlayerAlive(i))
        {
            return i;
        }
    }

    return 0;
}

bool IsPlayerGrounded(int client)
{
    return (GetEntityFlags(client) & FL_ONGROUND) != 0;
}

void ResetPlayerRecordingState(int client)
{
    if (client > 0)
    {
        g_RocketFired[client] = false;
        g_PlayerButtons[client] = 0;
    }
}

void ResetRocketFiredEvents()
{
    for (int i = 1; i <= MaxClients; i++)
    {
        g_RocketFired[i] = false;
    }
}
