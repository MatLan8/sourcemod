#include <sourcemod>
#include <sdktools>
#include <sdkhooks>

#include "VoxelVisualizer/core"
#include "VoxelVisualizer/drawing"

public Plugin myinfo =
{
    name = "Voxel Visualizer",
    author = "YourName",
    description = "Debug visualization volume for AI occupancy grids",
    version = "1.0"
};


public void OnPluginStart()
{
    RegConsoleCmd(
        "sm_voxel",
        Command_Voxel
    );


    InitVoxelSystem();
    InitDrawing();

    PrintToServer(
        "[VoxelVisualizer] Loaded"
    );
}


public Action Command_Voxel(
    int client,
    int args
)
{
    if(client <= 0)
    {
        return Plugin_Handled;
    }


    if(args < 1)
    {
        PrintToChat(
            client,
            "Usage: sm_voxel <on/off/size/offset/voxelsize/grid>"
        );

        return Plugin_Handled;
    }


    char command[32];

    GetCmdArg(
        1,
        command,
        sizeof(command)
    );


    if(StrEqual(command, "on"))
    {
        g_VoxelOwner = client;
        g_VoxelEnabled = true;

        PrintToChat(
            client,
            "[Voxel] Enabled"
        );
    }


    else if(StrEqual(command, "off"))
    {
        g_VoxelEnabled = false;

        PrintToChat(
            client,
            "[Voxel] Disabled"
        );
    }

    else if(StrEqual(command, "size"))
    {
        if(args < 4)
            return Plugin_Handled;


        char buffer[32];


        GetCmdArg(2, buffer, sizeof(buffer));
        g_BoxSize[0] = StringToFloat(buffer);


        GetCmdArg(3, buffer, sizeof(buffer));
        g_BoxSize[1] = StringToFloat(buffer);


        GetCmdArg(4, buffer, sizeof(buffer));
        g_BoxSize[2] = StringToFloat(buffer);


        RecalculateVoxelLayout();


        PrintToChat(
            client,
            "[Voxel] Size updated"
        );
    }


    else if(StrEqual(command, "offset"))
    {
        if(args < 4)
            return Plugin_Handled;


        char buffer[32];


        GetCmdArg(2, buffer, sizeof(buffer));
        g_BoxOffset[0] = StringToFloat(buffer);


        GetCmdArg(3, buffer, sizeof(buffer));
        g_BoxOffset[1] = StringToFloat(buffer);


        GetCmdArg(4, buffer, sizeof(buffer));
        g_BoxOffset[2] = StringToFloat(buffer);


        PrintToChat(
            client,
            "[Voxel] Offset updated"
        );
    }


    else if(StrEqual(command, "voxelsize"))
    {
        if(args < 2)
            return Plugin_Handled;


        char buffer[32];

        GetCmdArg(
            2,
            buffer,
            sizeof(buffer)
        );


        g_VoxelSize = StringToFloat(buffer);


        RecalculateVoxelLayout();


        PrintToChat(
            client,
            "[Voxel] Voxel size updated"
        );
    }


    else if(StrEqual(command, "grid"))
    {
        char value[16];

        GetCmdArg(
            2,
            value,
            sizeof(value)
        );


        g_DrawVoxelGrid =
            StrEqual(value,"on");


        PrintToChat(
            client,
            "[Voxel] Grid %s",
            g_DrawVoxelGrid ? "enabled" : "disabled"
        );
    }
    return Plugin_Handled;
}

int g_DrawTick = 0;

public void OnGameFrame()
{
    if(!g_VoxelEnabled)
        return;

    g_DrawTick++;

    if(g_DrawTick < 3)
        return;

    g_DrawTick = 0;

    int client = g_VoxelOwner;

    if(client <= 0)
        return;


    if(!IsClientInGame(client))
        return;


    UpdateVoxelVolume(client);
    DrawVoxelVolume();
}