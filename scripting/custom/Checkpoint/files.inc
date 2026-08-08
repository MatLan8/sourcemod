#if defined _checkpoint_files_included
    #endinput
#endif
#define _checkpoint_files_included

void SaveCheckpoints(bool flat = false)
{
    if (g_Checkpoints == null || g_Checkpoints.Length == 0)
    {
        PrintToServer(
            "[Checkpoint] No checkpoints available to save."
        );

        return;
    }

    char mapName[128];

    GetCurrentMap(
        mapName,
        sizeof(mapName)
    );

    char path[PLATFORM_MAX_PATH];

    if (flat)
    {
        BuildPath(
            Path_SM,
            path,
            sizeof(path),
            "%s/%s_checkpoints_flat.cfg",
            MAP_CONFIG_PATH,
            mapName
        );
    }
    else
    {
        BuildPath(
            Path_SM,
            path,
            sizeof(path),
            "%s/%s_checkpoints.cfg",
            MAP_CONFIG_PATH,
            mapName
        );
    }

    PrintToServer(
        "[Checkpoint] Saving %d entries to: %s",
        g_Checkpoints.Length,
        path
    );

    // --------------------------------------------------------
    // Create directory if necessary.
    // --------------------------------------------------------

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

    // --------------------------------------------------------
    // Create KeyValues root.
    // --------------------------------------------------------

    KeyValues kv = new KeyValues("Checkpoints");

    // --------------------------------------------------------
    // Save every entry in g_Checkpoints.
    //
    // This includes:
    //   - normal checkpoints
    //   - capture areas appended by GenerateCaptures()
    // --------------------------------------------------------

    for (int i = 0; i < g_Checkpoints.Length; i++)
    {
        CheckpointData checkpoint;

        g_Checkpoints.GetArray(
            i,
            checkpoint
        );

        char index[16];

        IntToString(
            i,
            index,
            sizeof(index)
        );

        kv.JumpToKey(
            index,
            true
        );

        SaveCheckpointData(
            kv,
            checkpoint
        );

        kv.GoBack();
    }

    // --------------------------------------------------------
    // Export.
    //
    // ExportToFile overwrites the existing file.
    // --------------------------------------------------------

    if (!kv.ExportToFile(path))
    {
        PrintToServer(
            "[Checkpoint] ERROR: Failed to save checkpoints."
        );

        PrintToServer(
            "[Checkpoint] Path: %s",
            path
        );

        delete kv;
        return;
    }

    PrintToServer(
        "[Checkpoint] Successfully saved %d entries.",
        g_Checkpoints.Length
    );

    PrintToServer(
        "[Checkpoint] File: %s",
        path
    );

    FireConfigChanged();
    delete kv;
}


// ============================================================
// Save a manually selected course
//
// Usage:
//     sm_checkpoint savecourse <course> <index> <index> ...
//
// Example:
//     sm_checkpoint savecourse Basic 0 55 146 21 12 69
//
// The indexes refer directly to g_Checkpoints.
// If the course already exists, only that course is replaced.
// All other courses remain untouched.
// ============================================================

