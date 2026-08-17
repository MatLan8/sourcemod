#include <sourcemod>
#include <sdktools>
#include "MapData/voxel.inc"

int g_VoxelFlags[VOXEL_COUNT];

static void CacheStaticProps()
{
    int count = Voxel_CacheStaticProps();

    PrintToServer("[VoxelTest] Cached %d static props", count);
}

static void CacheTriggers()
{
    int count = Voxel_CacheTriggers();

    PrintToServer("[VoxelTest] Cached %d triggers", count);
}

static void CacheMapGeometry()
{
    CacheStaticProps();
    CacheTriggers();
}

public void OnPluginStart()
{
    RegConsoleCmd("sm_voxeltest", Command_VoxelTest);
    HookEvent("teamplay_round_start", Event_OnRoundStart, EventHookMode_PostNoCopy);
    PrintToServer("[VoxelTest] Loaded. Use sm_voxeltest in-game.");
    // Map geometry is cached in OnMapStart. The map is not loaded here.
}



public void OnMapStart()
{
    CacheMapGeometry();
}

public void Event_OnRoundStart(Event event, const char[] name, bool dontBroadcast)
{
    CacheTriggers();
}

public Action Command_VoxelTest(int client, int args)
{
    if (client <= 0 || !IsClientInGame(client))
    {
        ReplyToCommand(client, "[VoxelTest] Must be used in-game.");
        return Plugin_Handled;
    }

    float origin[3];
    float angles[3];

    GetClientAbsOrigin(client, origin);
    GetClientEyeAngles(client, angles);

    bool success = Voxel_BuildCache(
        origin,
        angles,
        g_VoxelFlags,
        sizeof(g_VoxelFlags)
    );

    int occupied = 0;
    int solid = 0;
    int playerClip = 0;
    int water = 0;
    int noGrenades = 0;
    int teleport = 0;

    for (int i = 0; i < VOXEL_COUNT; i++)
    {
        int flags = g_VoxelFlags[i];

        if (flags == 0)
        {
            continue;
        }

        occupied++;

        if (flags & GEOM_SOLID)
        {
            solid++;
        }

        if (flags & GEOM_PLAYERCLIP)
        {
            playerClip++;
        }

        if (flags & GEOM_WATER)
        {
            water++;
        }

        if (flags & GEOM_NOGRENADES)
        {
            noGrenades++;
        }

        if (flags & GEOM_TELEPORT)
        {
            teleport++;
        }
    }

    PrintToServer(
        "[VoxelTest] success=%d occupied=%d/%d solid=%d clip=%d water=%d nogrenades=%d teleport=%d yaw=%.1f",
        success,
        occupied,
        VOXEL_COUNT,
        solid,
        playerClip,
        water,
        noGrenades,
        teleport,
        angles[1]
    );

    PrintToChat(
        client,
        "[VoxelTest] occupied %d / %d (solid %d clip %d water %d ng %d tp %d)",
        occupied,
        VOXEL_COUNT,
        solid,
        playerClip,
        water,
        noGrenades,
        teleport
    );

    return Plugin_Handled;
}