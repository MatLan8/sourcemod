#if defined _checkpoint_globals_included
    #endinput
#endif
#define _checkpoint_globals_included

#define PLANE_WIDTH        1800.0
#define PLANE_UP_HEIGHT    600.0
#define PLANE_DOWN_HEIGHT  60.0

#define PLANE_OFFSET       10.0

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

ArrayList g_Checkpoints;