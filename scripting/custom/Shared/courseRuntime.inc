#if defined _shared_courseRuntime_included
    #endinput
#endif

#define _shared_courseRuntime_included

public SharedPlugin __pl_courseruntime =
{
    name = "CourseRuntime",
    file = "CourseRuntime.smx",

#if defined REQUIRE_PLUGIN
    required = 1,
#else
    required = 0,
#endif
};

forward void CourseRuntime_OnCourseDataUpdated();

native ArrayList CourseRuntime_GetCurrentCourseCheckpoints();

native ArrayList CourseRuntime_GetCurrentCourseTriggers();

native ArrayList CourseRuntime_GetAllCoursesCheckpoints();

native ArrayList CourseRuntime_GetCourseCheckpoints(int courseIndex);

native ArrayList CourseRuntime_GetCourseNames();