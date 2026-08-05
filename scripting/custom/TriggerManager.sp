#include <sourcemod>
#include <sdktools>
#include <sdkhooks>

public Plugin myinfo =
{
    name = "Trigger Creator Test",
    author = "MatLan8",
    description = "Creates runtime trigger_multiple volumes from two points",
    version = "1.0"
};


// ============================================================
// Stored trigger corners
// ============================================================

float g_Corner1[3];
float g_Corner2[3];

bool g_HasCorner1 = false;
bool g_HasCorner2 = false;


// ============================================================
// Plugin start
// ============================================================

public void OnPluginStart()
{
    RegConsoleCmd(
        "sm_trigger",
        Command_Trigger
    );

    PrintToServer(
        "[Trigger] Plugin loaded."
    );
}


// ============================================================
// Main command
//
// Usage:
//
// sm_trigger point
// sm_trigger create <name>
// sm_trigger reset
// ============================================================

public Action Command_Trigger(
    int client,
    int args
)
{
    if (client <= 0)
    {
        return Plugin_Handled;
    }

    if (args < 1)
    {
        ReplyToCommand(
            client,
            "[Trigger] Usage:"
        );

        ReplyToCommand(
            client,
            "  sm_trigger point"
        );

        ReplyToCommand(
            client,
            "  sm_trigger create <name>"
        );

        ReplyToCommand(
            client,
            "  sm_trigger reset"
        );

        return Plugin_Handled;
    }

    char command[32];

    GetCmdArg(
        1,
        command,
        sizeof(command)
    );

    // ========================================================
    // Save point
    // ========================================================

    if (StrEqual(command, "point", false))
    {
        float position[3];

        GetClientAbsOrigin(
            client,
            position
        );

        // ----------------------------------------------------
        // First point
        // ----------------------------------------------------

        if (!g_HasCorner1)
        {
            g_Corner1[0] = position[0];
            g_Corner1[1] = position[1];
            g_Corner1[2] = position[2];

            g_HasCorner1 = true;

            ReplyToCommand(
                client,
                "[Trigger] Corner 1 saved: %.3f %.3f %.3f",
                position[0],
                position[1],
                position[2]
            );

            return Plugin_Handled;
        }

        // ----------------------------------------------------
        // Second point
        // ----------------------------------------------------

        if (!g_HasCorner2)
        {
            g_Corner2[0] = position[0];
            g_Corner2[1] = position[1];
            g_Corner2[2] = position[2];

            g_HasCorner2 = true;

            ReplyToCommand(
                client,
                "[Trigger] Corner 2 saved: %.3f %.3f %.3f",
                position[0],
                position[1],
                position[2]
            );

            ReplyToCommand(
                client,
                "[Trigger] Both corners saved."
            );

            ReplyToCommand(
                client,
                "[Trigger] Use: sm_trigger create <name>"
            );

            return Plugin_Handled;
        }

        // ----------------------------------------------------
        // Already have both points
        // ----------------------------------------------------

        ReplyToCommand(
            client,
            "[Trigger] Both corners are already set."
        );

        ReplyToCommand(
            client,
            "[Trigger] Use sm_trigger reset first."
        );

        return Plugin_Handled;
    }

    // ========================================================
    // Reset
    // ========================================================

    if (StrEqual(command, "reset", false))
    {
        g_HasCorner1 = false;
        g_HasCorner2 = false;

        ReplyToCommand(
            client,
            "[Trigger] Corner data reset."
        );

        return Plugin_Handled;
    }

    // ========================================================
    // Create trigger
    // ========================================================

    if (StrEqual(command, "create", false))
    {
        if (args < 2)
        {
            ReplyToCommand(
                client,
                "[Trigger] Usage: sm_trigger create <name>"
            );

            return Plugin_Handled;
        }

        if (!g_HasCorner1 || !g_HasCorner2)
        {
            ReplyToCommand(
                client,
                "[Trigger] You must save two corners first."
            );

            return Plugin_Handled;
        }

        char triggerName[128];

        GetCmdArg(
            2,
            triggerName,
            sizeof(triggerName)
        );

        CreateTrigger(
            triggerName
        );

        return Plugin_Handled;
    }

    // ========================================================
    // Unknown command
    // ========================================================

    ReplyToCommand(
        client,
        "[Trigger] Unknown command: %s",
        command
    );

    return Plugin_Handled;
}


// ============================================================
// Create runtime trigger
// ============================================================

