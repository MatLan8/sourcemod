

#define MAP_CONFIG_PATH "data/MapConfigs"


bool Calculate(TriggerData trigger)
{

    if (g_TriggerPointCount != 2)
    {
        PrintToServer(
            "[Trigger] ERROR: Exactly two points are required."
        );

        return false;
    }

    // --------------------------------------------------------
    // Calculate world-space minimum and maximum coordinates.
    // --------------------------------------------------------

    for (int axis = 0; axis < 3; axis++)
    {
        if (g_TriggerPoints[0][axis] < g_TriggerPoints[1][axis])
        {
            trigger.worldMins[axis] = g_TriggerPoints[0][axis];
            trigger.worldMaxs[axis] = g_TriggerPoints[1][axis];
        }
        else
        {
            trigger.worldMins[axis] = g_TriggerPoints[1][axis];
            trigger.worldMaxs[axis] = g_TriggerPoints[0][axis];
        }
    }

    // --------------------------------------------------------
    // Calculate center/origin.
    // --------------------------------------------------------

    trigger.origin[0] =
        (trigger.worldMins[0] + trigger.worldMaxs[0]) * 0.5;

    trigger.origin[1] =
        (trigger.worldMins[1] + trigger.worldMaxs[1]) * 0.5;

    trigger.origin[2] =
        (trigger.worldMins[2] + trigger.worldMaxs[2]) * 0.5;

    // --------------------------------------------------------
    // Calculate local mins/maxs.
    //
    // The trigger entity is positioned at its center, so its
    // local bounds extend equally in both directions.
    // --------------------------------------------------------

    trigger.localMins[0] =
        trigger.worldMins[0] - trigger.origin[0];

    trigger.localMins[1] =
        trigger.worldMins[1] - trigger.origin[1];

    trigger.localMins[2] =
        trigger.worldMins[2] - trigger.origin[2];

    trigger.localMaxs[0] =
        trigger.worldMaxs[0] - trigger.origin[0];

    trigger.localMaxs[1] =
        trigger.worldMaxs[1] - trigger.origin[1];

    trigger.localMaxs[2] =
        trigger.worldMaxs[2] - trigger.origin[2];

    return true;
}



void BuildTriggerName(
    int triggerNumber,
    char[] triggerName,
    int maxlen
)
{
    Format(
        triggerName,
        maxlen,
        "rjai_%d",
        triggerNumber
    );
}


void SaveTriggerData(
    KeyValues kv,
    TriggerData trigger
)
{
    kv.SetString(
        "name",
        trigger.name
    );

    kv.SetFloat(
        "worldMins_x",
        trigger.worldMins[0]
    );

    kv.SetFloat(
        "worldMins_y",
        trigger.worldMins[1]
    );

    kv.SetFloat(
        "worldMins_z",
        trigger.worldMins[2]
    );

    kv.SetFloat(
        "worldMaxs_x",
        trigger.worldMaxs[0]
    );

    kv.SetFloat(
        "worldMaxs_y",
        trigger.worldMaxs[1]
    );

    kv.SetFloat(
        "worldMaxs_z",
        trigger.worldMaxs[2]
    );

    kv.SetFloat(
        "origin_x",
        trigger.origin[0]
    );

    kv.SetFloat(
        "origin_y",
        trigger.origin[1]
    );

    kv.SetFloat(
        "origin_z",
        trigger.origin[2]
    );

    kv.SetFloat(
        "localMins_x",
        trigger.localMins[0]
    );

    kv.SetFloat(
        "localMins_y",
        trigger.localMins[1]
    );

    kv.SetFloat(
        "localMins_z",
        trigger.localMins[2]
    );

    kv.SetFloat(
        "localMaxs_x",
        trigger.localMaxs[0]
    );

    kv.SetFloat(
        "localMaxs_y",
        trigger.localMaxs[1]
    );

    kv.SetFloat(
        "localMaxs_z",
        trigger.localMaxs[2]
    );
}

