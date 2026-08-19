#if defined _courseruntime_triggerData_included
    #endinput
#endif
#define _courseruntime_triggerData_included

#include <sdkhooks>

bool LoadTriggers()
{
    // ========================================================
    // Get current map.
    // ========================================================

    char mapName[128];

    GetCurrentMap(
        mapName,
        sizeof(mapName)
    );

    // ========================================================
    // Build config path.
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

    if (!FileExists(path))
    {
        PrintToServer(
            "[Trigger] ERROR: Map config does not exist: %s",
            path
        );

        return false;
    }

    // ========================================================
    // Load KeyValues.
    // ========================================================

    KeyValues kv = new KeyValues("Triggers");

    if (!kv.ImportFromFile(path))
    {
        PrintToServer(
            "[Trigger] ERROR: Failed to load map config: %s",
            path
        );

        delete kv;
        return false;
    }

    // ========================================================
    // Keep track of courses that exist in the new CFG.
    //
    // After loading, any course not in this list can be
    // removed from g_AllCoursesTriggers.
    // ========================================================

    ArrayList loadedCourseNames =
        new ArrayList(ByteCountToCells(128));

    if (g_AllCoursesTriggers == null)
    {
        g_AllCoursesTriggers = new ArrayList(
            sizeof(CourseTriggers)
        );
    }

    // ========================================================
    // Check for Courses section.
    // ========================================================

    kv.Rewind();

    bool hasCourses = kv.JumpToKey(
        "Courses",
        false
    );

    // ========================================================
    // CASE 1:
    //
    // No Courses section.
    //
    // Entire CFG is one default course.
    // ========================================================

    if (!hasCourses)
    {
        char courseName[128];

        strcopy(
            courseName,
            sizeof(courseName),
            DEFAULT_COURSE_NAME
        );

        loadedCourseNames.PushString(
            courseName
        );

        // ----------------------------------------------------
        // Find existing default course.
        // ----------------------------------------------------

        int courseIndex = FindCourseTriggers(
            courseName
        );

        // ----------------------------------------------------
        // Create course if it doesn't exist yet.
        // ----------------------------------------------------

        if (courseIndex == -1)
        {
            CourseTriggers newCourse;

            strcopy(
                newCourse.courseName,
                sizeof(newCourse.courseName),
                courseName
            );

            newCourse.triggers = new ArrayList(
                sizeof(TriggerData)
            );

            g_AllCoursesTriggers.PushArray(
                newCourse
            );

            courseIndex =
                g_AllCoursesTriggers.Length - 1;
        }

        // ----------------------------------------------------
        // Retrieve existing course.
        // ----------------------------------------------------

        CourseTriggers course;

        g_AllCoursesTriggers.GetArray(
            courseIndex,
            course
        );

        // ----------------------------------------------------
        // Process CFG triggers.
        //
        // This synchronizes:
        //   - new triggers
        //   - changed triggers
        //   - unchanged triggers
        //   - deleted triggers
        // ----------------------------------------------------

        kv.Rewind();

        LoadTriggerList(
            kv,
            course.triggers,
            ""
        );

        // ----------------------------------------------------
        // Store updated course.
        // ----------------------------------------------------

        g_AllCoursesTriggers.SetArray(
            courseIndex,
            course
        );

        PrintToServer(
            "[Trigger] Synchronized default course \"%s\" with %d triggers.",
            course.courseName,
            course.triggers.Length
        );
    }

    // ========================================================
    // CASE 2:
    //
    // Courses section exists.
    //
    // Synchronize every course.
    // ========================================================

    else
    {
        // ----------------------------------------------------
        // Courses section must contain at least one course.
        // ----------------------------------------------------

        if (!kv.GotoFirstSubKey(false))
        {
            PrintToServer(
                "[Trigger] ERROR: Courses section is empty."
            );

            delete loadedCourseNames;
            delete kv;

            return false;
        }

        // ----------------------------------------------------
        // Process every course.
        // ----------------------------------------------------

        do
        {
            char courseName[128];

            kv.GetSectionName(
                courseName,
                sizeof(courseName)
            );

            // ------------------------------------------------
            // Remember that this course exists.
            // ------------------------------------------------

            loadedCourseNames.PushString(
                courseName
            );

            // ------------------------------------------------
            // Find existing course.
            // ------------------------------------------------

            int courseIndex = FindCourseTriggers(
                courseName
            );

            // ------------------------------------------------
            // Create course if necessary.
            // ------------------------------------------------

            if (courseIndex == -1)
            {
                CourseTriggers newCourse;

                strcopy(
                    newCourse.courseName,
                    sizeof(newCourse.courseName),
                    courseName
                );

                newCourse.triggers = new ArrayList(
                    sizeof(TriggerData)
                );

                g_AllCoursesTriggers.PushArray(
                    newCourse
                );

                courseIndex =
                    g_AllCoursesTriggers.Length - 1;
            }

            // ------------------------------------------------
            // Retrieve existing course.
            // ------------------------------------------------

            CourseTriggers course;

            g_AllCoursesTriggers.GetArray(
                courseIndex,
                course
            );

            // ------------------------------------------------
            // Synchronize triggers for this course.
            // ------------------------------------------------

            LoadTriggerList(
                kv,
                course.triggers,
                courseName
            );

            // ------------------------------------------------
            // Store updated course.
            // ------------------------------------------------

            g_AllCoursesTriggers.SetArray(
                courseIndex,
                course
            );

            PrintToServer(
                "[Trigger] Synchronized course \"%s\" with %d triggers.",
                course.courseName,
                course.triggers.Length
            );

        }
        while (kv.GotoNextKey(false));
    }

    // ========================================================
    // Remove courses that no longer exist in the CFG.
    //
    // This must happen AFTER all current courses have been
    // processed.
    // ========================================================

    RemoveDeletedCourses(
        loadedCourseNames
    );

    delete loadedCourseNames;
    delete kv;

    // ========================================================
    // Final validation.
    // ========================================================

    if (
        g_AllCoursesTriggers == null ||
        g_AllCoursesTriggers.Length == 0
    )
    {
        PrintToServer(
            "[Trigger] ERROR: No trigger courses were loaded."
        );

        return false;
    }

    PrintToServer(
        "[Trigger] Synchronized %d trigger courses.",
        g_AllCoursesTriggers.Length
    );
    PrintToServer(
        "-------------------------------------------"
    );

    return true;
}

