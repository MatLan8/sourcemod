#if defined _courseruntime_natives_included
    #endinput
#endif

#define _courseruntime_natives_included


void RegisterNatives()
{
    CreateNative(
        "CourseRuntime_GetCurrentCourseCheckpoints",
        Native_GetCurrentCourseCheckpoints
    );

    CreateNative(
        "CourseRuntime_GetCurrentCheckpoint",
        Native_GetCurrentCheckpoint
    );

    CreateNative(
        "CourseRuntime_GetCurrentCourseTriggers",
        Native_GetCurrentCourseTriggers
    );

    CreateNative(
        "CourseRuntime_GetAllCoursesCheckpoints",
        Native_GetAllCoursesCheckpoints
    );

    CreateNative(
        "CourseRuntime_GetCourseCheckpoints",
        Native_GetCourseCheckpoints
    );

    CreateNative(
        "CourseRuntime_GetCourseNames",
        Native_GetCourseNames
    );

}


// ============================================================
// Get current course checkpoints
// ============================================================

any Native_GetCurrentCourseCheckpoints(
    Handle plugin,
    int numParams
)
{
    if (
        g_AllCoursesCheckpoints == null ||
        g_CurrentCourseIndex < 0 ||
        g_CurrentCourseIndex >= g_AllCoursesCheckpoints.Length
    )
    {
        return 0;
    }

    CourseCheckpoints course;

    g_AllCoursesCheckpoints.GetArray(
        g_CurrentCourseIndex,
        course
    );

    if (course.checkpoints == null)
    {
        return 0;
    }

    return course.checkpoints;
}

any Native_GetCurrentCheckpoint(
    Handle plugin,
    int numParams
)
{
    SetNativeArray(
        1,
        g_CurrentCheckpoint,
        sizeof(g_CurrentCheckpoint)
    );

    return 0;
}

// ============================================================
// Get current course triggers
// ============================================================

any Native_GetCurrentCourseTriggers(
    Handle plugin,
    int numParams
)
{
    if (
        g_AllCoursesTriggers == null ||
        g_CurrentCourseIndex < 0 ||
        g_CurrentCourseIndex >= g_AllCoursesTriggers.Length
    )
    {
        return 0;
    }

    CourseTriggers course;

    g_AllCoursesTriggers.GetArray(
        g_CurrentCourseIndex,
        course
    );

    if (course.triggers == null)
    {
        return 0;
    }

    return course.triggers;
}

any Native_GetAllCoursesCheckpoints(
    Handle plugin,
    int numParams
)
{
    if (g_AllCoursesCheckpoints == null)
    {
        return 0;
    }

    return g_AllCoursesCheckpoints;
}

any Native_GetCourseCheckpoints(
    Handle plugin,
    int numParams
)
{
    if (g_AllCoursesCheckpoints == null)
    {
        return 0;
    }

    int courseIndex = GetNativeCell(1);

    if (
        courseIndex < 0 ||
        courseIndex >= g_AllCoursesCheckpoints.Length
    )
    {
        return 0;
    }

    CourseCheckpoints course;

    g_AllCoursesCheckpoints.GetArray(
        courseIndex,
        course
    );

    if (course.checkpoints == null)
    {
        return 0;
    }

    return course.checkpoints;
}

any Native_GetCourseNames(
    Handle plugin,
    int numParams
)
{
    ArrayList courseNames = new ArrayList(128);

    if (g_AllCoursesCheckpoints == null)
    {
        return courseNames;
    }

    for (int i = 0; i < g_AllCoursesCheckpoints.Length; i++)
    {
        CourseCheckpoints course;

        g_AllCoursesCheckpoints.GetArray(
            i,
            course
        );

        courseNames.PushString(
            course.courseName
        );
    }

    return courseNames;
}