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


// Volume center (player origin + offset) and yaw basis.
// Matches the extension: forward=(cos,sin,0), right=(sin,-cos,0), up=(0,0,1).
float g_VolumeCenter[3];
float g_VolumeRight[3];
float g_VolumeForward[3];
float g_VolumeUp[3];


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



void LocalToWorld(
    float localX,
    float localY,
    float localZ,
    float world[3]
)
{
    world[0] =
        g_VolumeCenter[0]
        + g_VolumeRight[0] * localX
        + g_VolumeForward[0] * localY
        + g_VolumeUp[0] * localZ;

    world[1] =
        g_VolumeCenter[1]
        + g_VolumeRight[1] * localX
        + g_VolumeForward[1] * localY
        + g_VolumeUp[1] * localZ;

    world[2] =
        g_VolumeCenter[2]
        + g_VolumeRight[2] * localX
        + g_VolumeForward[2] * localY
        + g_VolumeUp[2] * localZ;
}


void UpdateVoxelVolume(
    int client
)
{
    float playerOrigin[3];
    float angles[3];

    GetClientAbsOrigin(client, playerOrigin);
    GetClientEyeAngles(client, angles);

    g_VolumeCenter[0] = playerOrigin[0] + g_BoxOffset[0];
    g_VolumeCenter[1] = playerOrigin[1] + g_BoxOffset[1];
    g_VolumeCenter[2] = playerOrigin[2] + g_BoxOffset[2];

    float yaw = DegToRad(angles[1]);
    float cy = Cosine(yaw);
    float sy = Sine(yaw);

    g_VolumeForward[0] = cy;
    g_VolumeForward[1] = sy;
    g_VolumeForward[2] = 0.0;

    g_VolumeRight[0] = sy;
    g_VolumeRight[1] = -cy;
    g_VolumeRight[2] = 0.0;

    g_VolumeUp[0] = 0.0;
    g_VolumeUp[1] = 0.0;
    g_VolumeUp[2] = 1.0;

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


                LocalToWorld(
                    -(g_BoxSize[0] / 2.0) + (x * g_VoxelSize) + (g_VoxelSize / 2.0),
                    -(g_BoxSize[1] / 2.0) + (y * g_VoxelSize) + (g_VoxelSize / 2.0),
                    -(g_BoxSize[2] / 2.0) + (z * g_VoxelSize) + (g_VoxelSize / 2.0),
                    g_VoxelPositions[index]
                );


                index++;
            }
        }
    }
}