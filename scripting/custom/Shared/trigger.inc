#if defined _shared_trigger_included
    #endinput
#endif
#define _shared_trigger_included

#include <sdkhooks>

// ============================================================
// Find runtime trigger by name
//
// Returns the ArrayList index, or -1 if not found.
// ============================================================

int FindTriggerByName(
    const char[] name
)
{
    for (int i = 0; i < g_Triggers.Length; i++)
    {
        TriggerData trigger;

        g_Triggers.GetArray(
            i,
            trigger
        );

        if (StrEqual(
                trigger.name,
                name,
                false
            ))
        {
            return i;
        }
    }

    return -1;
}


// ============================================================
// Check whether two trigger bounds are different.
// ============================================================

bool TriggerBoundsChanged(
    TriggerData &existing,
    TriggerData &updated
)
{
    for (int axis = 0; axis < 3; axis++)
    {
        if (
            FloatAbs(
                existing.worldMins[axis] -
                updated.worldMins[axis]
            ) > 0.01
        )
        {
            return true;
        }

        if (
            FloatAbs(
                existing.worldMaxs[axis] -
                updated.worldMaxs[axis]
            ) > 0.01
        )
        {
            return true;
        }
    }

    return false;
}


// ============================================================
// Synchronize one trigger.
//
// If the trigger doesn't exist:
//     create it.
//
// If it exists and bounds haven't changed:
//     do nothing.
//
// If it exists but bounds changed:
//     kill old entity and recreate it.
//
// Returns true if the trigger exists/was successfully created.
// ============================================================

bool SyncTrigger(
    TriggerData &cfgTrigger
)
{
    int existingIndex = FindTriggerByName(
        cfgTrigger.name
    );


    // ========================================================
    // New trigger.
    // ========================================================

    if (existingIndex == -1)
    {
        int entity = CreateRuntimeTrigger(
            cfgTrigger
        );

        if (entity == -1)
        {
            PrintToServer(
                "[Trigger] ERROR: Failed to create \"%s\".",
                cfgTrigger.name
            );

            return false;
        }

        cfgTrigger.entity = entity;

        g_Triggers.PushArray(
            cfgTrigger
        );

        PrintToServer(
            "[Trigger] Created new trigger \"%s\" (entity %d).",
            cfgTrigger.name,
            entity
        );

        return true;
    }


    // ========================================================
    // Existing trigger.
    // ========================================================

    TriggerData existing;

    g_Triggers.GetArray(
        existingIndex,
        existing
    );


    // ========================================================
    // Check whether its bounds changed.
    // ========================================================

    if (!TriggerBoundsChanged(
            existing,
            cfgTrigger
        ))
    {
        // ----------------------------------------------------
        // Nothing changed.
        //
        // Keep the existing entity and preserve its entity
        // index in our TriggerData.
        // ----------------------------------------------------

        return true;
    }


    // ========================================================
    // Bounds changed.
    //
    // Remove old runtime entity.
    // ========================================================

    if (
        existing.entity > 0 &&
        IsValidEntity(existing.entity)
    )
    {
        AcceptEntityInput(
            existing.entity,
            "Kill"
        );
    }


    // ========================================================
    // Create replacement entity.
    // ========================================================

    int newEntity = CreateRuntimeTrigger(
        cfgTrigger
    );

    if (newEntity == -1)
    {
        PrintToServer(
            "[Trigger] ERROR: Failed to recreate \"%s\".",
            cfgTrigger.name
        );

        return false;
    }

    cfgTrigger.entity = newEntity;


    // ========================================================
    // Replace TriggerData at the same ArrayList index.
    //
    // This is important because replacing instead of erasing
    // keeps the ArrayList index stable.
    // ========================================================

    g_Triggers.SetArray(
        existingIndex,
        cfgTrigger
    );


    PrintToServer(
        "[Trigger] Updated runtime trigger \"%s\" (entity %d).",
        cfgTrigger.name,
        newEntity
    );

    return true;
}


// ============================================================
// Load / synchronize all triggers for the current map.
//
// This does NOT blindly recreate every trigger.
//
// Existing unchanged triggers are kept.
// Changed triggers are recreated.
// New triggers are created.
// Removed CFG triggers are deleted.
// ============================================================