void LoadTriggerList(
    KeyValues kv,
    ArrayList triggers,
    const char[] courseName
)
{
    ArrayList loadedNames =
        new ArrayList(ByteCountToCells(128));

    if (kv.GotoFirstSubKey(false))
    {
        do
        {
            LoadTriggerData(
                kv,
                courseName,
                triggers,
                loadedNames
            );

        }
        while (kv.GotoNextKey(false));

        kv.GoBack();
    }

    RemoveDeletedTriggers(
        triggers,
        loadedNames
    );

    delete loadedNames;
}

void LoadTriggerData(
    KeyValues kv,
    const char[] courseName,
    ArrayList triggers,
    ArrayList loadedNames
)
{
    TriggerCfgData triggerCfg;

    ReadCfgValues(
        kv,
        triggerCfg
    );

    char originalName[128];

    strcopy(
        originalName,
        sizeof(originalName),
        triggerCfg.name
    );

    if (strlen(courseName) > 0)
    {
        Format(
            triggerCfg.name,
            sizeof(triggerCfg.name),
            "%s_%s",
            courseName,
            originalName
        );
    }

    loadedNames.PushString(
        triggerCfg.name
    );

    SyncTrigger(
        triggers,
        triggerCfg
    );
}

int SyncTrigger(
    ArrayList triggers,
    TriggerCfgData cfg
)
{
    int existingIndex = FindTriggerInCourse(
        triggers,
        cfg.name
    );

    // ====================================================
    // NEW TRIGGER
    // ====================================================

    if (existingIndex == -1)
    {
        int entity = CreateRuntimeTrigger(
            cfg
        );

        if (entity == -1)
        {
            PrintToServer(
                "[Trigger] ERROR: Failed to create \"%s\".",
                cfg.name
            );

            return -1;
        }

        TriggerData trigger;

        strcopy(
            trigger.name,
            sizeof(trigger.name),
            cfg.name
        );

        for (int i = 0; i < 3; i++)
        {
            trigger.worldMins[i] =
                cfg.worldMins[i];

            trigger.worldMaxs[i] =
                cfg.worldMaxs[i];
        }

        trigger.entity = entity;

        triggers.PushArray(
            trigger
        );

        return entity;
    }

    // ====================================================
    // EXISTING TRIGGER
    // ====================================================

    TriggerData existing;

    triggers.GetArray(
        existingIndex,
        existing
    );

    // ====================================================
    // Check whether runtime entity still exists.
    // ====================================================

    if (
        existing.entity <= 0 ||
        !IsValidEntity(existing.entity)
    )
    {
        int entity = CreateRuntimeTrigger(
            cfg
        );

        if (entity == -1)
        {
            PrintToServer(
                "[Trigger] ERROR: Failed to recreate \"%s\".",
                cfg.name
            );

            return -1;
        }

        existing.entity = entity;
    }
    else if (
        TriggerBoundsChanged(
            existing,
            cfg
        )
    )
    {
        AcceptEntityInput(
            existing.entity,
            "Kill"
        );

        int entity = CreateRuntimeTrigger(
            cfg
        );

        if (entity == -1)
        {
            PrintToServer(
                "[Trigger] ERROR: Failed to recreate \"%s\".",
                cfg.name
            );

            return -1;
        }

        existing.entity = entity;

        PrintToServer(
            "[Trigger] Updated \"%s\".",
            cfg.name
        );
    }

    // ====================================================
    // Always update cached data.
    // ====================================================

    for (int i = 0; i < 3; i++)
    {
        existing.worldMins[i] =
            cfg.worldMins[i];

        existing.worldMaxs[i] =
            cfg.worldMaxs[i];
    }

    triggers.SetArray(
        existingIndex,
        existing
    );

    return existing.entity;
}


