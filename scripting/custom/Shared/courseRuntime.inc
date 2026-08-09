#if defined _shared_courseRuntime_included
    #endinput
#endif

#define _shared_courseRuntime_included

forward void CourseRuntime_OnCourseDataUpdated();

native ArrayList CourseRuntime_GetCurrentCourseCheckpoints();

native ArrayList CourseRuntime_GetCurrentCourseTriggers();

native ArrayList CourseRuntime_GetAllCoursesCheckpoints();

native ArrayList CourseRuntime_GetCourseCheckpoints(int courseIndex);

native ArrayList CourseRuntime_GetCourseNames();