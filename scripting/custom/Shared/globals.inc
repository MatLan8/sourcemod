#if defined _shared_globals_included
    #endinput
#endif
#define _shared_globals_included

#define MAX_ROCKET_SLOTS 12
#define MAP_CONFIG_PATH "data/MapConfigs"
bool g_RocketFired[MAXPLAYERS + 1];
int g_PlayerButtons[MAXPLAYERS + 1];
int g_RegenTouchCount[MAXPLAYERS + 1];

enum struct PlayerSnapshot
{
    float position[3];
    float velocity[3];
    float viewAngles[3];
    int regenerate;
    int grounded;
    int waterLevel;
    int ammo;
}

enum struct PlayerInput
{
    bool pressedForward;
    bool pressedBackward;
    bool pressedLeft;
    bool pressedRight;
    bool pressedJump;
    bool pressedDuck;
    bool pressedAttack;
}

enum struct RocketSlot
{
    bool exists;
    int entity;
    int id;
    int spawnTick;
    float position[3];
    float velocity[3];
    float spawnPitch;
    float spawnYaw;
    bool exploded;
    float explosionPosition[3];
    bool clearAfterRecord;
}

RocketSlot g_RocketSlots[MAX_ROCKET_SLOTS];
int g_NextRocketID = 1;

CheckpointData g_CurrentCheckpoint;