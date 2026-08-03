#if defined _checkpoint_globals_included
    #endinput
#endif
#define _checkpoint_globals_included

#define PLANE_WIDTH        1800.0
#define PLANE_UP_HEIGHT    600.0
#define PLANE_DOWN_HEIGHT  60.0

#define PLANE_OFFSET       10.0

#define DRAW_DISTANCE      800.0

#define PLANE_LINES        25
#define DRAW_INTERVAL      1.0
#define BEAM_WIDTH         4.0

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

int g_BeamSprite;

bool g_DrawEnabled = false;