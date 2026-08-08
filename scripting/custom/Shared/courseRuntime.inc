#if defined _shared_courseRuntime_included
    #endinput
#endif

#define _shared_courseRuntime_included

forward void CourseRuntime_OnCourseDataUpdated();

native ArrayList CourseRuntime_GetCurrentCourseCheckpoints();

native ArrayList CourseRuntime_GetCurrentCourseTriggers();