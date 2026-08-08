#if defined _debug_checkpoint_included
    #endinput
#endif

#define _debug_checkpoint_included

#define CHECKPOINT_MARKER_SIZE 40.0


void DrawCheckpoints()
{
    if (
        g_Checkpoints == null ||
        g_Checkpoints.Length == 0
    )
    {
        return;
    }


    for (int client = 1; client <= MaxClients; client++)
    {
        if (!IsClientInGame(client))
        {
            continue;
        }

        if (
            IsClientSourceTV(client) ||
            IsClientReplay(client)
        )
        {
            continue;
        }


        float playerPos[3];

        GetClientAbsOrigin(
            client,
            playerPos
        );


        for (int i = 0; i < g_Checkpoints.Length; i++)
        {
            CheckpointData checkpoint;

            g_Checkpoints.GetArray(
                i,
                checkpoint
            );


            // ------------------------------------------------
            // Distance check.
            // ------------------------------------------------

            float dx =
                playerPos[0] -
                checkpoint.coordinates[0];

            float dy =
                playerPos[1] -
                checkpoint.coordinates[1];

            float dz =
                playerPos[2] -
                checkpoint.coordinates[2];

            float distanceSquared =
                dx * dx +
                dy * dy +
                dz * dz;


            if (
                distanceSquared >
                DRAW_DISTANCE *
                DRAW_DISTANCE
            )
            {
                continue;
            }


            DrawCheckpointMarker(
                client,
                checkpoint
            );
        }
    }
}




void DrawCheckpointMarker(
    int client,
    CheckpointData checkpoint
)
{
    float center[3];

    center[0] = checkpoint.coordinates[0];
    center[1] = checkpoint.coordinates[1];
    center[2] = checkpoint.coordinates[2] + 20.0;


    // --------------------------------------------------------
    // Marker dimensions.
    // --------------------------------------------------------

    float halfSize = CHECKPOINT_MARKER_SIZE * 0.5;


    // --------------------------------------------------------
    // Four corners of the horizontal square.
    // --------------------------------------------------------

    float topLeft[3];
    float topRight[3];
    float bottomLeft[3];
    float bottomRight[3];


    topLeft[0] =
        center[0] - halfSize;

    topLeft[1] =
        center[1] - halfSize;

    topLeft[2] =
        center[2];


    topRight[0] =
        center[0] + halfSize;

    topRight[1] =
        center[1] - halfSize;

    topRight[2] =
        center[2];


    bottomLeft[0] =
        center[0] - halfSize;

    bottomLeft[1] =
        center[1] + halfSize;

    bottomLeft[2] =
        center[2];


    bottomRight[0] =
        center[0] + halfSize;

    bottomRight[1] =
        center[1] + halfSize;

    bottomRight[2] =
        center[2];


    // --------------------------------------------------------
    // Marker color.
    // --------------------------------------------------------

    int color[4] =
    {
        0,
        0,
        255,
        255
    };


    // --------------------------------------------------------
    // Draw square.
    //
    // Since all four points have the same Z coordinate,
    // the marker is always horizontal and does not depend
    // on the player's viewing angle.
    // --------------------------------------------------------

    TE_SetupBeamPoints(
        topLeft,
        topRight,
        g_BeamSprite,
        0,
        0,
        0,
        1.0,
        BEAM_WIDTH,
        BEAM_WIDTH,
        0,
        0.0,
        color,
        0
    );

    TE_SendToClient(client);


    TE_SetupBeamPoints(
        topRight,
        bottomRight,
        g_BeamSprite,
        0,
        0,
        0,
        1.0,
        BEAM_WIDTH,
        BEAM_WIDTH,
        0,
        0.0,
        color,
        0
    );

    TE_SendToClient(client);


    TE_SetupBeamPoints(
        bottomRight,
        bottomLeft,
        g_BeamSprite,
        0,
        0,
        0,
        1.0,
        BEAM_WIDTH,
        BEAM_WIDTH,
        0,
        0.0,
        color,
        0
    );

    TE_SendToClient(client);


    TE_SetupBeamPoints(
        bottomLeft,
        topLeft,
        g_BeamSprite,
        0,
        0,
        0,
        1.0,
        BEAM_WIDTH,
        BEAM_WIDTH,
        0,
        0.0,
        color,
        0
    );

    TE_SendToClient(client);
}