void CreateTrigger(const char[] name)
{
    // ========================================================
    // Calculate world-space bounds
    // ========================================================

    float worldMins[3];
    float worldMaxs[3];

    for (int i = 0; i < 3; i++)
    {
        worldMins[i] = g_Corner1[i];
        worldMaxs[i] = g_Corner2[i];

        if (worldMins[i] > worldMaxs[i])
        {
            float temp = worldMins[i];
            worldMins[i] = worldMaxs[i];
            worldMaxs[i] = temp;
        }
    }

    // ========================================================
    // Calculate center
    // ========================================================

    float origin[3];

    origin[0] = (worldMins[0] + worldMaxs[0]) * 0.5;
    origin[1] = (worldMins[1] + worldMaxs[1]) * 0.5;
    origin[2] = (worldMins[2] + worldMaxs[2]) * 0.5;

    // ========================================================
    // Convert to local bounds
    // ========================================================

    float mins[3];
    float maxs[3];

    for (int i = 0; i < 3; i++)
    {
        mins[i] = worldMins[i] - origin[i];
        maxs[i] = worldMaxs[i] - origin[i];
    }

    // ========================================================
    // Create entity
    // ========================================================

    int entity = CreateEntityByName("trigger_multiple");

    if (entity == -1)
    {
        PrintToServer(
            "[Trigger] ERROR: Failed to create trigger_multiple."
        );

        return;
    }

    // ========================================================
    // Target name
    // ========================================================

    DispatchKeyValue(
        entity,
        "targetname",
        name
    );

    // ========================================================
    // Spawn flags
    //
    // 1 = Clients
    // ========================================================

    DispatchKeyValue(
        entity,
        "spawnflags",
        "1"
    );

    DispatchKeyValue(
        entity,
        "StartDisabled",
        "0"
    );

    // ========================================================
    // Spawn
    // ========================================================

    DispatchSpawn(entity);
    ActivateEntity(entity);

    // ========================================================
    // Position
    // ========================================================

    TeleportEntity(
        entity,
        origin,
        NULL_VECTOR,
        NULL_VECTOR
    );

    // ========================================================
    // IMPORTANT:
    // Give the entity a model index so the engine creates
    // the collision representation for the trigger.
    // ========================================================

    SetEntProp(
        entity,
        Prop_Send,
        "m_nModelIndex",
        1
    );

    // ========================================================
    // Solid type = SOLID_BBOX
    // ========================================================

    SetEntProp(
        entity,
        Prop_Send,
        "m_nSolidType",
        2
    );

    // ========================================================
    // Solid flags
    // ========================================================

    SetEntProp(
        entity,
        Prop_Send,
        "m_usSolidFlags",
        152
    );

    // ========================================================
    // Collision group
    // ========================================================

    SetEntProp(
        entity,
        Prop_Send,
        "m_CollisionGroup",
        11
    );

    // ========================================================
    // Collision bounds
    // ========================================================

    SetEntPropVector(
        entity,
        Prop_Send,
        "m_vecMins",
        mins
    );

    SetEntPropVector(
        entity,
        Prop_Send,
        "m_vecMaxs",
        maxs
    );

    // ========================================================
    // Hook touch
    // ========================================================

    SDKHook(
        entity,
        SDKHook_StartTouch,
        OnTriggerStartTouch
    );

    SDKHook(
        entity,
        SDKHook_EndTouch,
        OnTriggerEndTouch
    );

    // ========================================================
    // Enable
    // ========================================================

    AcceptEntityInput(
        entity,
        "Enable"
    );

    // ========================================================
    // Debug
    // ========================================================

    PrintToServer(
        "[Trigger] Created \"%s\"",
        name
    );

    PrintToServer(
        "[Trigger] Entity: %d",
        entity
    );

    PrintToServer(
        "[Trigger] World mins: %.3f %.3f %.3f",
        worldMins[0],
        worldMins[1],
        worldMins[2]
    );

    PrintToServer(
        "[Trigger] World maxs: %.3f %.3f %.3f",
        worldMaxs[0],
        worldMaxs[1],
        worldMaxs[2]
    );

    PrintToServer(
        "[Trigger] Origin: %.3f %.3f %.3f",
        origin[0],
        origin[1],
        origin[2]
    );

    PrintToServer(
        "[Trigger] Local mins: %.3f %.3f %.3f",
        mins[0],
        mins[1],
        mins[2]
    );

    PrintToServer(
        "[Trigger] Local maxs: %.3f %.3f %.3f",
        maxs[0],
        maxs[1],
        maxs[2]
    );
}


// ============================================================
// Trigger StartTouch
// ============================================================

public void OnTriggerStartTouch(int entity, int other)
{
    if (other < 1 || other > MaxClients)
    {
        return;
    }

    if (!IsClientInGame(other))
    {
        return;
    }

    char name[128];

    GetEntPropString(
        entity,
        Prop_Data,
        "m_iName",
        name,
        sizeof(name)
    );

    PrintToChat(
        other,
        "[Trigger] ENTER: %s",
        name
    );

    PrintToServer(
        "[Trigger] Client %d ENTERED \"%s\" (entity %d)",
        other,
        name,
        entity
    );
}


public void OnTriggerEndTouch(int entity, int other)
{
    if (other < 1 || other > MaxClients)
    {
        return;
    }

    if (!IsClientInGame(other))
    {
        return;
    }

    char name[128];

    GetEntPropString(
        entity,
        Prop_Data,
        "m_iName",
        name,
        sizeof(name)
    );

    PrintToChat(
        other,
        "[Trigger] EXIT: %s",
        name
    );

    PrintToServer(
        "[Trigger] Client %d EXITED \"%s\" (entity %d)",
        other,
        name,
        entity
    );
}