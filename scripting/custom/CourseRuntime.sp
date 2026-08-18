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

    HookEvent("teamplay_round_start", Event_OnRoundStart, EventHookMode_PostNoCopy);
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
    FireCheckpointEntered();
}


public void Event_OnRoundStart(Event event, const char[] name, bool dontBroadcast)
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
    FireCheckpointEntered();
}


void DebugListRjaiEntities()
{
    PrintToServer("========================================");
    PrintToServer("[Trigger DEBUG] Searching for rjai entities...");
    PrintToServer("========================================");

    int count = 0;

    int entity = -1;

    while ((entity = FindEntityByClassname(
        entity,
        "trigger_multiple"
    )) != -1)
    {
        if (!IsValidEntity(entity))
        {
            continue;
        }

        char targetname[128];

        GetEntPropString(
            entity,
            Prop_Data,
            "m_iName",
            targetname,
            sizeof(targetname)
        );

        // Only show our runtime triggers.
        if (StrContains(
                targetname,
                "rjai",
                false
            ) == -1)
        {
            continue;
        }

        float origin[3];
        float mins[3];
        float maxs[3];

        GetEntPropVector(
            entity,
            Prop_Data,
            "m_vecOrigin",
            origin
        );

        GetEntPropVector(
            entity,
            Prop_Send,
            "m_vecMins",
            mins
        );

        GetEntPropVector(
            entity,
            Prop_Send,
            "m_vecMaxs",
            maxs
        );

        int solid = GetEntProp(
            entity,
            Prop_Send,
            "m_nSolidType"
        );

        int flags = GetEntProp(
            entity,
            Prop_Send,
            "m_usSolidFlags"
        );

        int collision = GetEntProp(
            entity,
            Prop_Send,
            "m_CollisionGroup"
        );

        PrintToServer(
            "[Trigger DEBUG] entity=%d name=\"%s\" origin=(%.1f %.1f %.1f)",
            entity,
            targetname,
            origin[0],
            origin[1],
            origin[2]
        );

        PrintToServer(
            "[Trigger DEBUG]    mins=(%.1f %.1f %.1f) maxs=(%.1f %.1f %.1f)",
            mins[0],
            mins[1],
            mins[2],
            maxs[0],
            maxs[1],
            maxs[2]
        );

        PrintToServer(
            "[Trigger DEBUG]    solid=%d flags=%d collision=%d",
            solid,
            flags,
            collision
        );

        count++;
    }

    PrintToServer(
        "[Trigger DEBUG] Found %d rjai trigger_multiple entities.",
        count
    );

    PrintToServer("========================================");
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
        SetCourse(client);
        FireCourseDataUpdated();
        FireCheckpointEntered();
        return Plugin_Handled;
    }

    else if (StrEqual(command, "debug", false))
    {
        DebugListRjaiEntities();
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
            FireCheckpointEntered();
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
            FireCheckpointEntered();
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
        FireCheckpointEntered();
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
    g_OnCheckpointEntered = CreateGlobalForward(
        "CourseRuntime_OnCheckpointEntered",
        ET_Ignore
    );
}

public void CheckpointManager_OnConfigChanged()
{
    LoadCheckpoints();
    SetCourse();
    FireCourseDataUpdated();
    FireCheckpointEntered();
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

    g_CurrentCheckpoint.name[0] = '\0';
    g_CurrentCheckpoint.coordinates[0] = 0.0;
    g_CurrentCheckpoint.coordinates[1] = 0.0;
    g_CurrentCheckpoint.coordinates[2] = 0.0;
}
