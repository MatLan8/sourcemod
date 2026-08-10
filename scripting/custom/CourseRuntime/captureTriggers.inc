#if defined _courseruntime_captureTriggers_included
    #endinput
#endif
#define _courseruntime_captureTriggers_included

#include <sdkhooks>

void HandleCaptureTrigger(const char[] triggerName)
{
    int entity = FindCaptureTriggerByName(
        triggerName
    );

    if (entity == -1)
    {
        PrintToServer(
            "[CaptureTrigger] ERROR: Could not find \"%s\".",
            triggerName
        );

        return;
    }

    if (g_CaptureTriggers == null) {
        g_CaptureTriggers = new ArrayList(sizeof(CaptureTriggerData));
    }

    if (FindCaptureTriggerByEntity(entity) == -1)
    {
        SDKHook(
            entity,
            SDKHook_StartTouch,
            OnCaptureTriggerStartTouch
        );

        CaptureTriggerData captureTrigger;

        strcopy(
            captureTrigger.name,
            sizeof(captureTrigger.name),
            triggerName
        );

        captureTrigger.entity = entity;

        g_CaptureTriggers.PushArray(
            captureTrigger
        );

        PrintToServer(
            "[CaptureTrigger] Registered capture trigger \"%s\" (entity %d)",
            triggerName,
            entity
        );
    }
}

int FindCaptureTriggerByName(const char[] triggerName)
{
    int entity = -1;

    while ((entity = FindEntityByClassname(
        entity,
        "trigger_capture_area"
    )) != -1)
    {
        char name[128];

        GetEntPropString(
            entity,
            Prop_Data,
            "m_iszCapPointName",
            name,
            sizeof(name)
        );

        if (StrEqual(name, triggerName))
        {
            return entity;
        }
    }

    return -1;
}

int FindCaptureTriggerByEntity(int entity)
{
    if (g_CaptureTriggers == null)
    {
        return -1;
    }

    for (int i = 0; i < g_CaptureTriggers.Length; i++)
    {
        CaptureTriggerData trigger;

        g_CaptureTriggers.GetArray(
            i,
            trigger
        );

        if (trigger.entity == entity)
        {
            return i;
        }
    }

    return -1;
}


public Action OnCaptureTriggerStartTouch(
    int entity,
    int other
)
{
    PrintToServer(
        "[CAPTURE] Entered entity=%d other=%d",
        entity,
        other,
        g_CurrentTrigger.entity
    );

    if (entity != g_CurrentCaptureTrigger.entity)
    {
        return Plugin_Continue;
    }

    if (other <= 0 || other > MaxClients)
    {
        return Plugin_Continue;
    }

    if (!IsClientInGame(other))
    {
        return Plugin_Continue;
    }

    PrintToChat(
        other,
        "[CaptureTrigger] ENTER: %s",
        g_CurrentCaptureTrigger.name
    );

    return Plugin_Continue;
}