#if defined _shared_trigger_included
    #endinput
#endif
#define _shared_trigger_included

#include <sdkhooks>

void LoadAll()
{
    // --------------------------------------------------------
    // Initialize / clear runtime trigger list.
    // --------------------------------------------------------

    if (g_Triggers == null)
    {
        g_Triggers = new ArrayList(sizeof(TriggerData));
    }
    else
    {
        g_Triggers.Clear();
    }

    // --------------------------------------------------------
    // Get current map.
    // --------------------------------------------------------

    char mapName[128];

    GetCurrentMap(
        mapName,
        sizeof(mapName)
    );

    // --------------------------------------------------------
    // Build trigger CFG path.
    //
    // Example:
    //
    // data/MapConfigs/jump_map_triggers.cfg
    // --------------------------------------------------------

    char path[PLATFORM_MAX_PATH];

    BuildPath(
        Path_SM,
        path,
        sizeof(path),
        "%s/%s_triggers.cfg",
        MAP_CONFIG_PATH,
        mapName
    );

    // --------------------------------------------------------
    // Make sure the file exists.
    // --------------------------------------------------------

    if (!FileExists(path))
    {
        PrintToServer(
            "[Trigger] No trigger file found: %s",
            path
        );

        g_CurrentTriggerIndex = -1;

        return;
    }

    // --------------------------------------------------------
    // Root is "Triggers".
    // --------------------------------------------------------

    KeyValues kv = new KeyValues("Triggers");

    if (!kv.ImportFromFile(path))
    {
        PrintToServer(
            "[Trigger] ERROR: Failed to read trigger file: %s",
            path
        );

        delete kv;

        g_CurrentTriggerIndex = -1;

        return;
    }

    kv.Rewind();

    // --------------------------------------------------------
    // Check whether this is a course-based trigger file.
    //
    // Course structure:
    //
    // Triggers
    // {
    //     Courses
    //     {
    //         Basic
    //         {
    //             rjai_0
    //             rjai_1
    //         }
    //     }
    // }
    // --------------------------------------------------------

    bool hasCourses = kv.JumpToKey(
        "Courses",
        false
    );

    if (!hasCourses)
    {
        // ====================================================
        // FLAT TRIGGER LIST
        // ====================================================

        kv.Rewind();

        if (!kv.GotoFirstSubKey(false))
        {
            PrintToServer(
                "[Trigger] Trigger file contains no triggers."
            );

            delete kv;

            g_CurrentTriggerIndex = -1;

            return;
        }

        do
        {
            char triggerName[128];

            kv.GetSectionName(
                triggerName,
                sizeof(triggerName)
            );

            // ------------------------------------------------
            // Only accept rjai_* entries.
            // ------------------------------------------------

            if (StrContains(
                    triggerName,
                    "rjai_",
                    false
                ) != 0)
            {
                continue;
            }

            TriggerData trigger;

            LoadTriggerFromKeyValues(
                kv,
                trigger
            );

            // ------------------------------------------------
            // Flat trigger keeps its original name.
            // ------------------------------------------------

            strcopy(
                trigger.name,
                sizeof(trigger.name),
                triggerName
            );

            // ------------------------------------------------
            // Create runtime entity.
            // ------------------------------------------------

            int entity = CreateRuntimeTrigger(
                trigger
            );

            if (entity == -1)
            {
                continue;
            }

            trigger.entity = entity;

            g_Triggers.PushArray(
                trigger
            );

        } while (kv.GotoNextKey(false));
    }
    else
    {
        // ====================================================
        // COURSE-BASED TRIGGER LIST
        // ====================================================

        // We are currently inside "Courses".

        if (!kv.GotoFirstSubKey(false))
        {
            PrintToServer(
                "[Trigger] Courses section contains no courses."
            );

            delete kv;

            g_CurrentTriggerIndex = -1;

            return;
        }

        do
        {
            char courseName[128];

            kv.GetSectionName(
                courseName,
                sizeof(courseName)
            );

            // ------------------------------------------------
            // We are now inside a course.
            // ------------------------------------------------

            if (!kv.GotoFirstSubKey(false))
            {
                continue;
            }

            do
            {
                char triggerName[128];

                kv.GetSectionName(
                    triggerName,
                    sizeof(triggerName)
                );

                // --------------------------------------------
                // Only accept rjai_* entries.
                // --------------------------------------------

                if (StrContains(
                        triggerName,
                        "rjai_",
                        false
                    ) != 0)
                {
                    continue;
                }

                TriggerData trigger;

                LoadTriggerFromKeyValues(
                    kv,
                    trigger
                );

                // --------------------------------------------
                // Runtime name:
                //
                // Basic + "_" + rjai_1
                //
                // = Basic_rjai_1
                // --------------------------------------------

                Format(
                    trigger.name,
                    sizeof(trigger.name),
                    "%s_%s",
                    courseName,
                    triggerName
                );

                // --------------------------------------------
                // Create runtime entity.
                // --------------------------------------------

                int entity = CreateRuntimeTrigger(
                    trigger
                );

                if (entity == -1)
                {
                    continue;
                }

                trigger.entity = entity;

                g_Triggers.PushArray(
                    trigger
                );

            } while (kv.GotoNextKey(false));

            // --------------------------------------------
            // Return from course to Courses.
            // --------------------------------------------

            kv.GoBack();

        } while (kv.GotoNextKey(false));
    }

    // --------------------------------------------------------
    // Set initial current trigger.
    // --------------------------------------------------------

    if (g_Triggers.Length > 0)
    {
        g_CurrentTriggerIndex = 0;

        TriggerData currentTrigger;

        g_Triggers.GetArray(
            g_CurrentTriggerIndex,
            currentTrigger
        );

        PrintToServer(
            "[Trigger] Current trigger: %d - %s (entity %d)",
            g_CurrentTriggerIndex,
            currentTrigger.name,
            currentTrigger.entity
        );
    }
    else
    {
        g_CurrentTriggerIndex = -1;
    }

    // --------------------------------------------------------
    // Diagnostics.
    // --------------------------------------------------------

    PrintToServer(
        "[Trigger] Loaded %d runtime triggers.",
        g_Triggers.Length
    );

    delete kv;
}


