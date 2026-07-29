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
    description = "Records synchronized TF2 rocket-jump data for AI training",
    version = "2.0"
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

    PlayerSnapshot player;
    PlayerInput input;

    GatherPlayerSnapshot(client, player);
    GatherPlayerInput(client, input);
    UpdateRocketSlots();

    WriteRecordingRow(GetGameTickCount(), player, input, g_RocketFired[client]);

    g_RocketFired[client] = false;
    ClearFinishedRocketSlots();
}

public void Event_PlayerSpawn(Event event, const char[] name, bool dontBroadcast)
{
    ResetPlayerRecordingState(GetClientOfUserId(event.GetInt("userid")));
}

public void OnEntityCreated(int entity, const char[] classname)
{
    HandleRocketEntityCreated(entity, classname);
}
