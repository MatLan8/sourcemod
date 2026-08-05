#if defined _trigger_core_included
    #endinput
#endif
#define _trigger_core_included


void SetPoint(int client)
{
    if (client <= 0 || !IsClientInGame(client))
    {
        return;
    }

    if (g_TriggerPointCount >= 2)
    {
        ReplyToCommand(
            client,
            "[Trigger] Two points already exist. Use sm_trigger reset first."
        );

        return;
    }

    float position[3];

    GetClientAbsOrigin(
        client,
        position
    );

    g_TriggerPoints[g_TriggerPointCount][0] = position[0];
    g_TriggerPoints[g_TriggerPointCount][1] = position[1];
    g_TriggerPoints[g_TriggerPointCount][2] = position[2];

    int point = g_TriggerPointCount;

    g_TriggerPointCount++;

    ReplyToCommand(
        client,
        "[Trigger] Point %d saved: %.3f %.3f %.3f",
        point + 1,
        position[0],
        position[1],
        position[2]
    );

    if (g_TriggerPointCount == 2)
    {
        ReplyToCommand(
            client,
            "[Trigger] Two points defined. You can now save or update the trigger."
        );
    }
}


void Reset(int client)
{

    g_TriggerPointCount = 0;

    for (int i = 0; i < 2; i++)
    {
        g_TriggerPoints[i][0] = 0.0;
        g_TriggerPoints[i][1] = 0.0;
        g_TriggerPoints[i][2] = 0.0;
    }

    ReplyToCommand(
            client,
            "[Trigger] Points reset."
    );
}

