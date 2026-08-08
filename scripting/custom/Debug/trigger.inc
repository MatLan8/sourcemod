#if defined _debug_trigger_included
    #endinput
#endif

#define _debug_trigger_included


// ============================================================
// DrawTriggers
//
// Draws all current-course triggers that are within
// DRAW_DISTANCE of each client.
// ============================================================

void DrawTriggers()
{
    if (
        g_Triggers == null ||
        g_Triggers.Length == 0
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

        for (int i = 0; i < g_Triggers.Length; i++)
        {
            TriggerData trigger;

            g_Triggers.GetArray(
                i,
                trigger
            );

            // ------------------------------------------------
            // Calculate trigger center.
            // ------------------------------------------------

            float center[3];

            center[0] =
                (trigger.worldMins[0] +
                 trigger.worldMaxs[0]) * 0.5;

            center[1] =
                (trigger.worldMins[1] +
                 trigger.worldMaxs[1]) * 0.5;

            center[2] =
                (trigger.worldMins[2] +
                 trigger.worldMaxs[2]) * 0.5;

            // ------------------------------------------------
            // Distance check.
            // ------------------------------------------------

            float dx =
                playerPos[0] -
                center[0];

            float dy =
                playerPos[1] -
                center[1];

            float dz =
                playerPos[2] -
                center[2];

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

            DrawTriggerBox(
                client,
                trigger
            );
        }
    }
}


// ============================================================
// DrawTriggerBox
//
// Draws a wireframe box using the trigger's world-space mins
// and maxs.
//
// worldMins/worldMaxs are already in world coordinates, so
// no origin/local-mins conversion is required.
// ============================================================

void DrawTriggerBox(
    int client,
    TriggerData trigger
)
{
    float corners[8][3];

    // --------------------------------------------------------
    // Bottom
    // --------------------------------------------------------

    // 0 = min X, min Y, min Z
    corners[0][0] = trigger.worldMins[0];
    corners[0][1] = trigger.worldMins[1];
    corners[0][2] = trigger.worldMins[2];

    // 1 = max X, min Y, min Z
    corners[1][0] = trigger.worldMaxs[0];
    corners[1][1] = trigger.worldMins[1];
    corners[1][2] = trigger.worldMins[2];

    // 2 = max X, max Y, min Z
    corners[2][0] = trigger.worldMaxs[0];
    corners[2][1] = trigger.worldMaxs[1];
    corners[2][2] = trigger.worldMins[2];

    // 3 = min X, max Y, min Z
    corners[3][0] = trigger.worldMins[0];
    corners[3][1] = trigger.worldMaxs[1];
    corners[3][2] = trigger.worldMins[2];

    // --------------------------------------------------------
    // Top
    // --------------------------------------------------------

    // 4 = min X, min Y, max Z
    corners[4][0] = trigger.worldMins[0];
    corners[4][1] = trigger.worldMins[1];
    corners[4][2] = trigger.worldMaxs[2];

    // 5 = max X, min Y, max Z
    corners[5][0] = trigger.worldMaxs[0];
    corners[5][1] = trigger.worldMins[1];
    corners[5][2] = trigger.worldMaxs[2];

    // 6 = max X, max Y, max Z
    corners[6][0] = trigger.worldMaxs[0];
    corners[6][1] = trigger.worldMaxs[1];
    corners[6][2] = trigger.worldMaxs[2];

    // 7 = min X, max Y, max Z
    corners[7][0] = trigger.worldMins[0];
    corners[7][1] = trigger.worldMaxs[1];
    corners[7][2] = trigger.worldMaxs[2];

    // --------------------------------------------------------
    // Color
    // --------------------------------------------------------

    int color[4] =
    {
        255,
        0,
        0,
        255
    };

    // --------------------------------------------------------
    // Bottom face
    // --------------------------------------------------------

    DrawTriggerBeam(
        client,
        corners[0],
        corners[1],
        color
    );

    DrawTriggerBeam(
        client,
        corners[1],
        corners[2],
        color
    );

    DrawTriggerBeam(
        client,
        corners[2],
        corners[3],
        color
    );

    DrawTriggerBeam(
        client,
        corners[3],
        corners[0],
        color
    );

    // --------------------------------------------------------
    // Top face
    // --------------------------------------------------------

    DrawTriggerBeam(
        client,
        corners[4],
        corners[5],
        color
    );

    DrawTriggerBeam(
        client,
        corners[5],
        corners[6],
        color
    );

    DrawTriggerBeam(
        client,
        corners[6],
        corners[7],
        color
    );

    DrawTriggerBeam(
        client,
        corners[7],
        corners[4],
        color
    );

    // --------------------------------------------------------
    // Vertical edges
    // --------------------------------------------------------

    DrawTriggerBeam(
        client,
        corners[0],
        corners[4],
        color
    );

    DrawTriggerBeam(
        client,
        corners[1],
        corners[5],
        color
    );

    DrawTriggerBeam(
        client,
        corners[2],
        corners[6],
        color
    );

    DrawTriggerBeam(
        client,
        corners[3],
        corners[7],
        color
    );
}


// ============================================================
// DrawTriggerBeam
// ============================================================

void DrawTriggerBeam(
    int client,
    float start[3],
    float end[3],
    int color[4]
)
{
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
