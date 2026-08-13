#if defined _voxel_included
    #endinput
#endif

#define _voxel_included

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