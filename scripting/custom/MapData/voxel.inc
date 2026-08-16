#if defined _voxel_included
    #endinput
#endif

#define _voxel_included

enum
{
    GEOM_SOLID      = (1 << 0),
    GEOM_PLAYERCLIP = (1 << 1),
    GEOM_WATER      = (1 << 2),
    GEOM_NOGRENADES = (1 << 3),
    GEOM_REGENERATE = (1 << 4),
    GEOM_TELEPORT   = (1 << 5)
};

#define VOXEL_COUNT_X 16
#define VOXEL_COUNT_Y 16
#define VOXEL_COUNT_Z 16
#define VOXEL_COUNT    (VOXEL_COUNT_X * VOXEL_COUNT_Y * VOXEL_COUNT_Z)

native int Voxel_GetBrushCount(
    float minX,
    float minY,
    float minZ,
    float maxX,
    float maxY,
    float maxZ
);

native bool Voxel_TestGetBrushInfo(
    int brushIndex
);

#define MAX_BRUSH_PLANES 64

native bool Voxel_GetBrushInfo(
    int brushIndex,
    float planes[MAX_BRUSH_PLANES * 4],
    int maxPlanes,
    int &planeCount,
    int &contents
);

native bool Voxel_TestWriteArray(int[] array, int maxlength);

native bool Voxel_BuildCache(
    const float origin[3],
    const float angles[3],
    int[] flags,
    int maxlength
);

// Cache solid prop_static collideables for the current map.
// Call from OnMapStart and OnPluginStart (late load). Not needed on round start.
native int Voxel_CacheStaticProps();
