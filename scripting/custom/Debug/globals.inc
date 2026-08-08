#if defined _shared_globals_included
    #endinput
#endif
#define _shared_globals_included

#define DRAW_DISTANCE      800.0
#define DRAW_INTERVAL      1.0
#define BEAM_WIDTH         4.0

int g_BeamSprite;

bool g_DrawEnabled = false;

enum struct CheckpointData
{
    char name[128];

    float coordinates[3];
}

char g_CurrentCourseName[128];
ArrayList g_CurrentCourse;

CheckpointData g_CurrentCheckpoint;
int g_CurrentCheckpointIndex;

#define TRIGGER_MODEL "models/error.mdl"
#define MAP_CONFIG_PATH "data/MapConfigs"

enum struct TriggerData
{
    int entity;

    char name[128];

    float worldMins[3];
    float worldMaxs[3];

    float origin[3];

    float localMins[3];
    float localMaxs[3];
}

ArrayList g_Triggers;
int g_CurrentTriggerIndex;