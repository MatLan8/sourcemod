#include <sourcemod>
#include <sdktools>
#include <sdkhooks>

// ============================================================
// Runtime Trigger Builder
//
// Workflow:
//
//     sm_trigger point
//
//     Fly to first corner.
//     sm_trigger point
//
//     Fly to opposite corner.
//     sm_trigger point
//
//     sm_trigger create <name>
//
// The two points represent opposite corners of the trigger.
//
// The trigger is created at the center of the two points and
// its local mins/maxs are calculated from those world points.
//
// ============================================================


// ============================================================
// Globals
// ============================================================

float g_TriggerPoints[2][3];
int g_TriggerPointCount = 0;

#define TRIGGER_MODEL "models/error.mdl"


// ============================================================
// Plugin info
// ============================================================

public Plugin myinfo =
{
    name = "Runtime Trigger Builder",
    author = "MatLan8",
    description = "Creates runtime trigger_multiple entities from two world-space corners.",
    version = "1.0"
};


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
        "[Trigger] Runtime trigger builder loaded."
    );
}


// ============================================================
// Map start
// ============================================================

public void OnMapStart()
{
    PrecacheModel(
        TRIGGER_MODEL,
        true
    );

    g_TriggerPointCount = 0;

    PrintToServer(
        "[Trigger] Precached %s",
        TRIGGER_MODEL
    );
}


// ============================================================
// Main command
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
    if (client <= 0 || !IsClientInGame(client))
    {
        return Plugin_Handled;
    }

    if (args < 1)
    {
        PrintTriggerUsage(client);
        return Plugin_Handled;
    }

    char command[32];

    GetCmdArg(
        1,
        command,
        sizeof(command)
    );

    // --------------------------------------------------------
    // Save point
    // --------------------------------------------------------

    if (StrEqual(command, "point", false))
    {
        if (g_TriggerPointCount >= 2)
        {
            ReplyToCommand(
                client,
                "[Trigger] Already have two points."
            );

            ReplyToCommand(
                client,
                "[Trigger] Use sm_trigger reset first."
            );

            return Plugin_Handled;
        }

        float origin[3];

        GetClientAbsOrigin(
            client,
            origin
        );

        g_TriggerPoints[g_TriggerPointCount][0] = origin[0];
        g_TriggerPoints[g_TriggerPointCount][1] = origin[1];
        g_TriggerPoints[g_TriggerPointCount][2] = origin[2];

        g_TriggerPointCount++;

        ReplyToCommand(
            client,
            "[Trigger] Point %d saved:",
            g_TriggerPointCount
        );

        ReplyToCommand(
            client,
            "    %.3f %.3f %.3f",
            origin[0],
            origin[1],
            origin[2]
        );

        return Plugin_Handled;
    }

    // --------------------------------------------------------
    // Reset
    // --------------------------------------------------------

    if (StrEqual(command, "reset", false))
    {
        g_TriggerPointCount = 0;

        ReplyToCommand(
            client,
            "[Trigger] Points reset."
        );

        return Plugin_Handled;
    }

    // --------------------------------------------------------
    // Create
    // --------------------------------------------------------

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

        if (g_TriggerPointCount != 2)
        {
            ReplyToCommand(
                client,
                "[Trigger] You need exactly two points first."
            );

            return Plugin_Handled;
        }

        char name[128];

        GetCmdArg(
            2,
            name,
            sizeof(name)
        );

        CreateRuntimeTrigger(
            client,
            name
        );

        return Plugin_Handled;
    }

    // --------------------------------------------------------
    // Unknown command
    // --------------------------------------------------------

    PrintTriggerUsage(client);

    return Plugin_Handled;
}


// ============================================================
// Create runtime trigger
// ============================================================

