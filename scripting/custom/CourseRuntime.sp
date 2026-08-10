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
#include "CourseRuntime/captureTriggers.inc"


public APLRes AskPluginLoad2(
    Handle myself,
    bool late,
    char[] error,
    int err_max
)
{
    RegPluginLibrary("CourseRuntime");

    RegisterNatives();

    return APLRes_Success;
}


public void OnPluginStart()
{
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
    ClearCourseRuntimeData();

    LoadCheckpoints();
    LoadTriggers();
    SetCourse();

    FireCourseDataUpdated();
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
        FireCourseDataUpdated();
        return Plugin_Handled;
    }
    
    else if (StrEqual(command, "loadTriggers", false))
    {
        LoadTriggers();
        FireCourseDataUpdated();
        return Plugin_Handled;
    }
    else if (StrEqual(command, "setCourse", false))
    {
        if (args == 1)
        {
            SetCourse(client);
            FireCourseDataUpdated();
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
            FireCourseDataUpdated();
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
        FireCourseDataUpdated();

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
    FireCourseDataUpdated();
}

public void TriggerManager_OnConfigChanged()
{
    LoadTriggers();
    FireCourseDataUpdated();
}

void ClearCourseRuntimeData()
{
    if (g_AllCoursesCheckpoints != null)
    {
        for (int i = 0; i < g_AllCoursesCheckpoints.Length; i++)
        {
            CourseCheckpoints course;

            g_AllCoursesCheckpoints.GetArray(
                i,
                course
            );

            if (course.checkpoints != null)
            {
                delete course.checkpoints;
            }
        }

        delete g_AllCoursesCheckpoints;
        g_AllCoursesCheckpoints = null;
    }

    if (g_AllCoursesTriggers != null)
    {
        for (int i = 0; i < g_AllCoursesTriggers.Length; i++)
        {
            CourseTriggers course;

            g_AllCoursesTriggers.GetArray(
                i,
                course
            );

            if (course.triggers != null)
            {
                delete course.triggers;
            }
        }

        delete g_AllCoursesTriggers;
        g_AllCoursesTriggers = null;
    }

    if (g_CaptureTriggers != null)
    {
        delete g_CaptureTriggers;
        g_CaptureTriggers = null;
    }

    g_CurrentCourseIndex = -1;
    g_CurrentCheckpointIndex = -1;
    g_CurrentTriggerIndex = -1;
}