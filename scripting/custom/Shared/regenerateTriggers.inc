#if defined _shared_regenerateTriggers_included
    #endinput
#endif
#define _shared_regenerateTriggers_included

#include <sdkhooks>

ArrayList g_RegenTriggers;

void CacheRegenerateTriggers()
{
    UnhookRegenerateTriggers();
    ResetAllRegenTouchCounts();

    if (g_RegenTriggers == null)
    {
        g_RegenTriggers = new ArrayList();
    }

    int entity = -1;
    int count = 0;

    while ((entity = FindEntityByClassname(
        entity,
        "func_regenerate"
    )) != -1)
    {
        SDKHook(
            entity,
            SDKHook_StartTouch,
            OnRegenerateStartTouch
        );

        SDKHook(
            entity,
            SDKHook_EndTouch,
            OnRegenerateEndTouch
        );

        g_RegenTriggers.Push(entity);
        count++;
    }

    PrintToServer(
        "[Regenerate] Cached %d func_regenerate triggers",
        count
    );
}

void UnhookRegenerateTriggers()
{
    if (g_RegenTriggers == null)
    {
        return;
    }

    for (int i = 0; i < g_RegenTriggers.Length; i++)
    {
        int entity = g_RegenTriggers.Get(i);

        if (!IsValidEntity(entity))
        {
            continue;
        }

        SDKUnhook(
            entity,
            SDKHook_StartTouch,
            OnRegenerateStartTouch
        );

        SDKUnhook(
            entity,
            SDKHook_EndTouch,
            OnRegenerateEndTouch
        );
    }

    g_RegenTriggers.Clear();
}

void ResetAllRegenTouchCounts()
{
    for (int i = 1; i <= MaxClients; i++)
    {
        g_RegenTouchCount[i] = 0;
    }
}

static bool IsRegenTouchClient(int other)
{
    if (other < 1 || other > MaxClients)
    {
        return false;
    }

    return IsClientInGame(other);
}

public void OnRegenerateStartTouch(
    int entity,
    int other
)
{
    if (!IsRegenTouchClient(other))
    {
        return;
    }

    g_RegenTouchCount[other]++;

    PrintToChat(
        other,
        "[Regenerate] ENTER: regenerate=%d",
        GetPlayerRegenerate(other)
    );
}

public void OnRegenerateEndTouch(
    int entity,
    int other
)
{
    if (!IsRegenTouchClient(other))
    {
        return;
    }

    if (g_RegenTouchCount[other] > 0)
    {
        g_RegenTouchCount[other]--;
    }

    PrintToChat(
        other,
        "[Regenerate] EXIT: regenerate=%d",
        GetPlayerRegenerate(other)
    );
}
