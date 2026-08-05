#include <sourcemod>
#include <sdktools>
#include "Debug/globals.inc"
#include "Shared/checkpoint.inc"
#include "Debug/checkpoint.inc"
#include "Shared/trigger.inc"

// ============================================================
// Plugin start
// ============================================================


public void OnPluginStart()
{
    RegConsoleCmd(
        "sm_debug",
        Command_Debug
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
    
    PrecacheModel(
        TRIGGER_MODEL,
        true
    );
    SetCourse();
}

// ============================================================
// Main checkpoint command
// ============================================================

public Action Command_Debug(
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
            "  sm_debug print"
        );

        ReplyToCommand(
            client,
            "  sm_debug drawCheckpoint"
        );

        ReplyToCommand(
            client,
            "  sm_debug course [name]"
        );

        ReplyToCommand(
            client,
            "  sm_debug next"
        );

        ReplyToCommand(
            client,
            "  sm_debug checkpoint <index>"
        );

        return Plugin_Handled;
    }

    char command[32];

    GetCmdArg(
        1,
        command,
        sizeof(command)
    );



    if (StrEqual(command, "drawCheckpoint", false))
    {
        g_DrawEnabled = !g_DrawEnabled;

        ReplyToCommand(
            client,
            "[Checkpoint] Drawing %s.",
            g_DrawEnabled ? "enabled" : "disabled"
        );
    }
    else if (StrEqual(command, "course", false))
    {
        if (args >= 2)
        {
            char courseName[128];

            GetCmdArg(
                2,
                courseName,
                sizeof(courseName)
            );

            SetCourse(courseName);
        }
        else
        {
            SetCourse();
        }

        ReplyToCommand(
            client,
            "[Checkpoint] Course set to: %s",
            g_CurrentCourseName
        );

        ReplyToCommand(
            client,
            "[Checkpoint] Checkpoints: %d",
            g_CurrentCourse.Length
        );
    }
    else if (StrEqual(command, "checkpoint", false))
    {
        if (args < 2)
        {
            bool finished = UpdateCheckpoint();

            if (finished)
            {
                ReplyToCommand(
                    client,
                    "[Checkpoint] Course completed!"
                );
            }
            else
            {
                ReplyToCommand(
                    client,
                    "[Checkpoint] Advanced to checkpoint %d: %s",
                    g_CurrentCheckpointIndex,
                    g_CurrentCheckpoint.name
                );
            }

            return Plugin_Handled;
        }

        char buffer[32];

        GetCmdArg(
            2,
            buffer,
            sizeof(buffer)
        );

        int index = StringToInt(buffer);

        if (index < 0 || index >= g_CurrentCourse.Length)
        {
            ReplyToCommand(
                client,
                "[Checkpoint] Invalid checkpoint index: %d",
                index
            );

            return Plugin_Handled;
        }

        UpdateCheckpoint(index);

        ReplyToCommand(
            client,
            "[Checkpoint] Current checkpoint set to %d: %s",
            g_CurrentCheckpointIndex,
            g_CurrentCheckpoint.name
        );

        return Plugin_Handled;
    }
    else if (StrEqual(command, "load", false))
    {
        LoadAll();
        return Plugin_Handled;
    }
    else if (StrEqual(command, "settrigger", false))
    {
        // --------------------------------------------------------
        // settrigger <number>
        // --------------------------------------------------------

        if (args == 2)
        {
            char buffer[32];

            GetCmdArg(
                2,
                buffer,
                sizeof(buffer)
            );

            int triggerNumber = StringToInt(buffer);

            SetTrigger(
                "",
                triggerNumber,
                client
            );

            return Plugin_Handled;
        }


        // --------------------------------------------------------
        // settrigger <course> <number>
        // --------------------------------------------------------

        if (args == 3)
        {
            char courseName[128];
            char buffer[32];

            GetCmdArg(
                2,
                courseName,
                sizeof(courseName)
            );

            GetCmdArg(
                3,
                buffer,
                sizeof(buffer)
            );

            int triggerNumber = StringToInt(buffer);

            SetTrigger(
                courseName,
                triggerNumber,
                client
            );

            return Plugin_Handled;
        }

        return Plugin_Handled;
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