void LoadAll()
{
    // ========================================================
    // Make sure runtime trigger list exists.
    // ========================================================

    if (g_Triggers == null)
    {
        g_Triggers = new ArrayList(
            sizeof(TriggerData)
        );
    }


    // ========================================================
    // Remember current trigger by NAME.
    //
    // We cannot rely on g_CurrentTriggerIndex because entries
    // can be erased from g_Triggers during synchronization.
    // ========================================================

    char previousCurrentName[128];

    previousCurrentName[0] = '\0';

    if (
        g_CurrentTriggerIndex >= 0 &&
        g_CurrentTriggerIndex < g_Triggers.Length
    )
    {
        TriggerData currentTrigger;

        g_Triggers.GetArray(
            g_CurrentTriggerIndex,
            currentTrigger
        );

        strcopy(
            previousCurrentName,
            sizeof(previousCurrentName),
            currentTrigger.name
        );
    }


    // ========================================================
    // Get current map.
    // ========================================================

    char mapName[128];

    GetCurrentMap(
        mapName,
        sizeof(mapName)
    );


    // ========================================================
    // Build CFG path.
    // ========================================================

    char path[PLATFORM_MAX_PATH];

    BuildPath(
        Path_SM,
        path,
        sizeof(path),
        "%s/%s_triggers.cfg",
        MAP_CONFIG_PATH,
        mapName
    );


    // ========================================================
    // If no CFG exists:
    //
    // remove every runtime trigger.
    // ========================================================

    if (!FileExists(path))
    {
        PrintToServer(
            "[Trigger] No trigger file found: %s",
            path
        );


        for (int i = g_Triggers.Length - 1; i >= 0; i--)
        {
            TriggerData trigger;

            g_Triggers.GetArray(
                i,
                trigger
            );

            if (
                trigger.entity > 0 &&
                IsValidEntity(trigger.entity)
            )
            {
                AcceptEntityInput(
                    trigger.entity,
                    "Kill"
                );
            }
        }


        g_Triggers.Clear();

        g_CurrentTriggerIndex = -1;

        return;
    }


    // ========================================================
    // Load KeyValues.
    // ========================================================

    KeyValues kv = new KeyValues(
        "Triggers"
    );

    if (!kv.ImportFromFile(path))
    {
        PrintToServer(
            "[Trigger] ERROR: Failed to read trigger file: %s",
            path
        );

        delete kv;

        return;
    }


    // ========================================================
    // Keep track of every trigger found in the CFG.
    //
    // Used later to remove runtime triggers which no longer
    // exist in the configuration.
    // ========================================================

    StringMap foundTriggers = new StringMap();


    // ========================================================
    // Determine whether this is a course-based file.
    // ========================================================

    kv.Rewind();

    bool hasCourses = kv.JumpToKey(
        "Courses",
        false
    );


    // ========================================================
    // FLAT TRIGGER FILE
    //
    // Triggers
    // {
    //     rjai_0
    //     {
    //     }
    //
    //     rjai_1
    //     {
    //     }
    // }
    // ========================================================

    if (!hasCourses)
    {
        kv.Rewind();

        if (kv.GotoFirstSubKey(false))
        {
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


                // ------------------------------------------------
                // Mark as found.
                // ------------------------------------------------

                foundTriggers.SetValue(
                    triggerName,
                    1
                );


                // ------------------------------------------------
                // Load trigger data.
                // ------------------------------------------------

                TriggerData trigger;

                LoadTriggerFromKeyValues(
                    kv,
                    trigger
                );


                // ------------------------------------------------
                // Runtime name is simply rjai_X.
                // ------------------------------------------------

                strcopy(
                    trigger.name,
                    sizeof(trigger.name),
                    triggerName
                );


                // ------------------------------------------------
                // Synchronize.
                // ------------------------------------------------

                SyncTrigger(
                    trigger
                );

            } while (kv.GotoNextKey(false));
        }
    }


    // ========================================================
    // COURSE-BASED TRIGGER FILE
    //
    // Triggers
    // {
    //     Courses
    //     {
    //         Basic
    //         {
    //             rjai_0
    //             {
    //             }
    //
    //             rjai_1
    //             {
    //             }
    //         }
    //
    //         Advanced
    //         {
    //             ...
    //         }
    //     }
    // }
    // ========================================================

    else
    {
        // ----------------------------------------------------
        // Currently inside Courses.
        // ----------------------------------------------------

        if (kv.GotoFirstSubKey(false))
        {
            do
            {
                char courseName[128];

                kv.GetSectionName(
                    courseName,
                    sizeof(courseName)
                );


                // ------------------------------------------------
                // Enter course.
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


                    // ------------------------------------------------
                    // Build runtime name.
                    //
                    // Basic + "_" + rjai_1
                    //
                    // = Basic_rjai_1
                    // ------------------------------------------------

                    char fullName[128];

                    Format(
                        fullName,
                        sizeof(fullName),
                        "%s_%s",
                        courseName,
                        triggerName
                    );


                    // ------------------------------------------------
                    // Mark as found.
                    // ------------------------------------------------

                    foundTriggers.SetValue(
                        fullName,
                        1
                    );


                    // ------------------------------------------------
                    // Load trigger data.
                    // ------------------------------------------------

                    TriggerData trigger;

                    LoadTriggerFromKeyValues(
                        kv,
                        trigger
                    );


                    // ------------------------------------------------
                    // Runtime name.
                    // ------------------------------------------------

                    strcopy(
                        trigger.name,
                        sizeof(trigger.name),
                        fullName
                    );


                    // ------------------------------------------------
                    // Synchronize.
                    // ------------------------------------------------

                    SyncTrigger(
                        trigger
                    );

                } while (kv.GotoNextKey(false));


                // ------------------------------------------------
                // Return from course to Courses.
                // ------------------------------------------------

                kv.GoBack();

            } while (kv.GotoNextKey(false));
        }
    }


    // ========================================================
    // Remove runtime triggers which no longer exist in CFG.
    // ========================================================

    for (
        int i = g_Triggers.Length - 1;
        i >= 0;
        i--
    )
    {
        TriggerData trigger;

        g_Triggers.GetArray(
            i,
            trigger
        );


        if (!foundTriggers.ContainsKey(
                trigger.name
            ))
        {
            // ------------------------------------------------
            // Kill runtime entity.
            // ------------------------------------------------

            if (
                trigger.entity > 0 &&
                IsValidEntity(trigger.entity)
            )
            {
                AcceptEntityInput(
                    trigger.entity,
                    "Kill"
                );
            }


            PrintToServer(
                "[Trigger] Removed runtime trigger \"%s\".",
                trigger.name
            );


            // ------------------------------------------------
            // Remove from ArrayList.
            // ------------------------------------------------

            g_Triggers.Erase(i);
        }
    }


    // ========================================================
    // Restore previous current trigger by NAME.
    //
    // The ArrayList index may have changed because entries
    // could have been removed.
    // ========================================================

    g_CurrentTriggerIndex = -1;

    if (strlen(previousCurrentName) > 0)
    {
        g_CurrentTriggerIndex = FindTriggerByName(
            previousCurrentName
        );
    }


    // ========================================================
    // If there was no previous current trigger, or it was
    // deleted, select the first available trigger.
    // ========================================================

    if (
        g_CurrentTriggerIndex == -1 &&
        g_Triggers.Length > 0
    )
    {
        g_CurrentTriggerIndex = 0;
    }


    // ========================================================
    // Diagnostics.
    // ========================================================

    PrintToServer(
        "[Trigger] Runtime trigger synchronization complete."
    );

    PrintToServer(
        "[Trigger] Active runtime triggers: %d",
        g_Triggers.Length
    );


    if (g_CurrentTriggerIndex >= 0)
    {
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
        PrintToServer(
            "[Trigger] No current trigger selected."
        );
    }


    // ========================================================
    // Cleanup.
    // ========================================================

    delete foundTriggers;
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

    int index = FindTriggerByName(fullName);

    if (index != -1)
    {
        g_CurrentTriggerIndex = index;

        PrintToChat(
            client,
            "[Trigger] Current trigger set to: %s",
            fullName
        );

        return true;
    }

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