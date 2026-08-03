#if defined _checkpoint_manage_included
    #endinput
#endif

#define _checkpoint_manage_included


// ============================================================
// Checkpoint generation
// ============================================================

void GenerateCheckpoints(int client)
{
    g_Checkpoints.Clear();

    int ent = -1;
    int count = 0;

    while ((ent = FindEntityByClassname(
        ent,
        "info_teleport_destination"
    )) != -1)
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

        // ----------------------------------------------------
        // Forward vector from yaw
        // ----------------------------------------------------

        float yawRad = DegToRad(checkpoint.angles[1]);

        checkpoint.forwardVec[0] = Cosine(yawRad);
        checkpoint.forwardVec[1] = Sine(yawRad);
        checkpoint.forwardVec[2] = 0.0;

        // ----------------------------------------------------
        // Side vector
        // ----------------------------------------------------

        checkpoint.side[0] = -checkpoint.forwardVec[1];
        checkpoint.side[1] = checkpoint.forwardVec[0];
        checkpoint.side[2] = 0.0;

        // ----------------------------------------------------
        // Plane center
        // 10 HU behind waypoint
        // ----------------------------------------------------

        checkpoint.planeCenter[0] =
            checkpoint.waypoint[0]
            - checkpoint.forwardVec[0] * PLANE_OFFSET;

        checkpoint.planeCenter[1] =
            checkpoint.waypoint[1]
            - checkpoint.forwardVec[1] * PLANE_OFFSET;

        checkpoint.planeCenter[2] =
            checkpoint.waypoint[2];

        g_Checkpoints.PushArray(checkpoint);

        count++;
    }

    PrintToServer(
        "[Checkpoint] Generated %d checkpoints.",
        count
    );

    if (client > 0 && IsClientInGame(client))
    {
        PrintToChat(
            client,
            "\x04[Checkpoint]\x01 Generated %d checkpoints.",
            count
        );
    }
}


// ============================================================
// Print checkpoints
// ============================================================

void PrintCheckpoints(int client)
{
    PrintToServer("========================================");
    PrintToServer(" Checkpoints");
    PrintToServer("========================================");

    if (g_Checkpoints == null || g_Checkpoints.Length == 0)
    {
        PrintToServer("No checkpoints.");

        if (client > 0 && IsClientInGame(client))
        {
            PrintToChat(
                client,
                "\x04[Checkpoint]\x01 No checkpoints."
            );
        }

        return;
    }

    for (int i = 0; i < g_Checkpoints.Length; i++)
    {
        CheckpointData checkpoint;

        g_Checkpoints.GetArray(i, checkpoint);

        PrintToServer(
            "[%d] \"%s\" | Pos: %.1f %.1f %.1f",
            i,
            checkpoint.name,
            checkpoint.waypoint[0],
            checkpoint.waypoint[1],
            checkpoint.waypoint[2]
        );
    }

    PrintToServer("----------------------------------------");
    PrintToServer(
        "Total checkpoints: %d",
        g_Checkpoints.Length
    );
    PrintToServer("========================================");

    if (client > 0 && IsClientInGame(client))
    {
        PrintToChat(
            client,
            "\x04[Checkpoint]\x01 %d checkpoints. See server console.",
            g_Checkpoints.Length
        );
    }
}


// ============================================================
// Save checkpoints
// ============================================================

// void SaveCheckpoints(int client)
// {
//     if (g_Checkpoints == null || g_Checkpoints.Length == 0)
//     {
//         ReplyToCommand(
//             client,
//             "[Checkpoint] No checkpoints to save."
//         );

//         return;
//     }

//     char mapName[128];

//     GetCurrentMap(
//         mapName,
//         sizeof(mapName)
//     );

//     char path[PLATFORM_MAX_PATH];

//     BuildPath(
//         Path_SM,
//         path,
//         sizeof(path),
//         "configs/RocketJumpAI/%s.cfg",
//         mapName
//     );

//     KeyValues kv = new KeyValues("checkpoints");


//     for (int i = 0; i < g_Checkpoints.Length; i++)
//     {
//         CheckpointData checkpoint;

//         g_Checkpoints.GetArray(
//             i,
//             checkpoint
//         );

//         char index[16];

//         IntToString(
//             i,
//             index,
//             sizeof(index)
//         );

//         kv.JumpToKey(
//             index,
//             true
//         );

//         kv.SetString(
//             "name",
//             checkpoint.name
//         );

//         kv.SetFloat(
//             "x",
//             checkpoint.waypoint[0]
//         );

//         kv.SetFloat(
//             "y",
//             checkpoint.waypoint[1]
//         );

//         kv.SetFloat(
//             "z",
//             checkpoint.waypoint[2]
//         );

//         kv.SetFloat(
//             "yaw",
//             checkpoint.angles[1]
//         );

//         kv.SetFloat(
//             "plane_x",
//             checkpoint.planeCenter[0]
//         );

//         kv.SetFloat(
//             "plane_y",
//             checkpoint.planeCenter[1]
//         );

//         kv.SetFloat(
//             "plane_z",
//             checkpoint.planeCenter[2]
//         );

//         kv.SetFloat(
//             "forward_x",
//             checkpoint.forwardVec[0]
//         );

//         kv.SetFloat(
//             "forward_y",
//             checkpoint.forwardVec[1]
//         );

//         kv.SetFloat(
//             "forward_z",
//             checkpoint.forwardVec[2]
//         );

