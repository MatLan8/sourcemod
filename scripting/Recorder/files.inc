#if defined _recorder_files_included
    #endinput
#endif
#define _recorder_files_included

public Action Command_StartRecording(int client, int args)
{
    if (g_Recording)
    {
        ReplyToCommand(client, "[Recorder] Already recording!");
        return Plugin_Handled;
    }

    g_PositionFile = OpenFile("addons/sourcemod/data/position.csv", "w");
    g_VelocityFile = OpenFile("addons/sourcemod/data/velocity.csv", "w");
    g_AnglesFile = OpenFile("addons/sourcemod/data/angles.csv", "w");
    g_MovementFile = OpenFile("addons/sourcemod/data/movement.csv", "w");
    g_StateFile = OpenFile("addons/sourcemod/data/state.csv", "w");
    g_RocketFile = OpenFile("addons/sourcemod/data/rocket.csv", "w");

    if (g_PositionFile == INVALID_HANDLE ||
        g_VelocityFile == INVALID_HANDLE ||
        g_AnglesFile == INVALID_HANDLE ||
        g_MovementFile == INVALID_HANDLE ||
        g_StateFile == INVALID_HANDLE ||
        g_RocketFile == INVALID_HANDLE)
    {
        ReplyToCommand(client, "[Recorder] Failed opening files");
        return Plugin_Handled;
    }

    WriteFileLine(g_PositionFile, "tick,x,y,z");
    WriteFileLine(g_VelocityFile, "tick,vx,vy,vz");
    WriteFileLine(g_AnglesFile, "tick,pitch,yaw");
    WriteFileLine(g_MovementFile, "tick,forward,left,jump,duck,attack");
    WriteFileLine(g_StateFile, "tick,grounded,water,ammo,rocket_fired");
    WriteFileLine(g_RocketFile, "tick,rocket_id,spawn_tick,owner,x,y,z,vx,vy,vz,exploded,explosion_tick,explosion_x,explosion_y,explosion_z");

    g_Recording = true;

    ReplyToCommand(client, "[Recorder] Recording started!");
    return Plugin_Handled;
}

public Action Command_StopRecording(int client, int args)
{
    if (!g_Recording)
    {
        ReplyToCommand(client, "[Recorder] Not recording!");
        return Plugin_Handled;
    }

    g_Recording = false;
    CloseFiles();

    ReplyToCommand(client, "[Recorder] Recording stopped!");
    return Plugin_Handled;
}

void CloseFiles()
{
    if (g_PositionFile != INVALID_HANDLE)
    {
        CloseHandle(g_PositionFile);
        g_PositionFile = INVALID_HANDLE;
    }

    if (g_VelocityFile != INVALID_HANDLE)
    {
        CloseHandle(g_VelocityFile);
        g_VelocityFile = INVALID_HANDLE;
    }

    if (g_AnglesFile != INVALID_HANDLE)
    {
        CloseHandle(g_AnglesFile);
        g_AnglesFile = INVALID_HANDLE;
    }

    if (g_MovementFile != INVALID_HANDLE)
    {
        CloseHandle(g_MovementFile);
        g_MovementFile = INVALID_HANDLE;
    }

    if (g_StateFile != INVALID_HANDLE)
    {
        CloseHandle(g_StateFile);
        g_StateFile = INVALID_HANDLE;
    }

    if (g_RocketFile != INVALID_HANDLE)
    {
        CloseHandle(g_RocketFile);
        g_RocketFile = INVALID_HANDLE;
    }
}
