#include <sourcemod>
#include <sdktools>

#define PLANE_WIDTH        1800.0
#define PLANE_UP_HEIGHT    600.0
#define PLANE_DOWN_HEIGHT  60.0

#define PLANE_OFFSET       10.0

#define DRAW_DISTANCE      800.0

// Number of horizontal beam lines used to visually fill the plane.
// Higher = more solid-looking, but more network traffic.
#define PLANE_LINES        25

// ~66 server frames at 66 tick = ~1 second.
#define DRAW_INTERVAL      1.0

#define BEAM_WIDTH         4.0

enum struct CheckpointData
{
    int entity;
    char name[128];

    float waypoint[3];
    float angles[3];

    float forwardVec[3];
    float side[3];

    float planeCenter[3];
}

ArrayList g_Checkpoints;

int g_BeamSprite;

public void OnPluginStart()
{
    g_Checkpoints = new ArrayList(sizeof(CheckpointData));

    // Draw the checkpoint planes periodically.
    CreateTimer(
        DRAW_INTERVAL,
        Timer_DrawCheckpointPlanes,
        _,
        TIMER_REPEAT | TIMER_FLAG_NO_MAPCHANGE
    );
}

public void OnMapStart()
{
    g_BeamSprite = PrecacheModel("materials/sprites/laserbeam.vmt");

    CreateTimer(1.0, Timer_PrintCheckpoints);
    CreateTimer(1.0, Timer_BuildCheckpoints);
}

public Action Timer_PrintCheckpoints(Handle timer)
{
    PrintToServer("========================================");
    PrintToServer(" Teleport Destinations / Checkpoints");
    PrintToServer("========================================");

    int ent = -1;
    int count = 0;

    // Store the entity indexes temporarily so we don't have to
    // enumerate the entities a second time for chat.
    int entities[2048];
    int entityCount = 0;

    while ((ent = FindEntityByClassname(ent, "info_teleport_destination")) != -1)
    {
        entities[entityCount] = ent;
        entityCount++;

        float origin[3];
        float angles[3];
        char name[128];

        GetEntPropVector(ent, Prop_Data, "m_vecOrigin", origin);
        GetEntPropVector(ent, Prop_Data, "m_angRotation", angles);
        GetEntPropString(ent, Prop_Data, "m_iName", name, sizeof(name));

        PrintToServer(
            "#%d | Entity: %d | Name: \"%s\" | Pos: %.1f %.1f %.1f | Yaw: %.1f",
            count + 1,
            ent,
            name,
            origin[0],
            origin[1],
            origin[2],
            angles[1]
        );

        count++;
    }

    PrintToServer("----------------------------------------");
    PrintToServer("Total checkpoints: %d", count);
    PrintToServer("========================================");

    // Print to players in chat.
    for (int client = 1; client <= MaxClients; client++)
    {
        if (!IsClientInGame(client))
            continue;

        PrintToChat(
            client,
            "\x04[Checkpoint]\x01 Found %d teleport destinations.",
            count
        );

        for (int i = 0; i < entityCount; i++)
        {
            ent = entities[i];

            float origin[3];
            float angles[3];
            char name[128];

            GetEntPropVector(ent, Prop_Data, "m_vecOrigin", origin);
            GetEntPropVector(ent, Prop_Data, "m_angRotation", angles);
            GetEntPropString(ent, Prop_Data, "m_iName", name, sizeof(name));

            PrintToChat(
                client,
                "\x04#%d\x01 \"%s\" | Pos: %.1f %.1f %.1f | Yaw: %.1f",
                i + 1,
                name,
                origin[0],
                origin[1],
                origin[2],
                angles[1]
            );
        }
    }

    return Plugin_Stop;
}

public Action Timer_BuildCheckpoints(Handle timer)
{
    g_Checkpoints.Clear();

    int ent = -1;
    int count = 0;

    while ((ent = FindEntityByClassname(ent, "info_teleport_destination")) != -1)
    {
        CheckpointData checkpoint;

        checkpoint.entity = ent;

        GetEntPropString(
            ent,
            Prop_Data,
            "m_iName",
            checkpoint.name,
            sizeof(checkpoint.name)
        );

        GetEntPropVector(
            ent,
            Prop_Data,
            "m_vecOrigin",
            checkpoint.waypoint
        );

        GetEntPropVector(
            ent,
            Prop_Data,
            "m_angRotation",
            checkpoint.angles
        );

        // -----------------------------------------
        // Forward vector from yaw
        // -----------------------------------------

        float yawRad = DegToRad(checkpoint.angles[1]);

        checkpoint.forwardVec[0] = Cosine(yawRad);
        checkpoint.forwardVec[1] = Sine(yawRad);
        checkpoint.forwardVec[2] = 0.0;

        // -----------------------------------------
        // Sideways vector
        // -----------------------------------------

        checkpoint.side[0] = -checkpoint.forwardVec[1];
        checkpoint.side[1] = checkpoint.forwardVec[0];
        checkpoint.side[2] = 0.0;

        // -----------------------------------------
        // Plane center
        // 10 units behind the waypoint
        // -----------------------------------------

        checkpoint.planeCenter[0] =
            checkpoint.waypoint[0]
            - checkpoint.forwardVec[0] * PLANE_OFFSET;

        checkpoint.planeCenter[1] =
            checkpoint.waypoint[1]
            - checkpoint.forwardVec[1] * PLANE_OFFSET;

        checkpoint.planeCenter[2] =
            checkpoint.waypoint[2];

        // -----------------------------------------
        // Store checkpoint
        // -----------------------------------------

        g_Checkpoints.PushArray(checkpoint);

        // -----------------------------------------
        // Debug output
        // -----------------------------------------

        PrintToServer(
            "[Checkpoint %d] Entity %d | Name: \"%s\" | Pos: %.1f %.1f %.1f | Yaw: %.1f",
            count + 1,
            checkpoint.entity,
            checkpoint.name,
            checkpoint.waypoint[0],
            checkpoint.waypoint[1],
            checkpoint.waypoint[2],
            checkpoint.angles[1]
        );

        PrintToServer(
            "  Plane center: %.1f %.1f %.1f",
            checkpoint.planeCenter[0],
            checkpoint.planeCenter[1],
            checkpoint.planeCenter[2]
        );

        count++;
    }

    PrintToServer("----------------------------------------");
    PrintToServer("Built %d checkpoint planes.", count);
    PrintToServer("----------------------------------------");

    return Plugin_Stop;
}


