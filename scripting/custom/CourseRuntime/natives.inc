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
        "CourseRuntime_GetCurrentCourseTriggers",
        Native_GetCurrentCourseTriggers
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