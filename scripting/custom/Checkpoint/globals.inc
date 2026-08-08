#if defined _checkpoint_globals_included
    #endinput
#endif
#define _checkpoint_globals_included

#define MAP_CONFIG_PATH "data/MapConfigs"

enum struct CheckpointData
{
    char name[128];

    float coordinates[3];
}

ArrayList g_Checkpoints;

Handle g_OnCheckpointConfigChanged;