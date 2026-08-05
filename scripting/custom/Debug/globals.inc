#if defined _shared_globals_included
    #endinput
#endif
#define _shared_globals_included

#define PLANE_WIDTH        1800.0
#define PLANE_UP_HEIGHT    600.0
#define PLANE_DOWN_HEIGHT  60.0

#define PLANE_OFFSET       10.0

#define DRAW_DISTANCE      800.0

#define PLANE_LINES        25
#define DRAW_INTERVAL      1.0
#define BEAM_WIDTH         4.0

int g_BeamSprite;

bool g_DrawEnabled = false;

enum struct CheckpointData
{
    int entity;
    char name[128];

    float waypoint[3];
    float angles[3];

    float forwardVec[3];
    float side[3];

    float planeCenter[3];
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