void Save(
    int client,
    const char[] courseName,
    int triggerNumber
)
{
    // ========================================================
    // Build trigger name
    // ========================================================

    char triggerName[128];

    BuildTriggerName(
        triggerNumber,
        triggerName,
        sizeof(triggerName)
    );


    // ========================================================
    // Calculate trigger data from current points
    // ========================================================

    TriggerData trigger;

    if (!Calculate(trigger))
    {
        ReplyToCommand(
            client,
            "[Trigger] Failed to calculate trigger. Two points are required."
        );

        return;
    }

    strcopy(
        trigger.name,
        sizeof(trigger.name),
        triggerName
    );


    // ========================================================
    // Get current map
    // ========================================================

    char mapName[128];

    GetCurrentMap(
        mapName,
        sizeof(mapName)
    );


    // ========================================================
    // Build file path
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
    // Make sure config directory exists
    // ========================================================

    char directory[PLATFORM_MAX_PATH];

    BuildPath(
        Path_SM,
        directory,
        sizeof(directory),
        "%s",
        MAP_CONFIG_PATH
    );

    if (!DirExists(directory))
    {
        CreateDirectory(
            directory,
            511
        );
    }


    // ========================================================
    // Create / load KeyValues
    // ========================================================

    KeyValues kv = new KeyValues("Triggers");

    if (FileExists(path))
    {
        if (!kv.ImportFromFile(path))
        {
            ReplyToCommand(
                client,
                "[Trigger] ERROR: Failed to read existing file: %s",
                path
            );

            delete kv;
            return;
        }
    }

    kv.Rewind();


    // ========================================================
    // Navigate to correct location
    //
    // No course:
    //
    // Triggers
    //     rjai_1
    //
    //
    // With course:
    //
    // Triggers
    //     Courses
    //         Basic
    //             rjai_1
    // ========================================================

    if (strlen(courseName) > 0)
    {
        // ----------------------------------------------------
        // Enter/create Courses.
        // ----------------------------------------------------

        if (!kv.JumpToKey("Courses", true))
        {
            ReplyToCommand(
                client,
                "[Trigger] ERROR: Failed to create Courses key."
            );

            delete kv;
            return;
        }

        // ----------------------------------------------------
        // Enter/create requested course.
        // ----------------------------------------------------

        if (!kv.JumpToKey(courseName, true))
        {
            ReplyToCommand(
                client,
                "[Trigger] ERROR: Failed to create course \"%s\".",
                courseName
            );

            delete kv;
            return;
        }
    }


    // ========================================================
    // Check whether trigger already exists
    // ========================================================

    if (kv.JumpToKey(triggerName, false))
    {
        ReplyToCommand(
            client,
            "[Trigger] Trigger \"%s\" already exists%s.",
            triggerName,
            strlen(courseName) > 0
                ? " in the specified course"
                : ""
        );

        delete kv;
        return;
    }


    // ========================================================
    // Create trigger entry
    // ========================================================

    kv.JumpToKey(
        triggerName,
        true
    );

    SaveTriggerData(
        kv,
        trigger
    );


    // ========================================================
    // Export file
    // ========================================================

    kv.Rewind();

    if (!kv.ExportToFile(path))
    {
        ReplyToCommand(
            client,
            "[Trigger] ERROR: Failed to save trigger file: %s",
            path
        );

        delete kv;
        return;
    }


    // ========================================================
    // Success message
    // ========================================================

    if (strlen(courseName) > 0)
    {
        ReplyToCommand(
            client,
            "[Trigger] Saved \"%s\" under course \"%s\".",
            triggerName,
            courseName
        );
    }
    else
    {
        ReplyToCommand(
            client,
            "[Trigger] Saved \"%s\".",
            triggerName
        );
    }


    // ========================================================
    // Points have now been consumed.
    // ========================================================

    Reset(client);
    FireConfigChanged();

    delete kv;
}