// ============================================================
// Load TriggerData from current KeyValues position
// ============================================================

void LoadTriggerFromKeyValues(
    KeyValues kv,
    TriggerData trigger
)
{
    // --------------------------------------------------------
    // Runtime entity starts invalid.
    // --------------------------------------------------------

    trigger.entity = -1;

    // --------------------------------------------------------
    // World mins.
    // --------------------------------------------------------

    trigger.worldMins[0] =
        kv.GetFloat("worldMins_x");

    trigger.worldMins[1] =
        kv.GetFloat("worldMins_y");

    trigger.worldMins[2] =
        kv.GetFloat("worldMins_z");

    // --------------------------------------------------------
    // World maxs.
    // --------------------------------------------------------

    trigger.worldMaxs[0] =
        kv.GetFloat("worldMaxs_x");

    trigger.worldMaxs[1] =
        kv.GetFloat("worldMaxs_y");

    trigger.worldMaxs[2] =
        kv.GetFloat("worldMaxs_z");

    // --------------------------------------------------------
    // Origin.
    // --------------------------------------------------------

    trigger.origin[0] =
        kv.GetFloat("origin_x");

    trigger.origin[1] =
        kv.GetFloat("origin_y");

    trigger.origin[2] =
        kv.GetFloat("origin_z");

    // --------------------------------------------------------
    // Local mins.
    // --------------------------------------------------------

    trigger.localMins[0] =
        kv.GetFloat("localMins_x");

    trigger.localMins[1] =
        kv.GetFloat("localMins_y");

    trigger.localMins[2] =
        kv.GetFloat("localMins_z");

    // --------------------------------------------------------
    // Local maxs.
    // --------------------------------------------------------

    trigger.localMaxs[0] =
        kv.GetFloat("localMaxs_x");

    trigger.localMaxs[1] =
        kv.GetFloat("localMaxs_y");

    trigger.localMaxs[2] =
        kv.GetFloat("localMaxs_z");
}


// ============================================================
// Create runtime trigger
//
// Returns:
//     Entity index on success
//     -1 on failure
// ============================================================

int CreateRuntimeTrigger(
    TriggerData trigger
)
{
    // --------------------------------------------------------
    // Create trigger entity.
    // --------------------------------------------------------

    int entity = CreateEntityByName(
        "trigger_multiple"
    );

    if (entity == -1)
    {
        PrintToServer(
            "[Trigger] ERROR: Failed to create \"%s\".",
            trigger.name
        );

        return -1;
    }

    // --------------------------------------------------------
    // Target name.
    // --------------------------------------------------------

    DispatchKeyValue(
        entity,
        "targetname",
        trigger.name
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
        PrintToServer(
            "[Trigger] ERROR: DispatchSpawn failed for \"%s\".",
            trigger.name
        );

        AcceptEntityInput(
            entity,
            "Kill"
        );

        return -1;
    }

    // --------------------------------------------------------
    // Activate.
    // --------------------------------------------------------

    ActivateEntity(entity);

    // --------------------------------------------------------
    // Move entity to stored origin.
    // --------------------------------------------------------

    TeleportEntity(
        entity,
        trigger.origin,
        NULL_VECTOR,
        NULL_VECTOR
    );

    // --------------------------------------------------------
    // Give trigger its model.
    //
    // This is required for the runtime collision volume.
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
        trigger.localMins
    );

    SetEntPropVector(
        entity,
        Prop_Send,
        "m_vecMaxs",
        trigger.localMaxs
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
    // FSOLID_TRIGGER.
    // --------------------------------------------------------

    SetEntProp(
        entity,
        Prop_Send,
        "m_usSolidFlags",
        8
    );

    // --------------------------------------------------------
    // Collision group.
    // --------------------------------------------------------

    SetEntProp(
        entity,
        Prop_Send,
        "m_CollisionGroup",
        1
    );

    // --------------------------------------------------------
    // Reapply bounds after model initialization.
    // --------------------------------------------------------

    SetEntPropVector(
        entity,
        Prop_Send,
        "m_vecMins",
        trigger.localMins
    );

    SetEntPropVector(
        entity,
        Prop_Send,
        "m_vecMaxs",
        trigger.localMaxs
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
        "[Trigger] Loaded \"%s\" -> entity %d",
        trigger.name,
        entity
    );

    PrintToServer(
        "  World mins: %.3f %.3f %.3f",
        trigger.worldMins[0],
        trigger.worldMins[1],
        trigger.worldMins[2]
    );

    PrintToServer(
        "  World maxs: %.3f %.3f %.3f",
        trigger.worldMaxs[0],
        trigger.worldMaxs[1],
        trigger.worldMaxs[2]
    );

    PrintToServer(
        "  Origin: %.3f %.3f %.3f",
        trigger.origin[0],
        trigger.origin[1],
        trigger.origin[2]
    );

    PrintToServer(
        "  Local mins: %.3f %.3f %.3f",
        trigger.localMins[0],
        trigger.localMins[1],
        trigger.localMins[2]
    );

    PrintToServer(
        "  Local maxs: %.3f %.3f %.3f",
        trigger.localMaxs[0],
        trigger.localMaxs[1],
        trigger.localMaxs[2]
    );

    return entity;
}


