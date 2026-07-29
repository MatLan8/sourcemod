#if defined _recorder_globals_included
    #endinput
#endif
#define _recorder_globals_included

bool g_Recording = false;

Handle g_PositionFile = INVALID_HANDLE;
Handle g_VelocityFile = INVALID_HANDLE;
Handle g_AnglesFile = INVALID_HANDLE;
Handle g_MovementFile = INVALID_HANDLE;
Handle g_StateFile = INVALID_HANDLE;
Handle g_RocketFile = INVALID_HANDLE;

bool g_RocketFired[MAXPLAYERS + 1];

enum struct RocketData
{
    int entity;
    int id;
    int owner;
    int spawnTick;
    bool exploded;
    int explosionTick;
    float explosionPos[3];
}

RocketData g_Rockets[2048];
int g_RocketCount = 0;
int g_NextRocketID = 1;
