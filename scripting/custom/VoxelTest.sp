#include <sourcemod>
#include "MapData/voxel.inc"

public void OnPluginStart()
{
    PrintToServer("[VOXEL TEST] Calling Voxel_GetBrushCount()");

    int index = Voxel_GetBrushCount(
        -4096.0,
        -4096.0,
        -4096.0,
        4096.0,
        4096.0,
        4096.0
    );

    PrintToServer(
        "[VOXEL TEST] Found brush %d",
        index
    );

    if (index <= 0)
    {
        return;
    }

    float planes[MAX_BRUSH_PLANES * 4];
    int planeCount = 0;
    int contents = 0;

    PrintToServer(
        "[VOXEL TEST] Calling Voxel_GetBrushInfo(%d)",
        index
    );

    bool success = Voxel_GetBrushInfo(
        index,
        planes,
        MAX_BRUSH_PLANES,
        planeCount,
        contents
    );

    PrintToServer(
        "[VOXEL TEST] Voxel_GetBrushInfo returned %d",
        success
    );

    if (!success)
    {
        return;
    }

    PrintToServer(
        "[VOXEL TEST] Brush %d: %d planes, contents=%d",
        index,
        planeCount,
        contents
    );

    for (int i = 0; i < planeCount; i++)
    {
        PrintToServer(
            "[VOXEL TEST] Plane %d: %.4f %.4f %.4f %.4f",
            i,
            planes[i * 4 + 0],
            planes[i * 4 + 1],
            planes[i * 4 + 2],
            planes[i * 4 + 3]
        );
    }
}