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

    g_OnCheckpointConfigChanged = CreateGlobalForward(
        "CheckpointManager_OnConfigChanged",
        ET_Ignore
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
            "  sm_checkpoint update"
        );

        ReplyToCommand(
            client,
            "  sm_checkpoint savecourse <course_name> <checkpoint_index> ..."
        );

        ReplyToCommand(
            client,
            "  sm_checkpoint order <index> <index> ..."
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
    else if (StrEqual(command, "update"))
    {
        Update(client);
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

void FireConfigChanged()
{
    if (g_OnCheckpointConfigChanged == null)
    {
        return;
    }

    Call_StartForward(
        g_OnCheckpointConfigChanged
    );

    Call_Finish();
}