int FindCourseTriggers(
    const char[] courseName
)
{
    if (g_AllCoursesTriggers == null)
    {
        return -1;
    }

    for (int i = 0; i < g_AllCoursesTriggers.Length; i++)
    {
        CourseTriggers course;

        g_AllCoursesTriggers.GetArray(
            i,
            course
        );

        if (StrEqual(
                course.courseName,
                courseName
            ))
        {
            return i;
        }
    }

    return -1;
}

int FindTriggerInCourse(
    ArrayList triggers,
    const char[] triggerName
)
{
    if (triggers == null)
    {
        return -1;
    }

    for (int i = 0; i < triggers.Length; i++)
    {
        TriggerData trigger;

        triggers.GetArray(
            i,
            trigger
        );

        if (StrEqual(
                trigger.name,
                triggerName
            ))
        {
            return i;
        }
    }

    return -1;
}

bool TriggerBoundsChanged(
    TriggerData existing,
    TriggerCfgData cfg
)
{
    for (int i = 0; i < 3; i++)
    {
        if (FloatAbs(
                existing.worldMins[i] -
                cfg.worldMins[i]
            ) > 0.01)
        {
            return true;
        }

        if (FloatAbs(
                existing.worldMaxs[i] -
                cfg.worldMaxs[i]
            ) > 0.01)
        {
            return true;
        }
    }

    return false;
}


void RemoveDeletedTriggers(
    ArrayList triggers,
    ArrayList loadedNames
)
{
    if (triggers == null)
    {
        return;
    }

    for (int i = triggers.Length - 1; i >= 0; i--)
    {
        TriggerData trigger;

        triggers.GetArray(
            i,
            trigger
        );

        bool found = false;

        for (int j = 0; j < loadedNames.Length; j++)
        {
            char name[128];

            loadedNames.GetString(
                j,
                name,
                sizeof(name)
            );

            if (StrEqual(
                    trigger.name,
                    name
                ))
            {
                found = true;
                break;
            }
        }

        if (!found)
        {
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

            triggers.Erase(i);

            PrintToServer(
                "[Trigger] Removed deleted trigger \"%s\".",
                trigger.name
            );
        }
    }
}

