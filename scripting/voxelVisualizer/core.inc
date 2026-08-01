#pragma semicolon 1


#define MAX_VOXELS 4096


bool g_VoxelEnabled = false;


// Box dimensions in Hammer Units
float g_BoxSize[3] =
{
    256.0,
    256.0,
    256.0
};


// Offset from player origin
float g_BoxOffset[3] =
{
    0.0,
    0.0,
    32.0
};


// Voxel size
float g_VoxelSize = 16.0;


// Number of voxels
int g_VoxelCount[3];


// Player volume origin
float g_CurrentVolumeOrigin[3];


// Voxel positions
float g_VoxelPositions[MAX_VOXELS][3];

int g_VoxelOwner = 0;
bool g_DrawVoxelGrid = false;

void InitVoxelSystem()
{
    RecalculateVoxelLayout();
}



void RecalculateVoxelLayout()
{
    g_VoxelCount[0] =
        RoundToFloor(
            g_BoxSize[0] / g_VoxelSize
        );


    g_VoxelCount[1] =
        RoundToFloor(
            g_BoxSize[1] / g_VoxelSize
        );


    g_VoxelCount[2] =
        RoundToFloor(
            g_BoxSize[2] / g_VoxelSize
        );


    PrintToServer(
        "[Voxel] Grid %d x %d x %d",
        g_VoxelCount[0],
        g_VoxelCount[1],
        g_VoxelCount[2]
    );
}



void UpdateVoxelVolume(
    int client
)
{
    float playerOrigin[3];


    GetClientAbsOrigin(
        client,
        playerOrigin
    );


    g_CurrentVolumeOrigin[0] =
        playerOrigin[0]
        - (g_BoxSize[0] / 2.0)
        + g_BoxOffset[0];


    g_CurrentVolumeOrigin[1] =
        playerOrigin[1]
        - (g_BoxSize[1] / 2.0)
        + g_BoxOffset[1];


    g_CurrentVolumeOrigin[2] =
        playerOrigin[2]
        - (g_BoxSize[2] / 2.0)
        + g_BoxOffset[2];



    CalculateVoxelPositions();
}



void CalculateVoxelPositions()
{
    int index = 0;


    for(int x = 0; x < g_VoxelCount[0]; x++)
    {
        for(int y = 0; y < g_VoxelCount[1]; y++)
        {
            for(int z = 0; z < g_VoxelCount[2]; z++)
            {

                if(index >= MAX_VOXELS)
                    return;


                g_VoxelPositions[index][0] =
                    g_CurrentVolumeOrigin[0]
                    + (x * g_VoxelSize)
                    + (g_VoxelSize / 2.0);


                g_VoxelPositions[index][1] =
                    g_CurrentVolumeOrigin[1]
                    + (y * g_VoxelSize)
                    + (g_VoxelSize / 2.0);


                g_VoxelPositions[index][2] =
                    g_CurrentVolumeOrigin[2]
                    + (z * g_VoxelSize)
                    + (g_VoxelSize / 2.0);


                index++;
            }
        }
    }
}