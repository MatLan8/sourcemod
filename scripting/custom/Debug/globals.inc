#if defined _shared_globals_included
    #endinput
#endif
#define _shared_globals_included

#define DRAW_DISTANCE      800.0
#define DRAW_INTERVAL      1.0
#define BEAM_WIDTH         4.0

int g_BeamSprite;

bool g_CheckpointDrawEnabled = false;
bool g_TriggerDrawEnabled = false;


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

ArrayList g_Checkpoints;
ArrayList g_Triggers;