void CreateRuntimeTrigger(
    int client,
    const char[] name
)
{
    float worldMins[3];
    float worldMaxs[3];

    // --------------------------------------------------------
    // Normalize the two world-space corners.
    // --------------------------------------------------------

    for (int axis = 0; axis < 3; axis++)
    {
        if (g_TriggerPoints[0][axis] < g_TriggerPoints[1][axis])
        {
            worldMins[axis] = g_TriggerPoints[0][axis];
            worldMaxs[axis] = g_TriggerPoints[1][axis];
        }
        else
        {
            worldMins[axis] = g_TriggerPoints[1][axis];
            worldMaxs[axis] = g_TriggerPoints[0][axis];
        }
    }

    // --------------------------------------------------------
    // Calculate center of the box.
    // --------------------------------------------------------

    float origin[3];

    for (int axis = 0; axis < 3; axis++)
    {
        origin[axis] =
            (worldMins[axis] + worldMaxs[axis]) * 0.5;
    }

    // --------------------------------------------------------
    // Calculate local bounds.
    //
    // Since origin is the center:
    //
    // local mins = world mins - origin
    // local maxs = world maxs - origin
    //
    // This guarantees:
    //
    // mins <= 0
    // maxs >= 0
    // --------------------------------------------------------

    float localMins[3];
    float localMaxs[3];

    for (int axis = 0; axis < 3; axis++)
    {
        localMins[axis] =
            worldMins[axis] - origin[axis];

        localMaxs[axis] =
            worldMaxs[axis] - origin[axis];
    }

    // --------------------------------------------------------
    // Create trigger entity.
    // --------------------------------------------------------

    int entity = CreateEntityByName(
        "trigger_multiple"
    );

    if (entity == -1)
    {
        ReplyToCommand(
            client,
            "[Trigger] ERROR: CreateEntityByName failed."
        );

        return;
    }

    // --------------------------------------------------------
    // Target name.
    // --------------------------------------------------------

    DispatchKeyValue(
        entity,
        "targetname",
        name
    );

    // --------------------------------------------------------
    // Trigger configuration.
    //
    // Spawnflag 64 = clients.
    // Wait 0 = can trigger again immediately.
    // --------------------------------------------------------

    DispatchKeyValue(
        entity,
        "spawnflags",
        "64"
    );

    DispatchKeyValue(
        entity,
        "wait",
        "0"
    );

    DispatchKeyValue(
        entity,
        "StartDisabled",
        "0"
    );

    // --------------------------------------------------------
    // Spawn.
    // --------------------------------------------------------

    if (!DispatchSpawn(entity))
    {
        ReplyToCommand(
            client,
            "[Trigger] ERROR: DispatchSpawn failed."
        );

        AcceptEntityInput(
            entity,
            "Kill"
        );

        return;
    }

    // --------------------------------------------------------
    // Activate.
    // --------------------------------------------------------

    ActivateEntity(entity);

    // --------------------------------------------------------
    // Move entity to center of requested volume.
    //
    // This must happen before calculating the effective local
    // bounds relative to the entity origin.
    // --------------------------------------------------------

    TeleportEntity(
        entity,
        origin,
        NULL_VECTOR,
        NULL_VECTOR
    );

    // --------------------------------------------------------
    // IMPORTANT:
    //
    // trigger_multiple is a brush entity.
    //
    // Give it a model so the engine initializes its brush/
    // collision representation.
    // --------------------------------------------------------

    SetEntityModel(
        entity,
        TRIGGER_MODEL
    );

    // --------------------------------------------------------
    // Set collision bounds.
    // --------------------------------------------------------

    SetEntPropVector(
        entity,
        Prop_Send,
        "m_vecMins",
        localMins
    );

    SetEntPropVector(
        entity,
        Prop_Send,
        "m_vecMaxs",
        localMaxs
    );

    // --------------------------------------------------------
    // SOLID_BBOX.
    // --------------------------------------------------------

    SetEntProp(
        entity,
        Prop_Send,
        "m_nSolidType",
        2
    );

    // --------------------------------------------------------
    // Trigger solid flags.
    //
    // FSOLID_TRIGGER = 8
    // --------------------------------------------------------

    SetEntProp(
        entity,
        Prop_Send,
        "m_usSolidFlags",
        8
    );

    // --------------------------------------------------------
    // Collision group.
    //
    // Keep the trigger as a trigger rather than a blocking
    // physical object.
    // --------------------------------------------------------

    SetEntProp(
        entity,
        Prop_Send,
        "m_CollisionGroup",
        1
    );

    // --------------------------------------------------------
    // Reapply the bounds after model/solid initialization.
    //
    // SetEntityModel() can alter the entity's collision state,
    // so explicitly set our desired bounds afterward.
    // --------------------------------------------------------

    SetEntPropVector(
        entity,
        Prop_Send,
        "m_vecMins",
        localMins
    );

    SetEntPropVector(
        entity,
        Prop_Send,
        "m_vecMaxs",
        localMaxs
    );

    SetEntProp(
        entity,
        Prop_Send,
        "m_nSolidType",
        2
    );

    // --------------------------------------------------------
    // Hook touches.
    // --------------------------------------------------------

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

    // --------------------------------------------------------
    // Diagnostics.
    // --------------------------------------------------------

    PrintToServer(
        "========================================"
    );

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
        localMins[0],
        localMins[1],
        localMins[2]
    );

    PrintToServer(
        "[Trigger] Local maxs: %.3f %.3f %.3f",
        localMaxs[0],
        localMaxs[1],
        localMaxs[2]
    );

    PrintToServer(
        "[Trigger] Model: %s",
        TRIGGER_MODEL
    );

    PrintToServer(
        "========================================"
    );

    ReplyToCommand(
        client,
        "[Trigger] Created \"%s\" (entity %d).",
        name,
        entity
    );
}


// ============================================================
// Start touch
// ============================================================

public void OnTriggerStartTouch(
    int entity,
    int other
)
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


// ============================================================
// End touch
// ============================================================

public void OnTriggerEndTouch(
    int entity,
    int other
)
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


// ============================================================
// Usage
// ============================================================

void PrintTriggerUsage(
    int client
)
{
    ReplyToCommand(
        client,
        "[Trigger] Commands:"
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
}