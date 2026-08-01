#pragma semicolon 1


#define VOXEL_BEAM_LIFE 0.1
#define VOXEL_BEAM_WIDTH 0.4


int g_BeamSprite;



void InitDrawing()
{
    g_BeamSprite = PrecacheModel(
        "materials/sprites/laserbeam.vmt"
    );
}



void DrawVoxelVolume()
{
    DrawOuterBox();


    if(g_DrawVoxelGrid)
    {
        DrawVoxelGrid();
    }
}



void DrawOuterBox()
{
    float min[3];
    float max[3];


    min[0] = g_CurrentVolumeOrigin[0];
    min[1] = g_CurrentVolumeOrigin[1];
    min[2] = g_CurrentVolumeOrigin[2];


    max[0] = min[0] + g_BoxSize[0];
    max[1] = min[1] + g_BoxSize[1];
    max[2] = min[2] + g_BoxSize[2];

    // bottom square

    DrawLine(
        min[0],min[1],min[2],
        max[0],min[1],min[2]
    );

    DrawLine(
        max[0],min[1],min[2],
        max[0],max[1],min[2]
    );

    DrawLine(
        max[0],max[1],min[2],
        min[0],max[1],min[2]
    );

    DrawLine(
        min[0],max[1],min[2],
        min[0],min[1],min[2]
    );


    // top square

    DrawLine(
        min[0],min[1],max[2],
        max[0],min[1],max[2]
    );

    DrawLine(
        max[0],min[1],max[2],
        max[0],max[1],max[2]
    );

    DrawLine(
        max[0],max[1],max[2],
        min[0],max[1],max[2]
    );

    DrawLine(
        min[0],max[1],max[2],
        min[0],min[1],max[2]
    );


    // vertical edges

    DrawLine(
        min[0],min[1],min[2],
        min[0],min[1],max[2]
    );

    DrawLine(
        max[0],min[1],min[2],
        max[0],min[1],max[2]
    );

    DrawLine(
        max[0],max[1],min[2],
        max[0],max[1],max[2]
    );

    DrawLine(
        min[0],max[1],min[2],
        min[0],max[1],max[2]
    );
}



void DrawVoxelGrid()
{

    for(int x = 0; x < 16; x++)
    {
        for(int y = 0; y < 16; y++)
        {
            for(int z = 0; z < 8; z++)
            {
                DrawVoxelCube(
                    x,
                    y,
                    z
                );
            }
        }
    }
}


void DrawVoxelCube(
    int x,
    int y,
    int z
)
{
    float min[3];
    float max[3];


    min[0] =
        g_CurrentVolumeOrigin[0]
        + x * g_VoxelSize;


    min[1] =
        g_CurrentVolumeOrigin[1]
        + y * g_VoxelSize;


    min[2] =
        g_CurrentVolumeOrigin[2]
        + z * g_VoxelSize;



    max[0] =
        min[0]
        + g_VoxelSize;


    max[1] =
        min[1]
        + g_VoxelSize;


    max[2] =
        min[2]
        + g_VoxelSize;



    // bottom square

    DrawLine(
        min[0],min[1],min[2],
        max[0],min[1],min[2]
    );

    DrawLine(
        max[0],min[1],min[2],
        max[0],max[1],min[2]
    );

    DrawLine(
        max[0],max[1],min[2],
        min[0],max[1],min[2]
    );

    DrawLine(
        min[0],max[1],min[2],
        min[0],min[1],min[2]
    );



    // top square

    DrawLine(
        min[0],min[1],max[2],
        max[0],min[1],max[2]
    );

    DrawLine(
        max[0],min[1],max[2],
        max[0],max[1],max[2]
    );

    DrawLine(
        max[0],max[1],max[2],
        min[0],max[1],max[2]
    );

    DrawLine(
        min[0],max[1],max[2],
        min[0],min[1],max[2]
    );



    // vertical edges

    DrawLine(
        min[0],min[1],min[2],
        min[0],min[1],max[2]
    );

    DrawLine(
        max[0],min[1],min[2],
        max[0],min[1],max[2]
    );

    DrawLine(
        max[0],max[1],min[2],
        max[0],max[1],max[2]
    );

    DrawLine(
        min[0],max[1],min[2],
        min[0],max[1],max[2]
    );
}



void DrawLine(
    float x1,
    float y1,
    float z1,

    float x2,
    float y2,
    float z2
)
{
    float start[3];
    float end[3];


    start[0] = x1;
    start[1] = y1;
    start[2] = z1;


    end[0] = x2;
    end[1] = y2;
    end[2] = z2;



    TE_SetupBeamPoints(
        start,
        end,
        g_BeamSprite,
        0,
        0,
        0,
        VOXEL_BEAM_LIFE,
        VOXEL_BEAM_WIDTH,
        VOXEL_BEAM_WIDTH,
        0,
        0.0,
        {0,255,0,255},
        0
    );


    TE_SendToAll();
}