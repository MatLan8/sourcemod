#if defined _checkpoint_files_included
    #endinput
#endif

#define _checkpoint_files_included

void GetCheckpointCourseName(
    const char[] checkpointName,
    char[] courseName,
    int maxlen
)
{
    strcopy(courseName, maxlen, checkpointName);

    int len = strlen(checkpointName);

    if (len == 0)
    {
        return;
    }

    // --------------------------------------------------------
    // Look for the final underscore.
    // --------------------------------------------------------

    int lastUnderscore = -1;

    for (int i = len - 1; i >= 0; i--)
    {
        if (checkpointName[i] == '_')
        {
            lastUnderscore = i;
            break;
        }
    }

    if (lastUnderscore != -1)
    {
        char suffix[64];

        strcopy(
            suffix,
            sizeof(suffix),
            checkpointName[lastUnderscore + 1]
        );

        // ----------------------------------------------------
        // course_5
        // course_10
        // ----------------------------------------------------

        if (IsNumericString(suffix))
        {
            courseName[lastUnderscore] = '\0';
            return;
        }

        // ----------------------------------------------------
        // course_t8
        // course_t9
        // ----------------------------------------------------

        if (suffix[0] == 't' || suffix[0] == 'T')
        {
            char numberPart[32];

            strcopy(
                numberPart,
                sizeof(numberPart),
                suffix[1]
            );

            if (IsNumericString(numberPart))
            {
                courseName[lastUnderscore] = '\0';
                return;
            }
        }

        // ----------------------------------------------------
        // course_start
        // ----------------------------------------------------

        if (StrEqual(suffix, "start", false))
        {
            courseName[lastUnderscore] = '\0';
            return;
        }
    }

    // --------------------------------------------------------
    // Handle names without an underscore:
    //
    // HardPogo11 -> HardPogo
    // hell3      -> hell
    //
    // Remove trailing digits.
    // --------------------------------------------------------

    int end = len;

    while (end > 0)
    {
        char c = checkpointName[end - 1];

        if (c < '0' || c > '9')
        {
            break;
        }

        end--;
    }

    if (end < len)
    {
        courseName[end] = '\0';
    }
}



bool IsNumericString(const char[] value)
{
    int len = strlen(value);

    if (len == 0)
    {
        return false;
    }

    for (int i = 0; i < len; i++)
    {
        if (value[i] < '0' || value[i] > '9')
        {
            return false;
        }
    }

    return true;
}


#define MAP_CONFIG_PATH "C:/MATAS/RocketJumpAI/MapConfigs"

void SaveCheckpoints(bool courses = false)
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
    // Build absolute config path.
    // --------------------------------------------------------

    char path[PLATFORM_MAX_PATH];

    Format(
        path,
        sizeof(path),
        "%s/%s.cfg",
        MAP_CONFIG_PATH,
        mapName
    );

    PrintToServer(
        "[Checkpoint] Saving to: %s",
        path
    );

    // --------------------------------------------------------
    // Create KeyValues root.
    // --------------------------------------------------------

    KeyValues kv = new KeyValues("Checkpoints");

    if (!courses)
    {
        SaveCheckpointsFlat(kv);
    }
    else
    {
        SaveCheckpointsByCourse(kv);
    }

    // --------------------------------------------------------
    // Write file.
    // --------------------------------------------------------

    if (!kv.ExportToFile(path))
    {
        PrintToServer(
            "[Checkpoint] ERROR: Failed to save checkpoints to:"
        );

        PrintToServer(
            "%s",
            path
        );

        delete kv;
        return;
    }

    PrintToServer(
        "[Checkpoint] Saved %d checkpoints.",
        g_Checkpoints.Length
    );

    PrintToServer(
        "[Checkpoint] File: %s",
        path
    );

    if (courses)
    {
        PrintToServer(
            "[Checkpoint] Checkpoints were grouped by course."
        );
    }

    delete kv;
}


// ============================================================
// Save flat checkpoint structure
// ============================================================

void SaveCheckpointsFlat(KeyValues kv)
{
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
}


// ============================================================
// Save checkpoints grouped by course
// ============================================================

void SaveCheckpointsByCourse(KeyValues kv)
{
    // --------------------------------------------------------
    // Root:
    //
    // "courses"
    // {
    //     ...
    // }
    // --------------------------------------------------------

    kv.JumpToKey(
        "courses",
        true
    );

    // --------------------------------------------------------
    // We don't know the course names beforehand.
    //
    // Therefore process each checkpoint and create its course
    // section when needed.
    // --------------------------------------------------------

    int courseCounts = 0;

    // Keep track of course names we've already created.
    ArrayList courseNames = new ArrayList(
        64
    );

    for (int i = 0; i < g_Checkpoints.Length; i++)
    {
        CheckpointData checkpoint;

        g_Checkpoints.GetArray(
            i,
            checkpoint
        );

        char courseName[128];

        GetCheckpointCourseName(
            checkpoint.name,
            courseName,
            sizeof(courseName)
        );

        // ----------------------------------------------------
        // Check whether this course already exists.
        // ----------------------------------------------------

        int courseIndex = -1;

        for (int j = 0; j < courseNames.Length; j++)
        {
            char existingName[128];

            courseNames.GetString(
                j,
                existingName,
                sizeof(existingName)
            );

            if (StrEqual(
                    existingName,
                    courseName,
                    false
                ))
            {
                courseIndex = j;
                break;
            }
        }

        // ----------------------------------------------------
        // First checkpoint belonging to this course.
        // ----------------------------------------------------

        if (courseIndex == -1)
        {
            courseIndex = courseNames.Length;

            courseNames.PushString(
                courseName
            );

            courseCounts++;
        }

        // ----------------------------------------------------
        // Go to the course.
        // ----------------------------------------------------

        kv.JumpToKey(
            courseName,
            true
        );

        // ----------------------------------------------------
        // Determine index inside the course.
        //
        // This preserves the current order of g_Checkpoints.
        // ----------------------------------------------------

        int checkpointIndex = 0;

        char existingIndex[16];

        while (true)
        {
            IntToString(
                checkpointIndex,
                existingIndex,
                sizeof(existingIndex)
            );

            if (!kv.JumpToKey(
                    existingIndex,
                    false
                ))
            {
                break;
            }

            kv.GoBack();

            checkpointIndex++;
        }

        // ----------------------------------------------------
        // Create checkpoint entry.
        // ----------------------------------------------------

        kv.JumpToKey(
            existingIndex,
            true
        );

        SaveCheckpointData(
            kv,
            checkpoint
        );

        kv.GoBack();

        // Back out of course.
        kv.GoBack();
    }

    delete courseNames;

    // Back out of "courses".
    kv.GoBack();

    PrintToServer(
        "[Checkpoint] Created %d course groups.",
        courseCounts
    );
}


// ============================================================
// Write one checkpoint's data into the current KeyValues node
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