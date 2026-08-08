#include <sourcemod>
#include <sdktools>

#include "Trigger/globals.inc"
#include "Trigger/core.inc"
#include "Trigger/file.inc"
#include "Trigger/event.inc"

public Plugin myinfo =
{
    name = "RocketJumpAI Trigger Manager",
    author = "MatLan8",
    description = "Manages RocketJumpAI trigger configuration data.",
    version = "1.0"
};


public void OnPluginStart()
{
    RegConsoleCmd(
        "sm_trigger",
        Command_Trigger
    );

    PrintToServer(
        "[TriggerManager] Loaded."
    );

    g_OnTriggerConfigChanged = CreateGlobalForward(
        "TriggerManager_OnConfigChanged",
        ET_Ignore
    );
}

public Action Command_Trigger(
    int client,
    int args
)
{
    if (args < 1)
    {
        ReplyToCommand(
            client,
            "[Trigger] Usage:"
        );

        ReplyToCommand(
            client,
            "  sm_trigger setpoint"
        );

        ReplyToCommand(
            client,
            "  sm_trigger reset"
        );

        ReplyToCommand(
            client,
            "  sm_trigger save <number>"
        );

        ReplyToCommand(
            client,
            "  sm_trigger save <course> <number>"
        );

        ReplyToCommand(
            client,
            "  sm_trigger update <number>"
        );

        ReplyToCommand(
            client,
            "  sm_trigger update <course> <number>"
        );

        ReplyToCommand(
            client,
            "  sm_trigger delete <number>"
        );

        ReplyToCommand(
            client,
            "  sm_trigger delete <course> <number>"
        );

        return Plugin_Handled;
    }

    char command[32];

    GetCmdArg(
        1,
        command,
        sizeof(command)
    );

    if (StrEqual(command, "setpoint", false))
    {
        SetPoint(client);

        return Plugin_Handled;
    }

    if (StrEqual(command, "reset", false))
    {
        Reset(client);

        return Plugin_Handled;
    }

    if (StrEqual(command, "save", false))
    {
        // ----------------------------------------------------
        // save <number>
        // ----------------------------------------------------

        if (args == 2)
        {
            char buffer[32];

            GetCmdArg(
                2,
                buffer,
                sizeof(buffer)
            );

            int triggerNumber = StringToInt(buffer);

            Save(
                client,
                "",
                triggerNumber
            );

            return Plugin_Handled;
        }

        // ----------------------------------------------------
        // save <course> <number>
        // ----------------------------------------------------

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

            Save(
                client,
                courseName,
                triggerNumber
            );

            return Plugin_Handled;
        }

        ReplyToCommand(
            client,
            "[Trigger] Usage: sm_trigger save <number>"
        );

        ReplyToCommand(
            client,
            "[Trigger] Or: sm_trigger save <course> <number>"
        );

        return Plugin_Handled;
    }

    if (StrEqual(command, "update", false))
    {
        // ----------------------------------------------------
        // update <number>
        // ----------------------------------------------------

        if (args == 2)
        {
            char buffer[32];

            GetCmdArg(
                2,
                buffer,
                sizeof(buffer)
            );

            int triggerNumber = StringToInt(buffer);

            Update(
                client,
                "",
                triggerNumber
            );

            return Plugin_Handled;
        }

        // ----------------------------------------------------
        // update <course> <number>
        // ----------------------------------------------------

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

            Update(
                client,
                courseName,
                triggerNumber
            );

            return Plugin_Handled;
        }

        ReplyToCommand(
            client,
            "[Trigger] Usage: sm_trigger update <number>"
        );

        ReplyToCommand(
            client,
            "[Trigger] Or: sm_trigger update <course> <number>"
        );

        return Plugin_Handled;
    }
    
    if (StrEqual(command, "delete", false))
    {
        // ----------------------------------------------------
        // delete <number>
        // ----------------------------------------------------

        if (args == 2)
        {
            char buffer[32];

            GetCmdArg(
                2,
                buffer,
                sizeof(buffer)
            );

            int triggerNumber = StringToInt(buffer);

            Delete(
                client,
                "",
                triggerNumber
            );

            return Plugin_Handled;
        }

        // ----------------------------------------------------
        // delete <course> <number>
        // ----------------------------------------------------

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

            Delete(
                client,
                courseName,
                triggerNumber
            );

            return Plugin_Handled;
        }

        ReplyToCommand(
            client,
            "[Trigger] Usage: sm_trigger delete <number>"
        );

        ReplyToCommand(
            client,
            "[Trigger] Or: sm_trigger delete <course> <number>"
        );

        return Plugin_Handled;
    }

    ReplyToCommand(
        client,
        "[Trigger] Unknown command: %s",
        command
    );

    return Plugin_Handled;
}

void FireConfigChanged()
{
    if (g_OnTriggerConfigChanged == null)
    {
        return;
    }

    Call_StartForward(
        g_OnTriggerConfigChanged
    );

    Call_Finish();
}