#include <sourcemod>
#include <sdktools>
#include <profiler>

#include "Shared/mapdata.inc"
#include "MapDataDebug/globals.inc"
#include "MapDataDebug/voxels.inc"
#include "MapDataDebug/raycasts.inc"
#include "MapDataDebug/voxelsDraw.inc"
#include "MapDataDebug/raycastsDraw.inc"

public Plugin myinfo =
{
    name = "MapData Debug",
    author = "MatLan8",
    description = "Debug commands for MapData voxels and raycasts",
    version = "1.0"
};

public void OnPluginStart()
{
    RegConsoleCmd(
        "sm_mapdata",
        Command_MapData
    );

    PrintToServer(
        "[MapDataDebug] Loaded."
    );
}

public void OnMapStart()
{
    g_RaysReady = false;
    g_RayDrawEnabled = false;
    g_VoxelDrawEnabled = false;
    g_BeamSprite = PrecacheModel("materials/sprites/laserbeam.vmt");
}

public void OnGameFrame()
{
    TickRayDraw();
    TickVoxelDraw();
}

void PrintMapDataUsage(int client)
{
    ReplyToCommand(client, "[MapDataDebug] Usage:");
    ReplyToCommand(client, "  sm_mapdata cache");
    ReplyToCommand(client, "  sm_mapdata voxels");
    ReplyToCommand(client, "  sm_mapdata rays");
    ReplyToCommand(client, "  sm_mapdata printRays");
    ReplyToCommand(client, "  sm_mapdata drawRays");
    ReplyToCommand(client, "  sm_mapdata drawVoxels");
}

void Command_Cache(int client)
{
    int propCount = MapData_CacheStaticProps();
    int triggerCount = MapData_CacheTriggers();

    PrintToServer(
        "[MapDataDebug] Cached %d static props, %d triggers",
        propCount,
        triggerCount
    );

    ReplyToCommand(
        client,
        "[MapDataDebug] Cached %d static props, %d triggers",
        propCount,
        triggerCount
    );
}

public Action Command_MapData(int client, int args)
{
    if (args < 1)
    {
        PrintMapDataUsage(client);
        return Plugin_Handled;
    }

    char command[32];

    GetCmdArg(
        1,
        command,
        sizeof(command)
    );

    if (StrEqual(command, "cache", false))
    {
        Command_Cache(client);
        return Plugin_Handled;
    }

    if (StrEqual(command, "voxels", false))
    {
        return Command_VoxelTest(client);
    }

    if (StrEqual(command, "rays", false))
    {
        return Command_RayTest(client);
    }

    if (StrEqual(command, "printRays", false))
    {
        return Command_PrintRays(client);
    }

    if (StrEqual(command, "drawRays", false))
    {
        return Command_RaysDraw(client);
    }

    if (StrEqual(command, "drawVoxels", false))
    {
        return Command_VoxelsDraw(client);
    }

    ReplyToCommand(
        client,
        "[MapDataDebug] Unknown command: %s",
        command
    );

    PrintMapDataUsage(client);
    return Plugin_Handled;
}