void SaveCourse(int client, int args)
{
    // --------------------------------------------------------
    // Validate arguments.
    // --------------------------------------------------------

    if (args < 3)
    {
        ReplyToCommand(
            client,
            "[Checkpoint] Usage: savecourse <course> <index> <index> ..."
        );

        return;
    }

    if (g_Checkpoints == null || g_Checkpoints.Length == 0)
    {
        ReplyToCommand(
            client,
            "[Checkpoint] No checkpoints generated."
        );

        return;
    }

    // --------------------------------------------------------
    // Argument 1 = course name.
    // Arguments 2+ = checkpoint indexes.
    // --------------------------------------------------------

    char courseName[128];

    GetCmdArg(
        2,
        courseName,
        sizeof(courseName)
    );

    if (strlen(courseName) == 0)
    {
        ReplyToCommand(
            client,
            "[Checkpoint] Invalid course name."
        );

        return;
    }

    // --------------------------------------------------------
    // Read checkpoint indexes.
    // --------------------------------------------------------

    int requested[2048];
    int requestedCount = 0;

    for (int arg = 3; arg <= args; arg++)
    {
        char buffer[32];

        GetCmdArg(
            arg,
            buffer,
            sizeof(buffer)
        );

        int index = StringToInt(buffer);

        if (index < 0 || index >= g_Checkpoints.Length)
        {
            ReplyToCommand(
                client,
                "[Checkpoint] Invalid checkpoint index: %d",
                index
            );

            return;
        }

        requested[requestedCount] = index;
        requestedCount++;
    }

    char mapName[128];

    GetCurrentMap(
        mapName,
        sizeof(mapName)
    );

    char path[PLATFORM_MAX_PATH];

    BuildPath(
        Path_SM,
        path,
        sizeof(path),
        "%s/%s_checkpoints.cfg",
        MAP_CONFIG_PATH,
        mapName
    );

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


    KeyValues kv = new KeyValues("Checkpoints");

    if (FileExists(path))
    {
        if (!kv.ImportFromFile(path))
        {
            ReplyToCommand(
                client,
                "[Checkpoint] ERROR: Failed to read existing file: %s",
                path
            );

            delete kv;
            return;
        }
    }

    kv.Rewind();

    kv.JumpToKey(
        "Courses",
        true
    );

    kv.DeleteKey(
        courseName
    );

    kv.JumpToKey(
        courseName,
        true
    );

    for (int i = 0; i < requestedCount; i++)
    {
        int checkpointIndex = requested[i];

        CheckpointData checkpoint;

        g_Checkpoints.GetArray(
            checkpointIndex,
            checkpoint
        );

        char indexString[16];

        IntToString(
            i,
            indexString,
            sizeof(indexString)
        );

        kv.JumpToKey(
            indexString,
            true
        );

        SaveCheckpointData(
            kv,
            checkpoint
        );

        kv.GoBack();
    }

    kv.Rewind();

    if (!kv.ExportToFile(path))
    {
        ReplyToCommand(
            client,
            "[Checkpoint] ERROR: Failed to save %s",
            path
        );

        delete kv;
        return;
    }

    ReplyToCommand(
        client,
        "[Checkpoint] Saved course \"%s\" with %d checkpoints.",
        courseName,
        requestedCount
    );

    FireConfigChanged();
    delete kv;
}

