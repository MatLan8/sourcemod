#if defined _trigger_globals_included
    #endinput
#endif
#define _trigger_globals_included

float g_TriggerPoints[2][3];
int g_TriggerPointCount;

enum struct TriggerData
{
    char name[128];

    float worldMins[3];
    float worldMaxs[3];

    float origin[3];

    float localMins[3];
    float localMaxs[3];
}

// forwards
Handle g_OnTriggerConfigChanged;