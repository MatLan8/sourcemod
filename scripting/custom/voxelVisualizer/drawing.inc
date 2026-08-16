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
       DrawVoxelCube(15,15,15);
    }
}



void DrawOrientedBox(
    float minX,
    float minY,
    float minZ,
    float maxX,
    float maxY,
    float maxZ
)
{
    float c000[3];
    float c100[3];
    float c110[3];
    float c010[3];
    float c001[3];
    float c101[3];
    float c111[3];
    float c011[3];

    LocalToWorld(minX, minY, minZ, c000);
    LocalToWorld(maxX, minY, minZ, c100);
    LocalToWorld(maxX, maxY, minZ, c110);
    LocalToWorld(minX, maxY, minZ, c010);
    LocalToWorld(minX, minY, maxZ, c001);
    LocalToWorld(maxX, minY, maxZ, c101);
    LocalToWorld(maxX, maxY, maxZ, c111);
    LocalToWorld(minX, maxY, maxZ, c011);

    DrawWorldLine(c000, c100);
    DrawWorldLine(c100, c110);
    DrawWorldLine(c110, c010);
    DrawWorldLine(c010, c000);

    DrawWorldLine(c001, c101);
    DrawWorldLine(c101, c111);
    DrawWorldLine(c111, c011);
    DrawWorldLine(c011, c001);

    DrawWorldLine(c000, c001);
    DrawWorldLine(c100, c101);
    DrawWorldLine(c110, c111);
    DrawWorldLine(c010, c011);
}


void DrawOuterBox()
{
    float halfX = g_BoxSize[0] / 2.0;
    float halfY = g_BoxSize[1] / 2.0;
    float halfZ = g_BoxSize[2] / 2.0;

    DrawOrientedBox(
        -halfX,
        -halfY,
        -halfZ,
        halfX,
        halfY,
        halfZ
    );
}


void DrawVoxelCube(
    int x,
    int y,
    int z
)
{
    float halfX = g_BoxSize[0] / 2.0;
    float halfY = g_BoxSize[1] / 2.0;
    float halfZ = g_BoxSize[2] / 2.0;

    float minX = -halfX + (x * g_VoxelSize);
    float minY = -halfY + (y * g_VoxelSize);
    float minZ = -halfZ + (z * g_VoxelSize);

    DrawOrientedBox(
        minX,
        minY,
        minZ,
        minX + g_VoxelSize,
        minY + g_VoxelSize,
        minZ + g_VoxelSize
    );
}



void DrawWorldLine(
    const float start[3],
    const float end[3]
)
{
    DrawLine(
        start[0],
        start[1],
        start[2],
        end[0],
        end[1],
        end[2]
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