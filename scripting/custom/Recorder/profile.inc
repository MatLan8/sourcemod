#if defined _recorder_profile_included
    #endinput
#endif

#define _recorder_profile_included

Profiler g_RecorderProfiler;
Profiler g_RecorderTickProfiler;

int g_ProfileTickCount = 0;
float g_PlayerSnapshotTime = 0.0;
float g_PlayerInputTime = 0.0;
float g_UpdateRocketSlotsTime = 0.0;
float g_GatherMapDataTime = 0.0;
float g_WriteRecordingRowTime = 0.0;
float g_WriteMapDataBinaryTime = 0.0;
float g_MaxTime = 0.0;

void StartRecorderTimer()
{
    if (g_RecorderProfiler == null)
    {
        g_RecorderProfiler = new Profiler();
    }

    g_RecorderProfiler.Start();
}

float StopRecorderTimerMs()
{
    g_RecorderProfiler.Stop();
    return g_RecorderProfiler.Time * 1000.0;
}

void StartRecorderTickTimer()
{
    if (g_RecorderTickProfiler == null)
    {
        g_RecorderTickProfiler = new Profiler();
    }

    g_RecorderTickProfiler.Start();
}

float StopRecorderTickTimerMs()
{
    g_RecorderTickProfiler.Stop();
    return g_RecorderTickProfiler.Time * 1000.0;
}

void ResetRecordingProfile()
{
    g_ProfileTickCount = 0;
    g_PlayerSnapshotTime = 0.0;
    g_PlayerInputTime = 0.0;
    g_UpdateRocketSlotsTime = 0.0;
    g_GatherMapDataTime = 0.0;
    g_WriteRecordingRowTime = 0.0;
    g_WriteMapDataBinaryTime = 0.0;
    g_MaxTime = 0.0;
}

void PrintRecordingProfile(int client)
{
    if (g_ProfileTickCount <= 0)
    {
        ReplyToCommand(client, "[Recorder] Profile: no ticks recorded.");
        return;
    }

    float ticks = float(g_ProfileTickCount);
    float time = ticks / 66.0;
    float avgSnapshot = g_PlayerSnapshotTime / ticks;
    float avgInput = g_PlayerInputTime / ticks;
    float avgRockets = g_UpdateRocketSlotsTime / ticks;
    float avgMap = g_GatherMapDataTime / ticks;
    float avgCsv = g_WriteRecordingRowTime / ticks;
    float avgBin = g_WriteMapDataBinaryTime / ticks;
    float avgTogether = avgSnapshot + avgInput + avgRockets + avgMap + avgCsv + avgBin;

    PrintToServer(
        "[Recorder] profile time=%d ticks=%d avg snapshot=%.4fms avg input=%.4fms avg rockets=%.4fms avg map=%.4fms avg csv=%.4fms avg bin=%.4fms avg together=%.4fms max=%.4fms",
        RoundFloat(time),
        g_ProfileTickCount,
        avgSnapshot,
        avgInput,
        avgRockets,
        avgMap,
        avgCsv,
        avgBin,
        avgTogether,
        g_MaxTime
    );

    ReplyToCommand(
        client,
        "[Recorder] profile time=%d ticks=%d avg snapshot=%.4fms avg input=%.4fms avg rockets=%.4fms avg map=%.4fms avg csv=%.4fms avg bin=%.4fms avg together=%.4fms max=%.4fms",
        RoundFloat(time),
        g_ProfileTickCount,
        avgSnapshot,
        avgInput,
        avgRockets,
        avgMap,
        avgCsv,
        avgBin,
        avgTogether,
        g_MaxTime
    );
}
