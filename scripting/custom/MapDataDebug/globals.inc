#if defined _mapdatadebug_globals_included
    #endinput
#endif

#define _mapdatadebug_globals_included

#define RAY_DRAW_LIFE 1.0
#define RAY_DRAW_WIDTH 0.4
#define RAY_DRAW_CYCLE 0.9
#define RAY_DRAW_BATCH 22
#define RAY_OFFSET_Z 32.0

#define VOXEL_BOX_SIZE 256.0
#define VOXEL_BOX_OFFSET_Z 32.0
#define VOXEL_SIZE 16.0
#define VOXEL_BEAM_LIFE 0.1
#define VOXEL_BEAM_WIDTH 0.4
#define VOXEL_DRAW_TICKS 3

int g_VoxelFlags[VOXEL_COUNT];
RayData g_Rays[RAY_COUNT];
bool g_RaysReady = false;

int g_BeamSprite = -1;

bool g_RayDrawEnabled = false;
int g_RayDrawClient = 0;
int g_RayDrawCycleStartTick = 0;

float g_RayStart[RAY_COUNT][3];
float g_RayEnd[RAY_COUNT][3];

bool g_VoxelDrawEnabled = false;
int g_VoxelDrawClient = 0;
int g_VoxelDrawTick = 0;

bool g_ProfileEnabled = false;
int g_ProfileClient = 0;
int g_ProfileTickCount = 0;
float g_ProfileVoxelTimeTotal = 0.0;
float g_ProfileRayTimeTotal = 0.0;
float g_ProfileVoxelTimeMax = 0.0;
float g_ProfileRayTimeMax = 0.0;

Profiler g_NativeProfiler;

void StartNativeTimer()
{
    if (g_NativeProfiler == null)
    {
        g_NativeProfiler = new Profiler();
    }

    g_NativeProfiler.Start();
}

float StopNativeTimerMs()
{
    g_NativeProfiler.Stop();
    return g_NativeProfiler.Time * 1000.0;
}

int ResolveMapDataTarget(int client)
{
    if (client > 0 && IsClientInGame(client) && !IsClientSourceTV(client) && !IsClientReplay(client))
    {
        return client;
    }

    for (int i = 1; i <= MaxClients; i++)
    {
        if (IsClientInGame(i) && !IsClientSourceTV(i) && !IsClientReplay(i))
        {
            return i;
        }
    }

    return 0;
}
