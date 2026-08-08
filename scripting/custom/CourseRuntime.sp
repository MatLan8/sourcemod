#include <sourcemod>
#include <sdktools>
#include "CourseRuntime/globals.inc"
#include "CourseRuntime/checkpointData.inc"
#include "CourseRuntime/triggerEvents.inc"
#include "CourseRuntime/triggerData.inc"
#include "CourseRuntime/natives.inc"
#include "CourseRuntime/events.inc"
#include "CourseRuntime/course.inc"
#include "Trigger/event.inc"
#include "Checkpoint/event.inc"

public void OnPluginStart()
{
    RegisterNatives();
    RegisterForwards();

    RegConsoleCmd(
        "sm_courseRuntime",
        Command_CourseRuntime
    );
}

public void OnMapStart()
{
    PrecacheModel(
        TRIGGER_MODEL,
        true
    );
    LoadCheckpoints();
    LoadTriggers();
}

public Action Command_CourseRuntime(
    int client,
    int args
)
{
    if (args < 1)
    {
        ReplyToCommand(
            client,
            "[CourseRuntime] Usage:"
        );

        ReplyToCommand(
            client,
            "  sm_courseRuntime loadCheckpoints"
        );

        ReplyToCommand(
            client,
            "  sm_courseRuntime loadTriggers"
        );

        ReplyToCommand(
            client,
            "  sm_courseRuntime setCourse <courseName> <courseIndex>"
        );

        
        return Plugin_Handled;
    }

    char command[32];

    GetCmdArg(
        1,
        command,
        sizeof(command)
    );


    if (StrEqual(command, "loadCheckpoints", false))
    {
        LoadCheckpoints();
        return Plugin_Handled;
    }
    
    else if (StrEqual(command, "loadTriggers", false))
    {
        LoadTriggers();
        return Plugin_Handled;
    }
    else if (StrEqual(command, "setCourse", false))
    {
        if (args == 1)
        {
            SetCourse(client);
            return Plugin_Handled;
        }
        if (args == 2)
        {
            char argument[128];

            GetCmdArg(
                2,
                argument,
                sizeof(argument)
            );

            if (IsNumericString(argument))
            {
                SetCourse(
                    client,
                    "",
                    StringToInt(argument)
                );
            }
            else
            {
                SetCourse(
                    client,
                    argument,
                    -1
                );
            }

            return Plugin_Handled;
        }

        char courseName[128];
        char indexString[32];

        GetCmdArg(
            2,
            courseName,
            sizeof(courseName)
        );

        GetCmdArg(
            3,
            indexString,
            sizeof(indexString)
        );

        SetCourse(
            client,
            courseName,
            StringToInt(indexString)
        );

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

void RegisterForwards()
{
    g_OnCourseDataUpdated = CreateGlobalForward(
        "CourseRuntime_OnCourseDataUpdated",
        ET_Ignore
    );
}

public void CheckpointManager_OnConfigChanged()
{
    LoadCheckpoints();
}

public void TriggerManager_OnConfigChanged()
{
    LoadTriggers();
}

