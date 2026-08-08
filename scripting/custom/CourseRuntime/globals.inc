#if defined _courseruntime_globals_included
    #endinput
#endif
#define _courseruntime_globals_included

#define MAP_CONFIG_PATH "data/MapConfigs"
#define MAX_NAME_LENGTH 128
#define DEFAULT_COURSE_NAME "_Default_"

enum struct TriggerCfgData
{
    char name[128];

    float worldMins[3];
    float worldMaxs[3];

    float origin[3];

    float localMins[3];
    float localMaxs[3];
}

enum struct CheckpointData
{
    char name[128];
    float coordinates[3];
}

enum struct TriggerData
{
    char name[128];

    float worldMins[3];
    float worldMaxs[3];

    int entity;
}

enum struct CourseCheckpoints
{
    char courseName[128];
    ArrayList checkpoints;
}

enum struct CourseTriggers
{
    char courseName[128];
    ArrayList triggers;
}

ArrayList g_AllCoursesCheckpoints;
ArrayList g_AllCoursesTriggers;

ArrayList g_CourseTriggers;
ArrayList g_CourseCheckpoints;

int g_CurrentCourseIndex = -1;
int g_CurrentCheckpointIndex = -1;
int g_CurrentTriggerIndex = -1;

CheckpointData g_CurrentCheckpoint;
TriggerData g_CurrentTrigger;