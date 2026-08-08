#include <sourcemod>
#include <sdktools>

#include "Debug/globals.inc"
#include "Shared/courseRuntime.inc"
#include "Debug/checkpoint.inc"
#include "Debug/trigger.inc"

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
        Timer_DrawCheckpoints,
        _,
        TIMER_REPEAT | TIMER_FLAG_NO_MAPCHANGE
    );

    CreateTimer(
        DRAW_INTERVAL,
        Timer_DrawTriggers,
        _,
        TIMER_REPEAT | TIMER_FLAG_NO_MAPCHANGE
    );
}


// ============================================================
// Map start
// ============================================================

public void OnMapStart()
{
    g_CheckpointDrawEnabled = false;
    g_TriggerDrawEnabled = false;

    g_BeamSprite =
        PrecacheModel(
            "materials/sprites/laserbeam.vmt"
        );
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
            "  sm_debug printCheckpoints"
        );

        ReplyToCommand(
            client,
            "  sm_debug printTriggers"
        );

        ReplyToCommand(
            client,
            "  sm_debug drawCheckpoints"
        );

        ReplyToCommand(
            client,
            "  sm_debug drawTriggers"
        );

        return Plugin_Handled;
    }

    char command[32];

    GetCmdArg(
        1,
        command,
        sizeof(command)
    );


    if (StrEqual(command, "drawCheckpoints", false))
    {
        g_CheckpointDrawEnabled = !g_CheckpointDrawEnabled;

        ReplyToCommand(
            client,
            "[Debug] Drawing %s.",
            g_CheckpointDrawEnabled ? "enabled" : "disabled"
        );
    }
    else if (StrEqual(command, "drawTriggers", false))
    {
        g_TriggerDrawEnabled = !g_TriggerDrawEnabled;

        ReplyToCommand(
            client,
            "[Debug] Drawing %s.",
            g_TriggerDrawEnabled ? "enabled" : "disabled"
        );
    }
    
    else
    {
        ReplyToCommand(
            client,
            "[Debug] Unknown command: %s",
            command
        );
    }

    return Plugin_Handled;
}

public Action Timer_DrawCheckpoints(
    Handle timer
)
{
    if (!g_CheckpointDrawEnabled)
    {
        return Plugin_Continue;
    }

    DrawCheckpoints();

    return Plugin_Continue;
}

public Action Timer_DrawTriggers(
    Handle timer
)
{
    if (!g_TriggerDrawEnabled)
    {
        return Plugin_Continue;
    }

    DrawTriggers();

    return Plugin_Continue;
}

public void CourseRuntime_OnCourseDataUpdated()
{
    g_Checkpoints =
        CourseRuntime_GetCurrentCourseCheckpoints();

    g_Triggers =
        CourseRuntime_GetCurrentCourseTriggers();
}