bool SetTrigger(
    const char[] courseName,
    int triggerNumber,
    int client
)
{
    char triggerName[128];

    Format(
        triggerName,
        sizeof(triggerName),
        "rjai_%d",
        triggerNumber
    );

    char fullName[128];

    if (strlen(courseName) > 0)
    {
        Format(
            fullName,
            sizeof(fullName),
            "%s_%s",
            courseName,
            triggerName
        );
    }
    else
    {
        strcopy(
            fullName,
            sizeof(fullName),
            triggerName
        );
    }


    // ========================================================
    // Search loaded runtime triggers.
    // ========================================================

    for (int i = 0; i < g_Triggers.Length; i++)
    {
        TriggerData trigger;

        g_Triggers.GetArray(
            i,
            trigger
        );

        if (StrEqual(
                trigger.name,
                fullName,
                false
            ))
        {
            g_CurrentTriggerIndex = i;

            PrintToChat(
                client,
                "[Trigger] Current trigger set to: %s",
                fullName
            );

            return true;
        }
    }


    // ========================================================
    // Trigger was not found.
    // ========================================================

    g_CurrentTriggerIndex = -1;

    PrintToChat(
        client,
        "[Trigger] Trigger not found: %s",
        fullName
    );

    return false;
}



// ============================================================
// Start Touch
// ============================================================

public void OnTriggerStartTouch(
    int entity,
    int other
)
{
    // --------------------------------------------------------
    // Only players.
    // --------------------------------------------------------

    if (other < 1 || other > MaxClients)
    {
        return;
    }

    if (!IsClientInGame(other))
    {
        return;
    }

    // --------------------------------------------------------
    // No current trigger.
    // --------------------------------------------------------

    if (g_CurrentTriggerIndex < 0 ||
        g_CurrentTriggerIndex >= g_Triggers.Length)
    {
        return;
    }

    // --------------------------------------------------------
    // Get current trigger.
    // --------------------------------------------------------

    TriggerData currentTrigger;

    g_Triggers.GetArray(
        g_CurrentTriggerIndex,
        currentTrigger
    );

    // --------------------------------------------------------
    // Ignore all triggers except current trigger.
    // --------------------------------------------------------

    if (entity != currentTrigger.entity)
    {
        return;
    }

    // --------------------------------------------------------
    // Current trigger entered.
    // --------------------------------------------------------

    PrintToChat(
        other,
        "[Trigger] ENTER: %s",
        currentTrigger.name
    );

    PrintToServer(
        "[Trigger] Client %d ENTERED \"%s\" (entity %d)",
        other,
        currentTrigger.name,
        entity
    );
}


// ============================================================
// End Touch
// ============================================================

public void OnTriggerEndTouch(
    int entity,
    int other
)
{
    // --------------------------------------------------------
    // Only players.
    // --------------------------------------------------------

    if (other < 1 || other > MaxClients)
    {
        return;
    }

    if (!IsClientInGame(other))
    {
        return;
    }

    // --------------------------------------------------------
    // No current trigger.
    // --------------------------------------------------------

    if (g_CurrentTriggerIndex < 0 ||
        g_CurrentTriggerIndex >= g_Triggers.Length)
    {
        return;
    }

    // --------------------------------------------------------
    // Get current trigger.
    // --------------------------------------------------------

    TriggerData currentTrigger;

    g_Triggers.GetArray(
        g_CurrentTriggerIndex,
        currentTrigger
    );

    // --------------------------------------------------------
    // Ignore all triggers except current trigger.
    // --------------------------------------------------------

    if (entity != currentTrigger.entity)
    {
        return;
    }

    // --------------------------------------------------------
    // Current trigger exited.
    // --------------------------------------------------------

    PrintToChat(
        other,
        "[Trigger] EXIT: %s",
        currentTrigger.name
    );

    PrintToServer(
        "[Trigger] Client %d EXITED \"%s\" (entity %d)",
        other,
        currentTrigger.name,
        entity
    );
}