void Delete(
    int client,
    const char[] courseName,
    int triggerNumber
)
{
    // ========================================================
    // Build trigger name
    // ========================================================

    char triggerName[128];

    BuildTriggerName(
        triggerNumber,
        triggerName,
        sizeof(triggerName)
    );


    // ========================================================
    // Get current map
    // ========================================================

    char mapName[128];

    GetCurrentMap(
        mapName,
        sizeof(mapName)
    );


    // ========================================================
    // Build file path
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
    // Check file exists
    // ========================================================

    if (!FileExists(path))
    {
        ReplyToCommand(
            client,
            "[Trigger] Configuration file does not exist."
        );

        return;
    }


    // ========================================================
    // Load KeyValues
    // ========================================================

    KeyValues kv = new KeyValues("Triggers");

    if (!kv.ImportFromFile(path))
    {
        ReplyToCommand(
            client,
            "[Trigger] ERROR: Failed to read file: %s",
            path
        );

        delete kv;
        return;
    }

    kv.Rewind();


    // ========================================================
    // Navigate to correct location
    //
    // No course:
    //
    // Triggers
    //     rjai_1
    //
    //
    // With course:
    //
    // Triggers
    //     Courses
    //         Basic
    //             rjai_1
    // ========================================================

    if (strlen(courseName) > 0)
    {
        // ----------------------------------------------------
        // Enter Courses.
        // ----------------------------------------------------

        if (!kv.JumpToKey("Courses", false))
        {
            ReplyToCommand(
                client,
                "[Trigger] Courses section does not exist."
            );

            delete kv;
            return;
        }

        // ----------------------------------------------------
        // Enter requested course.
        // ----------------------------------------------------

        if (!kv.JumpToKey(courseName, false))
        {
            ReplyToCommand(
                client,
                "[Trigger] Course \"%s\" does not exist.",
                courseName
            );

            delete kv;
            return;
        }
    }


    // ========================================================
    // Find trigger
    // ========================================================

    if (!kv.JumpToKey(triggerName, false))
    {
        ReplyToCommand(
            client,
            "[Trigger] Trigger \"%s\" does not exist%s.",
            triggerName,
            strlen(courseName) > 0
                ? " in the specified course"
                : ""
        );

        delete kv;
        return;
    }


    // ========================================================
    // Move back to parent key
    //
    // We need to be positioned at:
    //
    // Triggers
    //
    // or:
    //
    // Courses -> Basic
    //
    // so DeleteKey() can remove rjai_X.
    // ========================================================

    kv.GoBack();


    // ========================================================
    // Delete trigger
    // ========================================================

    if (!kv.DeleteKey(triggerName))
    {
        ReplyToCommand(
            client,
            "[Trigger] ERROR: Failed to delete \"%s\".",
            triggerName
        );

        delete kv;
        return;
    }


    // ========================================================
    // Save updated file
    // ========================================================

    kv.Rewind();

    if (!kv.ExportToFile(path))
    {
        ReplyToCommand(
            client,
            "[Trigger] ERROR: Failed to save updated file: %s",
            path
        );

        delete kv;
        return;
    }


    // ========================================================
    // Success message
    // ========================================================

    if (strlen(courseName) > 0)
    {
        ReplyToCommand(
            client,
            "[Trigger] Deleted \"%s\" from course \"%s\".",
            triggerName,
            courseName
        );
    }
    else
    {
        ReplyToCommand(
            client,
            "[Trigger] Deleted \"%s\".",
            triggerName
        );
    }
    FireConfigChanged();

    delete kv;
}

