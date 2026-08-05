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
    GenerateCaptures(client);
}

void GenerateCaptures(int client)
{
    int ent = -1;
    int count = 0;

    while ((ent = FindEntityByClassname(ent, "trigger_capture_area")) != -1)
    {
        CheckpointData checkpoint;

        // Clear the entire structure first.
        checkpoint.entity = ent;
        checkpoint.name[0] = '\0';

        for (int i = 0; i < 3; i++)
        {
            checkpoint.waypoint[i] = 0.0;
            checkpoint.angles[i] = 0.0;
            checkpoint.forwardVec[i] = 0.0;
            checkpoint.side[i] = 0.0;
            checkpoint.planeCenter[i] = 0.0;
        }

        // ----------------------------------------------------
        // Get control point name.
        // ----------------------------------------------------

        GetEntPropString(
            ent,
            Prop_Data,
            "m_iszCapPointName",
            checkpoint.name,
            sizeof(checkpoint.name)
        );

        // ----------------------------------------------------
        // Capture area origin.
        // ----------------------------------------------------

        GetEntPropVector(
            ent,
            Prop_Data,
            "m_vecOrigin",
            checkpoint.waypoint
        );

        // ----------------------------------------------------
        // Add to the end of the checkpoint array.
        // ----------------------------------------------------

        g_Checkpoints.PushArray(checkpoint);

        count++;

        PrintToServer(
            "[Capture %d] Entity %d | Name: \"%s\" | Pos: %.1f %.1f %.1f",
            count,
            ent,
            checkpoint.name,
            checkpoint.waypoint[0],
            checkpoint.waypoint[1],
            checkpoint.waypoint[2]
        );
    }

    PrintToServer(
        "[Checkpoint] Added %d capture areas.",
        count
    );

    if (client > 0 && IsClientInGame(client))
    {
        PrintToChat(
            client,
            "\x04[Checkpoint]\x01 Added %d capture areas.",
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