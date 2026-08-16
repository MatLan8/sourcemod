#include <sourcemod>
#include <sdktools>
#include "MapData/voxel.inc"

int g_VoxelFlags[VOXEL_COUNT];

static void CacheStaticProps()
{
    int count = Voxel_CacheStaticProps();

    PrintToServer("[VoxelTest] Cached %d static props", count);
}

public void OnPluginStart()
{
    RegConsoleCmd("sm_voxeltest", Command_VoxelTest);
    CacheStaticProps();
    PrintToServer("[VoxelTest] Loaded. Use sm_voxeltest in-game.");
}

public void OnMapStart()
{
    CacheStaticProps();
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
    }

    PrintToServer(
        "[VoxelTest] success=%d occupied=%d/%d solid=%d clip=%d water=%d yaw=%.1f",
        success,
        occupied,
        VOXEL_COUNT,
        solid,
        playerClip,
        water,
        angles[1]
    );

    PrintToChat(
        client,
        "[VoxelTest] occupied %d / %d (solid %d clip %d water %d)",
        occupied,
        VOXEL_COUNT,
        solid,
        playerClip,
        water
    );

    return Plugin_Handled;
}