void Update(
    int client,
    const char[] courseName,
    int triggerNumber
)
{
    // ========================================================
    // Build trigger name
    // ========================================================

    char triggerName[128];

    BuildTriggerName(
        triggerNumber,
        triggerName,
        sizeof(triggerName)
    );


    // ========================================================
    // Calculate new trigger data from current points
    // ========================================================

    TriggerData trigger;

    if (!Calculate(trigger))
    {
        ReplyToCommand(
            client,
            "[Trigger] Failed to calculate trigger. Two points are required."
        );

        return;
    }

    strcopy(
        trigger.name,
        sizeof(trigger.name),
        triggerName
    );


    // ========================================================
    // Get current map
    // ========================================================

    char mapName[128];

    GetCurrentMap(
        mapName,
        sizeof(mapName)
    );


    // ========================================================
    // Build file path
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
    // Check file exists
    // ========================================================

    if (!FileExists(path))
    {
        ReplyToCommand(
            client,
            "[Trigger] Configuration file does not exist."
        );

        return;
    }


    // ========================================================
    // Load KeyValues
    // ========================================================

    KeyValues kv = new KeyValues("Triggers");

    if (!kv.ImportFromFile(path))
    {
        ReplyToCommand(
            client,
            "[Trigger] ERROR: Failed to read file: %s",
            path
        );

        delete kv;
        return;
    }

    kv.Rewind();


    // ========================================================
    // Navigate to correct location
    //
    // No course:
    //
    // Triggers
    //     rjai_1
    //
    //
    // With course:
    //
    // Triggers
    //     Courses
    //         Basic
    //             rjai_1
    // ========================================================

    if (strlen(courseName) > 0)
    {
        // ----------------------------------------------------
        // Enter Courses.
        // ----------------------------------------------------

        if (!kv.JumpToKey("Courses", false))
        {
            ReplyToCommand(
                client,
                "[Trigger] Courses section does not exist."
            );

            delete kv;
            return;
        }

        // ----------------------------------------------------
        // Enter requested course.
        // ----------------------------------------------------

        if (!kv.JumpToKey(courseName, false))
        {
            ReplyToCommand(
                client,
                "[Trigger] Course \"%s\" does not exist.",
                courseName
            );

            delete kv;
            return;
        }
    }


    // ========================================================
    // Enter existing trigger
    // ========================================================

    if (!kv.JumpToKey(triggerName, false))
    {
        ReplyToCommand(
            client,
            "[Trigger] Trigger \"%s\" does not exist%s.",
            triggerName,
            strlen(courseName) > 0
                ? " in the specified course"
                : ""
        );

        delete kv;
        return;
    }


    // ========================================================
    // Overwrite trigger data
    //
    // We are already inside:
    //
    // rjai_X
    //
    // so SaveTriggerData() can directly update the values.
    // ========================================================

    SaveTriggerData(
        kv,
        trigger
    );


    // ========================================================
    // Save updated file
    // ========================================================

    kv.Rewind();

    if (!kv.ExportToFile(path))
    {
        ReplyToCommand(
            client,
            "[Trigger] ERROR: Failed to save updated file: %s",
            path
        );

        delete kv;
        return;
    }


    // ========================================================
    // Success message
    // ========================================================

    if (strlen(courseName) > 0)
    {
        ReplyToCommand(
            client,
            "[Trigger] Updated \"%s\" in course \"%s\".",
            triggerName,
            courseName
        );
    }
    else
    {
        ReplyToCommand(
            client,
            "[Trigger] Updated \"%s\".",
            triggerName
        );
    }


    // ========================================================
    // Points have now been consumed.
    // ========================================================

    Reset(client);
    FireConfigChanged();

    delete kv;
}

void SaveNextCourseTrigger(int client)
{
    if (client <= 0 || !IsClientInGame(client))
    {
        return;
    }

    if (strlen(currentCourse) == 0)
    {
        ReplyToCommand(
            client,
            "[Trigger] ERROR: No course is currently set. Use \"!trigger setCurrentCourse <name>\" to set a course."
        );

        return;
    }

    if (currentNumber < 0)
    {
        ReplyToCommand(
            client,
            "[Trigger] ERROR: No trigger number is currently set. Use \"!trigger setCurrentNumber <number>\" to set a trigger number."
        );

        return;
    }

    Save(
        client,
        currentCourse,
        currentNumber
    );

    currentNumber++;
}