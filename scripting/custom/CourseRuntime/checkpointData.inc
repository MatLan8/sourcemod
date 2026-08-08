#if defined _courseruntime_checkpointData_included
    #endinput
#endif

#define _courseruntime_checkpointData_included

bool LoadCheckpoints()
{
    // --------------------------------------------------------
    // Clear previously loaded courses.
    // --------------------------------------------------------

    if (g_AllCoursesCheckpoints != null)
    {
        for (int i = 0; i < g_AllCoursesCheckpoints.Length; i++)
        {
            CourseCheckpoints course;

            g_AllCoursesCheckpoints.GetArray(
                i,
                course
            );

            if (course.checkpoints != null)
            {
                delete course.checkpoints;
            }
        }

        delete g_AllCoursesCheckpoints;
    }


    g_AllCoursesCheckpoints = new ArrayList(
        sizeof(CourseCheckpoints)
    );


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


    if (!FileExists(path))
    {
        PrintToServer(
            "[Checkpoint] ERROR: Map config does not exist: %s",
            path
        );

        return false;
    }


    // --------------------------------------------------------
    // Load KeyValues.
    // --------------------------------------------------------

    KeyValues kv = new KeyValues("Checkpoints");

    if (!kv.ImportFromFile(path))
    {
        PrintToServer(
            "[Checkpoint] ERROR: Failed to load map config: %s",
            path
        );

        delete kv;

        return false;
    }


    kv.Rewind();


    // ========================================================
    // Check for Courses.
    // ========================================================

    bool hasCourses = kv.JumpToKey(
        "Courses",
        false
    );


    // ========================================================
    // CASE 1:
    //
    // No Courses key.
    //
    // The entire file is one course.
    // ========================================================

    if (!hasCourses)
    {
        CourseCheckpoints course;

        strcopy(
            course.courseName,
            sizeof(course.courseName),
            DEFAULT_COURSE_NAME
        );

        course.checkpoints = new ArrayList(
            sizeof(CheckpointData)
        );


        kv.Rewind();

        LoadCheckpointList(
            kv,
            course.checkpoints
        );


        if (course.checkpoints.Length > 0)
        {
            g_AllCoursesCheckpoints.PushArray(
                course
            );

            PrintToServer(
                "[Checkpoint] Loaded default course \"%s\" with %d checkpoints.",
                course.courseName,
                course.checkpoints.Length
            );
        }
        else
        {
            delete course.checkpoints;

            PrintToServer(
                "[Checkpoint] ERROR: No checkpoints found in %s.",
                path
            );

            delete kv;

            return false;
        }
    }


    // ========================================================
    // CASE 2:
    //
    // Courses key exists.
    //
    // Load every course.
    // ========================================================

    else
    {
        // ----------------------------------------------------
        // We are currently inside "Courses".
        // ----------------------------------------------------

        if (!kv.GotoFirstSubKey(false))
        {
            PrintToServer(
                "[Checkpoint] ERROR: Courses section is empty."
            );

            delete kv;

            return false;
        }


        do
        {
            char courseName[128];

            kv.GetSectionName(
                courseName,
                sizeof(courseName)
            );


            // ------------------------------------------------
            // Create course container.
            // ------------------------------------------------

            CourseCheckpoints course;

            strcopy(
                course.courseName,
                sizeof(course.courseName),
                courseName
            );

            course.checkpoints = new ArrayList(
                sizeof(CheckpointData)
            );


            // ------------------------------------------------
            // Load checkpoints from this course.
            // ------------------------------------------------

            LoadCheckpointList(
                kv,
                course.checkpoints
            );


            // ------------------------------------------------
            // Store course.
            // ------------------------------------------------

            if (course.checkpoints.Length > 0)
            {
                g_AllCoursesCheckpoints.PushArray(
                    course
                );

                PrintToServer(
                    "[Checkpoint] Loaded course \"%s\" with %d checkpoints.",
                    course.courseName,
                    course.checkpoints.Length
                );
            }
            else
            {
                PrintToServer(
                    "[Checkpoint] WARNING: Course \"%s\" contains no checkpoints.",
                    course.courseName
                );

                delete course.checkpoints;
            }

        }
        while (kv.GotoNextKey(false));
    }


    delete kv;


    // --------------------------------------------------------
    // Final validation.
    // --------------------------------------------------------

    if (g_AllCoursesCheckpoints.Length == 0)
    {
        PrintToServer(
            "[Checkpoint] ERROR: No checkpoint courses were loaded."
        );

        return false;
    }


    PrintToServer(
        "[Checkpoint] Loaded %d checkpoint courses.",
        g_AllCoursesCheckpoints.Length
    );


    return true;
}


// ============================================================
// LoadCheckpointList
//
// Loads all numeric checkpoint sections from the current
// KeyValues position into the supplied ArrayList.
//
// Example:
//
// "0"
// {
//     "name" "Basic_Start"
//     "x" "-1216.0"
//     "y" "-13122.0"
//     "z" "-11401.0"
// }
// ============================================================

void LoadCheckpointList(
    KeyValues kv,
    ArrayList checkpoints
)
{
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
        // Only accept numeric checkpoint keys.
        // ----------------------------------------------------

        if (!IsNumericString(sectionName))
        {
            continue;
        }


        CheckpointData checkpoint;

        LoadCheckpointData(
            kv,
            checkpoint
        );


        checkpoints.PushArray(
            checkpoint
        );

    }
    while (kv.GotoNextKey(false));


    kv.GoBack();
}


// ============================================================
// LoadCheckpointData
//
// Converts one KeyValues checkpoint into CheckpointData.
// ============================================================

void LoadCheckpointData(
    KeyValues kv,
    CheckpointData checkpoint
)
{
    kv.GetString(
        "name",
        checkpoint.name,
        sizeof(checkpoint.name),
        ""
    );


    checkpoint.coordinates[0] =
        kv.GetFloat("x");

    checkpoint.coordinates[1] =
        kv.GetFloat("y");

    checkpoint.coordinates[2] =
        kv.GetFloat("z");
}


// ============================================================
// IsNumericString
//
// Accepts:
//
//     "0"
//     "1"
//     "123"
//
// Rejects:
//
//     "Courses"
//     "Basic"
//     "foo"
// ============================================================

bool IsNumericString(
    const char[] value
)
{
    int length = strlen(value);


    if (length == 0)
    {
        return false;
    }


    for (int i = 0; i < length; i++)
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
