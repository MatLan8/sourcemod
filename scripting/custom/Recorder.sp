#include <sourcemod>
#include <sdktools>
#include <sdkhooks>

bool g_Recording = false;
Handle g_RecordingFile = INVALID_HANDLE;
Handle g_MapDataFile = INVALID_HANDLE;
int g_RecordRowNumber = 0;

#include "Shared/courseRuntime.inc"
#include "Shared/mapDataNatives.inc"
#include "Shared/globals.inc"
#include "Shared/map.inc"
#include "Shared/input.inc"
#include "Shared/player.inc"
#include "Shared/regenerateTriggers.inc"
#include "Shared/rocket.inc"
#include "Recorder/files.inc"

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
    HookEvent("teamplay_round_start", Event_OnRoundStart, EventHookMode_PostNoCopy);

    PrintToServer("[Recorder] Loaded");
}

public void OnMapStart()
{
    int propCount = MapData_CacheStaticProps();

    PrintToServer(
        "[Recorder] Cached %d static props",
        propCount
    );

    MapSync();
}

public void Event_OnRoundStart(Event event, const char[] name, bool dontBroadcast)
{
    MapSync();
}

public void MapSync() {
    int regenCount = CacheRegenerateTriggers();
    int triggerCount = MapData_CacheTriggers();

    SyncRecorderCheckpoint();

    PrintToServer(
        "[MapDataDebug] Cached %d triggers, %d regenerate triggers",
        triggerCount,
        regenCount
    );
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
    GatherMapData(client);

    int tick = GetGameTickCount();

    WriteRecordingRow(tick, player, input, g_RocketFired[client]);
    WriteMapDataBinary(tick);

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

void SyncRecorderCheckpoint()
{
    CourseRuntime_GetCurrentCheckpoint(g_CurrentCheckpoint);
}

public void CourseRuntime_OnCheckpointEntered()
{
    SyncRecorderCheckpoint();
}

public void CourseRuntime_OnCourseDataUpdated()
{
    SyncRecorderCheckpoint();
}