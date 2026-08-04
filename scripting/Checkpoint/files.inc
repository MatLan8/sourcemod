#if defined _checkpoint_files_included
    #endinput
#endif
#define _checkpoint_files_included


#define MAP_CONFIG_PATH "C:/MATAS/RocketJumpAI/MapConfigs"

void SaveCheckpoints(bool flat = false)
{
    if (g_Checkpoints == null || g_Checkpoints.Length == 0)
    {
        PrintToServer(
            "[Checkpoint] No checkpoints available to save."
        );

        return;
    }

    // --------------------------------------------------------
    // Get current map name.
    // --------------------------------------------------------

    char mapName[128];

    GetCurrentMap(
        mapName,
        sizeof(mapName)
    );

    // --------------------------------------------------------
    // Build output filename.
    //
    // Normal:
    //     mapname.cfg
    //
    // Flat:
    //     mapname_flat.cfg
    // --------------------------------------------------------

    char path[PLATFORM_MAX_PATH];

    if (flat)
    {
        Format(
            path,
            sizeof(path),
            "%s/%s_flat.cfg",
            MAP_CONFIG_PATH,
            mapName
        );
    }
    else
    {
        Format(
            path,
            sizeof(path),
            "%s/%s.cfg",
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
    // Create KeyValues root.
    // --------------------------------------------------------

    KeyValues kv = new KeyValues("Checkpoints");

    // --------------------------------------------------------
    // Save every entry in g_Checkpoints.
    //
    // This includes:
    //   - normal checkpoints
    //   - capture areas appended by GenerateCaptures()
    //
    // Capture areas already have their unused values set to 0,
    // so we don't need any special handling here.
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

    delete kv;
}

// ============================================================
// Save a manually selected course
//
// Usage:
//     savecourse <course_name> <checkpoint_index> <index> ...
//
// Example:
//     savecourse Basic 0 55 146 21 12 69
//
// The indexes refer directly to g_Checkpoints.
// Their order determines the order inside the course.
//
// If the course already exists, it is completely replaced.
// Other courses remain untouched.
// ============================================================

void SaveCourse(int client, int args)
{
    if (args < 2)
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
            "[Checkpoint] No checkpoints available."
        );

        return;
    }

    // --------------------------------------------------------
    // Get course name.
    // --------------------------------------------------------

    char courseName[128];

    GetCmdArg(
        1,
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
    // Read and validate indexes.
    //
    // These indexes refer directly to g_Checkpoints.
    // --------------------------------------------------------

    int indexes[2048];
    int indexCount = 0;

    for (int arg = 2; arg <= args; arg++)
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

        indexes[indexCount] = index;
        indexCount++;
    }

    // --------------------------------------------------------
    // Get map config path.
    // --------------------------------------------------------

    char mapName[128];

    GetCurrentMap(
        mapName,
        sizeof(mapName)
    );

    char path[PLATFORM_MAX_PATH];

    Format(
        path,
        sizeof(path),
        "%s/%s.cfg",
        MAP_CONFIG_PATH,
        mapName
    );

    // --------------------------------------------------------
    // Load existing config.
    //
    // This preserves all other courses.
    // --------------------------------------------------------

    KeyValues kv = new KeyValues("Checkpoints");

    if (FileExists(path))
    {
        if (!kv.ImportFromFile(path))
        {
            ReplyToCommand(
                client,
                "[Checkpoint] ERROR: Could not read %s",
                path
            );

            delete kv;
            return;
        }
    }

    // --------------------------------------------------------
    // Navigate to:
    //
    // Checkpoints
    //     courses
    // --------------------------------------------------------

    kv.JumpToKey(
        "courses",
        true
    );

    // --------------------------------------------------------
    // Remove the existing course.
    // --------------------------------------------------------

    kv.DeleteKey(
        courseName
    );

    // --------------------------------------------------------
    // Create the new course.
    // --------------------------------------------------------

    kv.JumpToKey(
        courseName,
        true
    );

    // --------------------------------------------------------
    // Write checkpoints using the supplied g_Checkpoints
    // indexes, preserving the order from the command.
    // --------------------------------------------------------

    for (int i = 0; i < indexCount; i++)
    {
        CheckpointData checkpoint;

        g_Checkpoints.GetArray(
            indexes[i],
            checkpoint
        );

        char outputIndex[16];

        IntToString(
            i,
            outputIndex,
            sizeof(outputIndex)
        );

        kv.JumpToKey(
            outputIndex,
            true
        );

        SaveCheckpointData(
            kv,
            checkpoint
        );

        kv.GoBack();
    }

    // --------------------------------------------------------
    // Return to root:
    //
    // course -> courses -> Checkpoints
    // --------------------------------------------------------

    kv.GoBack();
    kv.GoBack();

    // --------------------------------------------------------
    // Export.
    // --------------------------------------------------------

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

    PrintToServer(
        "[Checkpoint] Saved course \"%s\" with %d checkpoints.",
        courseName,
        indexCount
    );

    PrintToServer(
        "[Checkpoint] File: %s",
        path
    );

    ReplyToCommand(
        client,
        "[Checkpoint] Saved course \"%s\" with %d checkpoints.",
        courseName,
        indexCount
    );

    delete kv;
}


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
        checkpoint.waypoint[0]
    );

    kv.SetFloat(
        "y",
        checkpoint.waypoint[1]
    );

    kv.SetFloat(
        "z",
        checkpoint.waypoint[2]
    );

    kv.SetFloat(
        "yaw",
        checkpoint.angles[1]
    );

    kv.SetFloat(
        "plane_x",
        checkpoint.planeCenter[0]
    );

    kv.SetFloat(
        "plane_y",
        checkpoint.planeCenter[1]
    );

    kv.SetFloat(
        "plane_z",
        checkpoint.planeCenter[2]
    );

    kv.SetFloat(
        "forward_x",
        checkpoint.forwardVec[0]
    );

    kv.SetFloat(
        "forward_y",
        checkpoint.forwardVec[1]
    );

    kv.SetFloat(
        "forward_z",
        checkpoint.forwardVec[2]
    );

    kv.SetFloat(
        "side_x",
        checkpoint.side[0]
    );

    kv.SetFloat(
        "side_y",
        checkpoint.side[1]
    );

    kv.SetFloat(
        "side_z",
        checkpoint.side[2]
    );
}