void ReadCfgValues(
    KeyValues kv,
    TriggerCfgData triggerCfg
)
{
    kv.GetString(
        "name",
        triggerCfg.name,
        sizeof(triggerCfg.name),
        ""
    );

    triggerCfg.worldMins[0] =
        kv.GetFloat("worldMins_x");

    triggerCfg.worldMins[1] =
        kv.GetFloat("worldMins_y");

    triggerCfg.worldMins[2] =
        kv.GetFloat("worldMins_z");

    triggerCfg.worldMaxs[0] =
        kv.GetFloat("worldMaxs_x");

    triggerCfg.worldMaxs[1] =
        kv.GetFloat("worldMaxs_y");

    triggerCfg.worldMaxs[2] =
        kv.GetFloat("worldMaxs_z");

    triggerCfg.origin[0] =
        kv.GetFloat("origin_x");

    triggerCfg.origin[1] =
        kv.GetFloat("origin_y");

    triggerCfg.origin[2] =
        kv.GetFloat("origin_z");

    triggerCfg.localMins[0] =
        kv.GetFloat("localMins_x");

    triggerCfg.localMins[1] =
        kv.GetFloat("localMins_y");

    triggerCfg.localMins[2] =
        kv.GetFloat("localMins_z");

    triggerCfg.localMaxs[0] =
        kv.GetFloat("localMaxs_x");

    triggerCfg.localMaxs[1] =
        kv.GetFloat("localMaxs_y");

    triggerCfg.localMaxs[2] =
        kv.GetFloat("localMaxs_z");
}

void RemoveDeletedCourses(
    ArrayList loadedCourseNames
)
{
    if (
        g_AllCoursesTriggers == null ||
        loadedCourseNames == null
    )
    {
        return;
    }

    // --------------------------------------------------------
    // Iterate backwards because we may erase entries.
    // --------------------------------------------------------

    for (
        int i = g_AllCoursesTriggers.Length - 1;
        i >= 0;
        i--
    )
    {
        CourseTriggers course;

        g_AllCoursesTriggers.GetArray(
            i,
            course
        );

        // ----------------------------------------------------
        // Check whether this course still exists in the CFG.
        // ----------------------------------------------------

        bool found = false;

        for (
            int j = 0;
            j < loadedCourseNames.Length;
            j++
        )
        {
            char loadedCourseName[128];

            loadedCourseNames.GetString(
                j,
                loadedCourseName,
                sizeof(loadedCourseName)
            );

            if (StrEqual(
                course.courseName,
                loadedCourseName
            ))
            {
                found = true;
                break;
            }
        }

        // ----------------------------------------------------
        // Course still exists.
        // ----------------------------------------------------

        if (found)
        {
            continue;
        }

        // ----------------------------------------------------
        // Course was removed from the CFG.
        //
        // Kill all runtime trigger entities belonging to it.
        // ----------------------------------------------------

        if (course.triggers != null)
        {
            for (
                int j = 0;
                j < course.triggers.Length;
                j++
            )
            {
                TriggerData trigger;

                course.triggers.GetArray(
                    j,
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

            delete course.triggers;
        }

        // ----------------------------------------------------
        // Remove the course from the cached course list.
        // ----------------------------------------------------

        g_AllCoursesTriggers.Erase(i);

        PrintToServer(
            "[Trigger] Removed deleted course \"%s\".",
            course.courseName
        );
    }
}

int CreateRuntimeTrigger(
    TriggerCfgData trigger
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
        152
    );

    // --------------------------------------------------------
    // Collision group.
    // --------------------------------------------------------

    SetEntProp(
        entity,
        Prop_Send,
        "m_CollisionGroup",
        11
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

    EntityCollisionRulesChanged(entity);

    SDKHook(
        entity,
        SDKHook_StartTouch,
        OnTriggerStartTouch
    );
    
    return entity;
}