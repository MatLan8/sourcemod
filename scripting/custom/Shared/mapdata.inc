#if defined _mapdata_included
    #endinput
#endif

#define _mapdata_included

enum
{
    GEOM_SOLID      = (1 << 0),
    GEOM_PLAYERCLIP = (1 << 1),
    GEOM_WATER      = (1 << 2),
    GEOM_NOGRENADES = (1 << 3),
    GEOM_TELEPORT   = (1 << 4)
};

#define VOXEL_COUNT_X 16
#define VOXEL_COUNT_Y 16
#define VOXEL_COUNT_Z 16
#define VOXEL_COUNT    (VOXEL_COUNT_X * VOXEL_COUNT_Y * VOXEL_COUNT_Z)

#define RAY_COUNT_OMNI 64
#define RAY_LOOK_INDEX 64
#define RAY_COUNT      (RAY_COUNT_OMNI + 1)
#define RAY_MAX_RANGE  2048.0
#define RAY_LOOK_RANGE 15000.0

enum struct RayData
{
    int flags;
    float geomDistance;
    float geomNormal[3];
    float noGrenadesDistance;
    float teleportDistance;
}

native bool MapData_BuildVoxels(
    const float origin[3],
    const float angles[3],
    int[] flags,
    int maxlength
);

// Cache solid prop_static collideables for the current map.
// Call from OnMapStart only. Not needed on plugin start (map is not loaded yet)
// or round start (static props live in the BSP lump).
native int MapData_CacheStaticProps();

// Cache func_nogrenades and trigger_teleport.
// Call from OnMapStart and teamplay_round_start (edicts respawn).
// Do not call from OnPluginStart — the map is not loaded yet.
native int MapData_CacheTriggers();

native bool MapData_TraceRays(
    const float origin[3],
    const float angles[3],
    const float eyeOrigin[3],
    RayData rays[RAY_COUNT],
    int maxlength
);
