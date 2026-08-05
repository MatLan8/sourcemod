#if defined _shared_checkpoint_included
    #endinput
#endif
#define _shared_checkpoint_included

#define CHECKPOINT_CONFIG_PATH "data/MapConfigs"


// ============================================================
// SetCourse
//
// If courseName is provided:
//     Loads that specific course.
//
// If courseName is empty:
//     If the map has "Courses", loads the first course.
//     Otherwise loads the entire flat checkpoint list.
//
// After loading:
//     - g_CurrentCourseName is updated
//     - g_CurrentCheckpointIndex = 0
//     - g_CurrentCheckpoint = first checkpoint
// ============================================================

bool SetCourse(const char[] courseName = "")
{
    // --------------------------------------------------------
    // Clear previous course.
    // --------------------------------------------------------

    if (g_CurrentCourse != null)
    {
        delete g_CurrentCourse;
    }

    g_CurrentCourse = new ArrayList(sizeof(CheckpointData));

    g_CurrentCourseName[0] = '\0';
    g_CurrentCheckpointIndex = 0;

    // --------------------------------------------------------
    // Get map name.
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
        "%s/%s.cfg",
        CHECKPOINT_CONFIG_PATH,
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

    // --------------------------------------------------------
    // Check whether the file contains Courses.
    // --------------------------------------------------------

    kv.Rewind();

    bool hasCourses = kv.JumpToKey(
        "Courses",
        false
    );

    // ========================================================
    // CASE 1:
    //
    // No Courses key.
    //
    // The entire config is one course.
    // ========================================================

    if (!hasCourses)
    {
        kv.Rewind();

        LoadCheckpointList(
            kv,
            g_CurrentCourse
        );

        strcopy(
            g_CurrentCourseName,
            sizeof(g_CurrentCourseName),
            mapName
        );

        delete kv;

        if (g_CurrentCourse.Length == 0)
        {
            PrintToServer(
                "[Checkpoint] ERROR: No checkpoints found in %s.",
                path
            );

            return false;
        }

        g_CurrentCheckpointIndex = 0;

        g_CurrentCourse.GetArray(
            0,
            g_CurrentCheckpoint
        );

        PrintToServer(
            "[Checkpoint] Loaded flat course \"%s\" with %d checkpoints.",
            g_CurrentCourseName,
            g_CurrentCourse.Length
        );

        return true;
    }

    // ========================================================
    // CASE 2:
    //
    // Courses exists.
    // ========================================================

    char selectedCourse[128];

    // --------------------------------------------------------
    // Explicit course name supplied.
    // --------------------------------------------------------

    if (strlen(courseName) > 0)
    {
        strcopy(
            selectedCourse,
            sizeof(selectedCourse),
            courseName
        );

        // Try to find requested course.
        if (!kv.JumpToKey(selectedCourse, false))
        {
            PrintToServer(
                "[Checkpoint] ERROR: Course \"%s\" was not found.",
                selectedCourse
            );

            delete kv;
            return false;
        }
    }
    else
    {
        // ----------------------------------------------------
        // No course specified.
        //
        // Select the first course in Courses.
        // ----------------------------------------------------

        if (!kv.GotoFirstSubKey(false))
        {
            PrintToServer(
                "[Checkpoint] ERROR: Courses section is empty."
            );

            delete kv;
            return false;
        }

        kv.GetSectionName(
            selectedCourse,
            sizeof(selectedCourse)
        );
    }

    // --------------------------------------------------------
    // We are now inside the selected course.
    // --------------------------------------------------------

    LoadCheckpointList(
        kv,
        g_CurrentCourse
    );

    strcopy(
        g_CurrentCourseName,
        sizeof(g_CurrentCourseName),
        selectedCourse
    );

    delete kv;

    // --------------------------------------------------------
    // Validate course.
    // --------------------------------------------------------

    if (g_CurrentCourse.Length == 0)
    {
        PrintToServer(
            "[Checkpoint] ERROR: Course \"%s\" contains no checkpoints.",
            g_CurrentCourseName
        );

        return false;
    }

    // --------------------------------------------------------
    // Set first checkpoint.
    // --------------------------------------------------------

    g_CurrentCheckpointIndex = 0;

    g_CurrentCourse.GetArray(
        0,
        g_CurrentCheckpoint
    );

    PrintToServer(
        "[Checkpoint] Loaded course \"%s\" with %d checkpoints.",
        g_CurrentCourseName,
        g_CurrentCourse.Length
    );

    PrintToServer(
        "[Checkpoint] Starting checkpoint: 0 \"%s\"",
        g_CurrentCheckpoint.name
    );

    return true;
}


