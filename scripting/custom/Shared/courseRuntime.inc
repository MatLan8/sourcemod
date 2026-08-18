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

enum struct CheckpointData
{
    char name[128];

    float coordinates[3];
}

forward void CourseRuntime_OnCourseDataUpdated();

forward void CourseRuntime_OnCheckpointEntered();

native ArrayList CourseRuntime_GetCurrentCourseCheckpoints();

native void CourseRuntime_GetCurrentCheckpoint(CheckpointData checkpoint);

native ArrayList CourseRuntime_GetCurrentCourseTriggers();

native ArrayList CourseRuntime_GetAllCoursesCheckpoints();

native ArrayList CourseRuntime_GetCourseCheckpoints(int courseIndex);

native ArrayList CourseRuntime_GetCourseNames();