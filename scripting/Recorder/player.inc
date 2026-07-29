#if defined _recorder_player_included
    #endinput
#endif
#define _recorder_player_included

void RecordPlayerData(int client, int tick)
{
    float pos[3];
    float vel[3];
    float ang[3];

    GetClientAbsOrigin(client, pos);
    GetEntPropVector(client, Prop_Data, "m_vecVelocity", vel);
    GetClientEyeAngles(client, ang);

    int water = GetEntProp(client, Prop_Send, "m_nWaterLevel");
    int ammo = 0;
    int weapon = GetEntPropEnt(client, Prop_Send, "m_hActiveWeapon");

    if (weapon != -1)
    {
        ammo = GetEntProp(weapon, Prop_Send, "m_iClip1");
    }

    WriteFileLine(g_PositionFile, "%d,%.3f,%.3f,%.3f", tick, pos[0], pos[1], pos[2]);
    WriteFileLine(g_VelocityFile, "%d,%.3f,%.3f,%.3f", tick, vel[0], vel[1], vel[2]);
    WriteFileLine(g_AnglesFile, "%d,%.3f,%.3f", tick, ang[0], ang[1]);
    WriteFileLine(g_StateFile, "%d,%d,%d,%d,%d", tick, IsPlayerGrounded(client), water, ammo, g_RocketFired[client]);

    FlushFile(g_PositionFile);
    FlushFile(g_VelocityFile);
    FlushFile(g_AnglesFile);
    FlushFile(g_StateFile);

    g_RocketFired[client] = false;
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
    }
}