// ============================================================
// LoadCheckpointList
//
// Loads all numbered checkpoint entries from the current
// KeyValues position into an ArrayList.
//
// Expected structure:
//
// "0"
// {
//     "name" ...
//     "x" ...
//     ...
// }
//
// "1"
// {
//     ...
// }
// ============================================================

void LoadCheckpointList(
    KeyValues kv,
    ArrayList checkpoints
)
{
    kv.GotoFirstSubKey(false);

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
    checkpoint.entity = -1;

    kv.GetString(
        "name",
        checkpoint.name,
        sizeof(checkpoint.name),
        ""
    );

    checkpoint.waypoint[0] = kv.GetFloat("x");
    checkpoint.waypoint[1] = kv.GetFloat("y");
    checkpoint.waypoint[2] = kv.GetFloat("z");

    checkpoint.angles[0] = kv.GetFloat("yaw");
    checkpoint.angles[1] = 0.0;
    checkpoint.angles[2] = 0.0;

    checkpoint.planeCenter[0] = kv.GetFloat("plane_x");
    checkpoint.planeCenter[1] = kv.GetFloat("plane_y");
    checkpoint.planeCenter[2] = kv.GetFloat("plane_z");

    checkpoint.forwardVec[0] = kv.GetFloat("forward_x");
    checkpoint.forwardVec[1] = kv.GetFloat("forward_y");
    checkpoint.forwardVec[2] = kv.GetFloat("forward_z");

    checkpoint.side[0] = kv.GetFloat("side_x");
    checkpoint.side[1] = kv.GetFloat("side_y");
    checkpoint.side[2] = kv.GetFloat("side_z");
}


// ============================================================
// UpdateCheckpoint
//
// Normal:
//
//     UpdateCheckpoint()
//
// Advances to the next checkpoint.
//
// Returns:
//     false = advanced successfully
//     true  = current checkpoint was already the last one
//
// Optional index:
//
//     UpdateCheckpoint(10)
//
// Directly selects checkpoint 10.
//
// Returns true if the selected checkpoint is the last one.
// ============================================================

bool UpdateCheckpoint(int index = -1)
{
    if (g_CurrentCourse == null || g_CurrentCourse.Length == 0)
    {
        return false;
    }

    // ========================================================
    // Explicit checkpoint index.
    // ========================================================

    if (index >= 0)
    {
        if (index >= g_CurrentCourse.Length)
        {
            PrintToServer(
                "[Checkpoint] ERROR: Invalid checkpoint index %d.",
                index
            );

            return false;
        }

        g_CurrentCheckpointIndex = index;

        g_CurrentCourse.GetArray(
            g_CurrentCheckpointIndex,
            g_CurrentCheckpoint
        );

        return (
            g_CurrentCheckpointIndex ==
            g_CurrentCourse.Length - 1
        );
    }

    // ========================================================
    // Already at final checkpoint.
    // ========================================================

    if (
        g_CurrentCheckpointIndex >=
        g_CurrentCourse.Length - 1
    )
    {
        return true;
    }

    // ========================================================
    // Advance to next checkpoint.
    // ========================================================

    g_CurrentCheckpointIndex++;

    g_CurrentCourse.GetArray(
        g_CurrentCheckpointIndex,
        g_CurrentCheckpoint
    );

    return false;
}


// ============================================================
// IsNumericString
//
// Used because SourcePawn doesn't have a convenient
// starts-with / numeric string helper.
//
// Accepts:
//     "0"
//     "1"
//     "123"
//
// Rejects:
//     "Courses"
//     "Basic"
// ============================================================

bool IsNumericString(const char[] value)
{
    int length = strlen(value);

    if (length == 0)
    {
        return false;
    }

    for (int i = 0; i < length; i++)
    {
        if (value[i] < '0' || value[i] > '9')
        {
            return false;
        }
    }

    return true;
}