//         kv.SetFloat(
//             "side_x",
//             checkpoint.side[0]
//         );

//         kv.SetFloat(
//             "side_y",
//             checkpoint.side[1]
//         );

//         kv.SetFloat(
//             "side_z",
//             checkpoint.side[2]
//         );

//         kv.GoBack();
//     }

//     // Make sure the directory exists.
//     char directory[PLATFORM_MAX_PATH];

//     BuildPath(
//         Path_SM,
//         directory,
//         sizeof(directory),
//         "configs/RocketJumpAI"
//     );

//     if (!DirExists(directory))
//     {
//         CreateDirectory(
//             directory,
//             0
//         );
//     }

//     if (!KeyValuesToFile(kv, path))
//     {
//         PrintToServer(
//             "[Checkpoint] Failed to save: %s",
//             path
//         );

//         ReplyToCommand(
//             client,
//             "[Checkpoint] Failed to save checkpoint data."
//         );

//         delete kv;

//         return;
//     }

//     delete kv;

//     PrintToServer(
//         "[Checkpoint] Saved %d checkpoints to %s",
//         g_Checkpoints.Length,
//         path
//     );

//     ReplyToCommand(
//         client,
//         "[Checkpoint] Saved %d checkpoints.",
//         g_Checkpoints.Length
//     );
// }


// ============================================================
// Order checkpoints
// ============================================================

void OrderCheckpoints(int client, int args)
{
    if (args < 2)
    {
        ReplyToCommand(
            client,
            "[Checkpoint] Usage: sm_checkpoint order <index> <index> ..."
        );

        return;
    }

    if (g_Checkpoints == null || g_Checkpoints.Length == 0)
    {
        ReplyToCommand(
            client,
            "[Checkpoint] No checkpoints."
        );

        return;
    }

    // ========================================================
    // Two indexes = swap them
    // ========================================================

    if (args == 2)
    {
        char buffer1[16];
        char buffer2[16];

        GetCmdArg(
            2,
            buffer1,
            sizeof(buffer1)
        );

        GetCmdArg(
            3,
            buffer2,
            sizeof(buffer2)
        );

        int index1 = StringToInt(buffer1);
        int index2 = StringToInt(buffer2);

        if (index1 < 0 ||
            index1 >= g_Checkpoints.Length ||
            index2 < 0 ||
            index2 >= g_Checkpoints.Length)
        {
            ReplyToCommand(
                client,
                "[Checkpoint] Invalid checkpoint index."
            );

            return;
        }

        CheckpointData first;
        CheckpointData second;

        g_Checkpoints.GetArray(
            index1,
            first
        );

        g_Checkpoints.GetArray(
            index2,
            second
        );

        g_Checkpoints.SetArray(
            index1,
            second
        );

        g_Checkpoints.SetArray(
            index2,
            first
        );

        PrintToServer(
            "[Checkpoint] Swapped %d and %d.",
            index1,
            index2
        );

        PrintCheckpoints(client);

        return;
    }

    // ========================================================
    // More than two indexes
    //
    // order 5 2 8
    //
    // becomes:
    //
    // [5] [2] [8] [everything else in original order]
    // ========================================================

    bool selected[2048];
    int requested[2048];

    int requestedCount = 0;

    for (int arg = 2; arg <= args; arg++)
    {
        char buffer[16];

        GetCmdArg(
            arg,
            buffer,
            sizeof(buffer)
        );

        int index = StringToInt(buffer);

        if (index < 0 ||
            index >= g_Checkpoints.Length)
        {
            ReplyToCommand(
                client,
                "[Checkpoint] Invalid checkpoint index: %d",
                index
            );

            return;
        }

        if (selected[index])
        {
            ReplyToCommand(
                client,
                "[Checkpoint] Duplicate index: %d",
                index
            );

            return;
        }

        selected[index] = true;

        requested[requestedCount] = index;
        requestedCount++;
    }

    ArrayList original =
        new ArrayList(sizeof(CheckpointData));

    for (int i = 0; i < g_Checkpoints.Length; i++)
    {
        CheckpointData checkpoint;

        g_Checkpoints.GetArray(
            i,
            checkpoint
        );

        original.PushArray(
            checkpoint
        );
    }

    int outputIndex = 0;

    // First put requested indexes in requested order.
    for (int i = 0; i < requestedCount; i++)
    {
        CheckpointData checkpoint;

        original.GetArray(
            requested[i],
            checkpoint
        );

        g_Checkpoints.SetArray(
            outputIndex,
            checkpoint
        );

        outputIndex++;
    }

    // Then append everything that wasn't selected.
    for (int i = 0; i < original.Length; i++)
    {
        if (selected[i])
        {
            continue;
        }

        CheckpointData checkpoint;

        original.GetArray(
            i,
            checkpoint
        );

        g_Checkpoints.SetArray(
            outputIndex,
            checkpoint
        );

        outputIndex++;
    }

    delete original;

    PrintToServer(
        "[Checkpoint] Reordered %d checkpoints.",
        requestedCount
    );

    PrintCheckpoints(client);
}


// ============================================================
// Drawing
// ============================================================

void DrawCheckpointPlanes()
{
    if (g_Checkpoints == null ||
        g_Checkpoints.Length == 0)
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

        for (int i = 0; i < g_Checkpoints.Length; i++)
        {
            CheckpointData checkpoint;

            g_Checkpoints.GetArray(
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