bool Update(int client)
{
    // --------------------------------------------------------
    // Validate generated checkpoints.
    // --------------------------------------------------------

    if (
        g_Checkpoints == null ||
        g_Checkpoints.Length == 0
    )
    {
        ReplyToCommand(
            client,
            "[Checkpoint] No generated checkpoints available to update."
        );

        return false;
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
    // Build config path.
    // --------------------------------------------------------

    char path[PLATFORM_MAX_PATH];

    BuildPath(
        Path_SM,
        path,
        sizeof(path),
        "%s/%s_checkpoints.cfg",
        MAP_CONFIG_PATH,
        mapName
    );


    // --------------------------------------------------------
    // Make sure config exists.
    // --------------------------------------------------------

    if (!FileExists(path))
    {
        ReplyToCommand(
            client,
            "[Checkpoint] Configuration file does not exist: %s",
            path
        );

        return false;
    }


    // --------------------------------------------------------
    // Load existing KeyValues.
    // --------------------------------------------------------

    KeyValues kv = new KeyValues("Checkpoints");

    if (!kv.ImportFromFile(path))
    {
        ReplyToCommand(
            client,
            "[Checkpoint] ERROR: Failed to read checkpoint file: %s",
            path
        );

        delete kv;
        return false;
    }


    // --------------------------------------------------------
    // Track which generated checkpoints were found.
    //
    // This is only for diagnostics.
    //
    // Maximum of 2048 generated checkpoints.
    // --------------------------------------------------------

    bool matched[2048];

    for (
        int i = 0;
        i < g_Checkpoints.Length && i < sizeof(matched);
        i++
    )
    {
        matched[i] = false;
    }


    int updatedCount = 0;


    // ========================================================
    // Determine configuration structure.
    // ========================================================

    kv.Rewind();

    bool hasCourses = kv.JumpToKey(
        "Courses",
        false
    );


    if (hasCourses)
    {
        // ====================================================
        // COURSE CONFIG
        //
        // Current position:
        //
        // Checkpoints -> Courses
        // ====================================================

        if (kv.GotoFirstSubKey(false))
        {
            do
            {
                // --------------------------------------------
                // Current position:
                //
                // Checkpoints -> Courses -> <course>
                //
                // Process the numeric checkpoint entries.
                // --------------------------------------------

                UpdateCheckpointList(
                    kv,
                    matched,
                    updatedCount
                );

            }
            while (kv.GotoNextKey(false));
        }
    }
    else
    {
        // ====================================================
        // FLAT CONFIG
        //
        // Current position:
        //
        // Checkpoints
        //
        // Numeric checkpoint entries are directly underneath.
        // ====================================================

        kv.Rewind();

        UpdateCheckpointList(
            kv,
            matched,
            updatedCount
        );
    }


    // ========================================================
    // Save the modified KeyValues.
    //
    // Since we never delete/recreate checkpoint sections,
    // their original ordering and structure are retained.
    // ========================================================

    kv.Rewind();

    if (!kv.ExportToFile(path))
    {
        ReplyToCommand(
            client,
            "[Checkpoint] ERROR: Failed to save updated checkpoint file: %s",
            path
        );

        delete kv;
        return false;
    }


    // ========================================================
    // Count generated checkpoints that weren't found.
    // ========================================================

    int unmatchedCount = 0;

    for (
        int i = 0;
        i < g_Checkpoints.Length;
        i++
    )
    {
        if (
            i >= sizeof(matched) ||
            !matched[i]
        )
        {
            unmatchedCount++;
        }
    }


    // ========================================================
    // Diagnostics.
    // ========================================================

    PrintToServer(
        "[Checkpoint] Updated %d checkpoint(s).",
        updatedCount
    );


    if (unmatchedCount > 0)
    {
        PrintToServer(
            "[Checkpoint] %d generated checkpoint(s) were not found in the config.",
            unmatchedCount
        );
    }


    // ========================================================
    // Client feedback.
    // ========================================================

    ReplyToCommand(
        client,
        "[Checkpoint] Updated %d checkpoint(s).",
        updatedCount
    );


    if (unmatchedCount > 0)
    {
        ReplyToCommand(
            client,
            "[Checkpoint] %d generated checkpoint(s) were not found in the config.",
            unmatchedCount
        );
    }


    delete kv;
    
    FireConfigChanged();
    return true;
}


// ============================================================
// UpdateCheckpointList
//
// Updates checkpoint entries directly underneath the current
// KeyValues section.
//
// Current position can be:
//
// Checkpoints
//
// OR:
//
// Checkpoints -> Courses -> basic
//
// The function only processes numeric child sections.
// ============================================================

void UpdateCheckpointList(
    KeyValues kv,
    bool matched[2048],
    int &updatedCount
)
{
    // --------------------------------------------------------
    // Make sure this section contains children.
    // --------------------------------------------------------

    if (!kv.GotoFirstSubKey(false))
    {
        return;
    }


    do
    {
        char sectionName[128];

        kv.GetSectionName(
            sectionName,
            sizeof(sectionName)
        );


        // ----------------------------------------------------
        // Only numeric sections represent checkpoints.
        //
        // Examples:
        //
        // "0"
        // "1"
        // "2"
        //
        // Ignore anything else.
        // ----------------------------------------------------

        if (!IsNumericString(sectionName))
        {
            continue;
        }


        // ----------------------------------------------------
        // Read the existing checkpoint name.
        // ----------------------------------------------------

        char existingName[128];

        kv.GetString(
            "name",
            existingName,
            sizeof(existingName),
            ""
        );


        if (strlen(existingName) == 0)
        {
            continue;
        }


        // ----------------------------------------------------
        // Find matching generated checkpoint.
        // ----------------------------------------------------

        int generatedIndex =
            FindGeneratedCheckpointByName(
                existingName
            );


        if (generatedIndex == -1)
        {
            // ------------------------------------------------
            // No generated checkpoint with this name.
            //
            // Leave the existing entry untouched.
            // ------------------------------------------------

            continue;
        }


        // ----------------------------------------------------
        // Get generated checkpoint data.
        // ----------------------------------------------------

        CheckpointData checkpoint;

        g_Checkpoints.GetArray(
            generatedIndex,
            checkpoint
        );


        // ----------------------------------------------------
        // Remove obsolete properties.
        //
        // The numeric section itself is NOT deleted.
        //
        // Therefore:
        //
        // "0"
        // {
        //     ...
        // }
        //
        // remains exactly where it was.
        // ----------------------------------------------------

        DeleteLegacyCheckpointProperties(
            kv
        );


        // ----------------------------------------------------
        // Write the new minimal checkpoint data.
        // ----------------------------------------------------

        SaveCheckpointData(
            kv,
            checkpoint
        );


        // ----------------------------------------------------
        // Mark generated checkpoint as matched.
        // ----------------------------------------------------

        if (
            generatedIndex >= 0 &&
            generatedIndex < sizeof(matched)
        )
        {
            matched[generatedIndex] = true;
        }


        updatedCount++;

    }
    while (kv.GotoNextKey(false));


    // --------------------------------------------------------
    // Return to parent.
    // --------------------------------------------------------

    kv.GoBack();
}


// ============================================================
// FindGeneratedCheckpointByName
//
// Searches g_Checkpoints for a checkpoint matching name.
//
// Returns:
//     Generated checkpoint index.
//
// Returns:
//     -1 if no matching checkpoint exists.
// ============================================================

int FindGeneratedCheckpointByName(
    const char[] name
)
{
    if (
        g_Checkpoints == null ||
        g_Checkpoints.Length == 0
    )
    {
        return -1;
    }


    for (
        int i = 0;
        i < g_Checkpoints.Length;
        i++
    )
    {
        CheckpointData checkpoint;

        g_Checkpoints.GetArray(
            i,
            checkpoint
        );


        if (StrEqual(
                checkpoint.name,
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
// DeleteLegacyCheckpointProperties
//
// Removes all properties from the old checkpoint format that
// are no longer needed.
//
// The checkpoint KeyValues section itself is preserved.
//
// Old:
//
// "0"
// {
//     "name"
//     "x"
//     "y"
//     "z"
//     "yaw"
//     "plane_x"
//     "plane_y"
//     "plane_z"
//     "forward_x"
//     "forward_y"
//     "forward_z"
//     "side_x"
//     "side_y"
//     "side_z"
// }
//
// After this function:
//
// "0"
// {
//     "name"
//     "x"
//     "y"
//     "z"
// }
// ============================================================

void DeleteLegacyCheckpointProperties(
    KeyValues kv
)
{
    kv.DeleteKey("yaw");

    kv.DeleteKey("plane_x");
    kv.DeleteKey("plane_y");
    kv.DeleteKey("plane_z");

    kv.DeleteKey("forward_x");
    kv.DeleteKey("forward_y");
    kv.DeleteKey("forward_z");

    kv.DeleteKey("side_x");
    kv.DeleteKey("side_y");
    kv.DeleteKey("side_z");
}


// ============================================================
// SaveCheckpointData
//
// Writes the new minimal checkpoint format.
// ============================================================

void SaveCheckpointData(
    KeyValues kv,
    CheckpointData checkpoint
)
{
    kv.SetString(
        "name",
        checkpoint.name
    );

    kv.SetFloat(
        "x",
        checkpoint.coordinates[0]
    );

    kv.SetFloat(
        "y",
        checkpoint.coordinates[1]
    );

    kv.SetFloat(
        "z",
        checkpoint.coordinates[2]
    );
}


bool IsNumericString(
    const char[] value
)
{
    int length = strlen(value);

    if (length == 0)
    {
        return false;
    }


    for (
        int i = 0;
        i < length;
        i++
    )
    {
        if (
            value[i] < '0' ||
            value[i] > '9'
        )
        {
            return false;
        }
    }


    return true;
}
