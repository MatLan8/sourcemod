#if defined _commandMenu_globals_included
    #endinput
#endif
#define _commandMenu_globals_included

enum struct CourseCheckpoints
{
    char courseName[128];
    ArrayList checkpoints;
}

ArrayList g_CourseNames;
ArrayList g_AllCoursesCheckpoints;
ArrayList g_CurrentCourseCheckpoints;