// ============================================================
// Draw nearby checkpoint planes
// ============================================================

public Action Timer_DrawCheckpointPlanes(Handle timer)
{
    if (g_Checkpoints == null || g_Checkpoints.Length == 0)
    {
        return Plugin_Continue;
    }

    for (int client = 1; client <= MaxClients; client++)
    {
        if (!IsClientInGame(client))
        {
            continue;
        }

        if (IsClientSourceTV(client) || IsClientReplay(client))
        {
            continue;
        }

        float playerPos[3];

        GetClientAbsOrigin(client, playerPos);

        for (int i = 0; i < g_Checkpoints.Length; i++)
        {
            CheckpointData checkpoint;

            g_Checkpoints.GetArray(i, checkpoint);

            float dx = playerPos[0] - checkpoint.waypoint[0];
            float dy = playerPos[1] - checkpoint.waypoint[1];
            float dz = playerPos[2] - checkpoint.waypoint[2];

            float distanceSquared =
                dx * dx +
                dy * dy +
                dz * dz;

            if (distanceSquared > DRAW_DISTANCE * DRAW_DISTANCE)
            {
                continue;
            }

            DrawCheckpointPlane(client, checkpoint);
        }
    }

    return Plugin_Continue;
}


// ============================================================
// Draw a visually filled plane
// ============================================================

void DrawCheckpointPlane(int client, CheckpointData checkpoint)
{
    float halfWidth = PLANE_WIDTH * 0.5;

    // -----------------------------------------
    // Calculate plane corners
    // -----------------------------------------

    float topLeft[3];
    float topRight[3];
    float bottomLeft[3];
    float bottomRight[3];

    // Top-left
    topLeft[0] =
        checkpoint.planeCenter[0]
        + checkpoint.side[0] * halfWidth;

    topLeft[1] =
        checkpoint.planeCenter[1]
        + checkpoint.side[1] * halfWidth;

    topLeft[2] =
        checkpoint.planeCenter[2]
        + PLANE_UP_HEIGHT;

    // Top-right
    topRight[0] =
        checkpoint.planeCenter[0]
        - checkpoint.side[0] * halfWidth;

    topRight[1] =
        checkpoint.planeCenter[1]
        - checkpoint.side[1] * halfWidth;

    topRight[2] =
        checkpoint.planeCenter[2]
        + PLANE_UP_HEIGHT;

    // Bottom-left
    bottomLeft[0] =
        checkpoint.planeCenter[0]
        + checkpoint.side[0] * halfWidth;

    bottomLeft[1] =
        checkpoint.planeCenter[1]
        + checkpoint.side[1] * halfWidth;

    bottomLeft[2] =
        checkpoint.planeCenter[2]
        - PLANE_DOWN_HEIGHT;

    // Bottom-right
    bottomRight[0] =
        checkpoint.planeCenter[0]
        - checkpoint.side[0] * halfWidth;

    bottomRight[1] =
        checkpoint.planeCenter[1]
        - checkpoint.side[1] * halfWidth;

    bottomRight[2] =
        checkpoint.planeCenter[2]
        - PLANE_DOWN_HEIGHT;

    // -----------------------------------------
    // Color
    // -----------------------------------------

    // RGBA.
    // Alpha = 80 / 255.
    int color[4] = {0, 0, 255, 255};

    // -----------------------------------------
    // Draw horizontal lines from top to bottom
    // -----------------------------------------

    for (int i = 0; i < PLANE_LINES; i++)
    {
        float t;

        if (PLANE_LINES <= 1)
        {
            t = 0.0;
        }
        else
        {
            t = float(i) / float(PLANE_LINES - 1);
        }

        float start[3];
        float end[3];

        // Interpolate vertically along the left/right edges.
        for (int axis = 0; axis < 3; axis++)
        {
            start[axis] =
                topLeft[axis]
                + (bottomLeft[axis] - topLeft[axis]) * t;

            end[axis] =
                topRight[axis]
                + (bottomRight[axis] - topRight[axis]) * t;
        }

        TE_SetupBeamPoints(
            start,
            end,
            g_BeamSprite,
            0,              // Halo index
            0,              // Start frame
            0,              // Frame rate
            1.0,            // Life
            BEAM_WIDTH,
            BEAM_WIDTH,
            0,
            0.0,            // Amplitude
            color,
            0               // Speed
        );

        TE_SendToClient(client);
    }
}