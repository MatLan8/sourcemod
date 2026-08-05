#if defined _debug_checkpoint_included
    #endinput
#endif

#define _debug_checkpoint_included

// ============================================================
// Drawing
// ============================================================

void DrawCheckpointPlanes()
{
    if (g_CurrentCourse == null ||
        g_CurrentCourse.Length == 0)
    {
        return;
    }

    for (int client = 1; client <= MaxClients; client++)
    {
        if (!IsClientInGame(client))
        {
            continue;
        }

        if (IsClientSourceTV(client) ||
            IsClientReplay(client))
        {
            continue;
        }

        float playerPos[3];

        GetClientAbsOrigin(
            client,
            playerPos
        );

        for (int i = 0; i < g_CurrentCourse.Length; i++)
        {
            CheckpointData checkpoint;

            g_CurrentCourse.GetArray(
                i,
                checkpoint
            );

            float dx =
                playerPos[0] -
                checkpoint.waypoint[0];

            float dy =
                playerPos[1] -
                checkpoint.waypoint[1];

            float dz =
                playerPos[2] -
                checkpoint.waypoint[2];

            float distanceSquared =
                dx * dx +
                dy * dy +
                dz * dz;

            if (distanceSquared >
                DRAW_DISTANCE * DRAW_DISTANCE)
            {
                continue;
            }

            DrawCheckpointPlane(
                client,
                checkpoint
            );
        }
    }
}


// ============================================================
// Draw individual checkpoint plane
// ============================================================

void DrawCheckpointPlane(
    int client,
    CheckpointData checkpoint
)
{
    float halfWidth =
        PLANE_WIDTH * 0.5;

    float topLeft[3];
    float topRight[3];

    float bottomLeft[3];
    float bottomRight[3];

    // --------------------------------------------------------
    // Top-left
    // --------------------------------------------------------

    topLeft[0] =
        checkpoint.planeCenter[0] +
        checkpoint.side[0] * halfWidth;

    topLeft[1] =
        checkpoint.planeCenter[1] +
        checkpoint.side[1] * halfWidth;

    topLeft[2] =
        checkpoint.planeCenter[2] +
        PLANE_UP_HEIGHT;

    // --------------------------------------------------------
    // Top-right
    // --------------------------------------------------------

    topRight[0] =
        checkpoint.planeCenter[0] -
        checkpoint.side[0] * halfWidth;

    topRight[1] =
        checkpoint.planeCenter[1] -
        checkpoint.side[1] * halfWidth;

    topRight[2] =
        checkpoint.planeCenter[2] +
        PLANE_UP_HEIGHT;

    // --------------------------------------------------------
    // Bottom-left
    // --------------------------------------------------------

    bottomLeft[0] =
        checkpoint.planeCenter[0] +
        checkpoint.side[0] * halfWidth;

    bottomLeft[1] =
        checkpoint.planeCenter[1] +
        checkpoint.side[1] * halfWidth;

    bottomLeft[2] =
        checkpoint.planeCenter[2] -
        PLANE_DOWN_HEIGHT;

    // --------------------------------------------------------
    // Bottom-right
    // --------------------------------------------------------

    bottomRight[0] =
        checkpoint.planeCenter[0] -
        checkpoint.side[0] * halfWidth;

    bottomRight[1] =
        checkpoint.planeCenter[1] -
        checkpoint.side[1] * halfWidth;

    bottomRight[2] =
        checkpoint.planeCenter[2] -
        PLANE_DOWN_HEIGHT;

    // Saturated, fully opaque blue.
    int color[4] =
    {
        0,
        0,
        255,
        255
    };

    // --------------------------------------------------------
    // Fill plane with horizontal beams
    // --------------------------------------------------------

    for (int i = 0; i < PLANE_LINES; i++)
    {
        float t;

        if (PLANE_LINES <= 1)
        {
            t = 0.0;
        }
        else
        {
            t =
                float(i) /
                float(PLANE_LINES - 1);
        }

        float start[3];
        float end[3];

        for (int axis = 0; axis < 3; axis++)
        {
            start[axis] =
                topLeft[axis] +
                (bottomLeft[axis] -
                 topLeft[axis]) * t;

            end[axis] =
                topRight[axis] +
                (bottomRight[axis] -
                 topRight[axis]) * t;
        }

        TE_SetupBeamPoints(
            start,
            end,
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
}