#include <sourcemod>
#include <sdktools>
#include <sdkhooks>
#include <profiler>

bool g_Recording = false;
Handle g_RecordingFile = INVALID_HANDLE;
Handle g_MapDataFile = INVALID_HANDLE;
int g_RecordRowNumber = 0;
char g_RecordingType[64];
char g_LastRecordingCsvPath[PLATFORM_MAX_PATH];
char g_LastMapDataPath[PLATFORM_MAX_PATH];
bool g_CheckpointReached = false;
bool g_CourseComplete = false;

#include "Shared/courseRuntime.inc"
#include "Shared/mapDataNatives.inc"
#include "Shared/globals.inc"
#include "Shared/map.inc"
#include "Shared/input.inc"
#include "Shared/player.inc"
#include "Shared/regenerateTriggers.inc"
#include "Shared/rocket.inc"
#include "Recorder/profile.inc"
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
    RegConsoleCmd("sm_rerecord", Command_RerecordLastClip);
    RegConsoleCmd("sm_stoprecord", Command_StopRecording);
    RegConsoleCmd("sm_setrecordingtype", Command_SetRecordingType);

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

    StartRecorderTickTimer();

    StartRecorderTimer();
    GatherPlayerSnapshot(client, player);
    g_PlayerSnapshotTime += StopRecorderTimerMs();

    StartRecorderTimer();
    GatherPlayerInput(client, input);
    g_PlayerInputTime += StopRecorderTimerMs();

    StartRecorderTimer();
    UpdateRocketSlots();
    g_UpdateRocketSlotsTime += StopRecorderTimerMs();

    StartRecorderTimer();
    GatherMapData(client);
    g_GatherMapDataTime += StopRecorderTimerMs();

    int tick = GetGameTickCount();

    StartRecorderTimer();
    WriteRecordingRow(tick, player, input, g_RocketFired[client], g_CheckpointReached, g_CourseComplete);
    g_WriteRecordingRowTime += StopRecorderTimerMs();

    StartRecorderTimer();
    WriteMapDataBinary(tick);
    g_WriteMapDataBinaryTime += StopRecorderTimerMs();

    g_ProfileTickCount++;

    g_RocketFired[client] = false;
    g_CheckpointReached = false;
    
    ClearFinishedRocketSlots();

    float tickMs = StopRecorderTickTimerMs();

    if (tickMs > g_MaxTime)
    {
        g_MaxTime = tickMs;
    }

    if (g_CourseComplete) {
        Command_StopRecording(client, 0);
        return;
    }

    g_CourseComplete = false;
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

    if (g_Recording)
    {
        g_CheckpointReached = true;
    }
}

public void CourseRuntime_OnCourseComplete()
{
    if (g_Recording)
    {
        g_CourseComplete = true;
    }
}

public void CourseRuntime_OnCourseDataUpdated()
{
    SyncRecorderCheckpoint();
}