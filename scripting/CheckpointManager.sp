#include <sourcemod>
#include <sdktools>
#include "Checkpoint/globals.inc"
#include "Checkpoint/manage.inc"
#include "Checkpoint/files.inc"


// ============================================================
// Plugin start
// ============================================================

public void OnPluginStart()
{
    g_Checkpoints =
        new ArrayList(sizeof(CheckpointData));

    RegConsoleCmd(
        "sm_checkpoint",
        Command_Checkpoint
    );

    CreateTimer(
        DRAW_INTERVAL,
        Timer_DrawCheckpointPlanes,
        _,
        TIMER_REPEAT | TIMER_FLAG_NO_MAPCHANGE
    );
}


// ============================================================
// Map start
// ============================================================

public void OnMapStart()
{
    g_DrawEnabled = false;

    g_BeamSprite =
        PrecacheModel(
            "materials/sprites/laserbeam.vmt"
        );
}

// ============================================================
// Main checkpoint command
// ============================================================

public Action Command_Checkpoint(
    int client,
    int args
)
{
    if (args < 1)
    {
        ReplyToCommand(
            client,
            "[Checkpoint] Usage:"
        );

        ReplyToCommand(
            client,
            "  sm_checkpoint generate"
        );

        ReplyToCommand(
            client,
            "  sm_checkpoint print"
        );

        ReplyToCommand(
            client,
            "  sm_checkpoint save"
        );

        ReplyToCommand(
            client,
            "  sm_checkpoint order <index> <index> ..."
        );

        ReplyToCommand(
            client,
            "  sm_checkpoint draw"
        );

        return Plugin_Handled;
    }

    char command[32];

    GetCmdArg(
        1,
        command,
        sizeof(command)
    );

    if (StrEqual(command, "generate"))
    {
        GenerateCheckpoints(client);
    }
    else if (StrEqual(command, "print"))
    {
        PrintCheckpoints(client);
    }
    else if (StrEqual(command, "save", false))
    {
        bool flat = false;

        if (args >= 2)
        {
            char option[32];

            GetCmdArg(
                2,
                option,
                sizeof(option)
            );

            if (StrEqual(option, "flat", false))
            {
                flat = true;
            }
        }
        SaveCheckpoints(flat);
    }
    else if (StrEqual(command, "savecourse"))
    {
        SaveCourse(
            client,
            args
        );
    }
    else if (StrEqual(command, "order"))
    {
        OrderCheckpoints(
            client,
            args
        );
    }
    else if (StrEqual(command, "draw"))
    {
        g_DrawEnabled = !g_DrawEnabled;

        ReplyToCommand(
            client,
            "[Checkpoint] Drawing %s.",
            g_DrawEnabled ? "enabled" : "disabled"
        );
    }
    else
    {
        ReplyToCommand(
            client,
            "[Checkpoint] Unknown command: %s",
            command
        );
    }

    return Plugin_Handled;
}



// ============================================================
// Drawing timer
// ============================================================

public Action Timer_DrawCheckpointPlanes(
    Handle timer
)
{
    if (!g_DrawEnabled)
    {
        return Plugin_Continue;
    }

    DrawCheckpointPlanes();

    return Plugin_Continue;
}