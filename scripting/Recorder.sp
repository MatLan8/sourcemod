#include <sourcemod>
#include <sdktools>
#include <sdkhooks>

#include "Recorder/globals.inc"
#include "Recorder/files.inc"
#include "Recorder/input.inc"
#include "Recorder/player.inc"
#include "Recorder/rocket.inc"

public Plugin myinfo =
{
    name = "Rocket Jump Recorder",
    author = "YourName",
    description = "Records TF2 movement data for AI training",
    version = "1.0"
};

public void OnPluginStart()
{
    RegConsoleCmd("sm_record", Command_StartRecording);
    RegConsoleCmd("sm_stoprecord", Command_StopRecording);

    HookEvent("player_spawn", Event_PlayerSpawn);

    PrintToServer("[Recorder] Loaded");
}

public void OnPluginEnd()
{
    CloseFiles();
}

public void OnGameFrame()
{
    if (!g_Recording)
    {
        return;
    }

    int client = GetRecordingPlayer();

    if (client == 0)
    {
        return;
    }

    int tick = GetGameTickCount();

    RecordPlayerData(client, tick);
    RecordPlayerInput(client, tick);
    RecordRockets();
}

public void Event_PlayerSpawn(Event event, const char[] name, bool dontBroadcast)
{
    ResetPlayerRecordingState(GetClientOfUserId(event.GetInt("userid")));
}

public void OnEntityCreated(int entity, const char[] classname)
{
    HandleRocketEntityCreated(entity, classname);
}
