#if defined _courseruntime_triggerTouch_included
    #endinput
#endif
#define _courseruntime_triggerTouch_included

#include <sdkhooks>



public void OnTriggerStartTouch(
    int entity,
    int other
)
{
    // --------------------------------------------------------
    // Only players.
    // --------------------------------------------------------

    if (other < 1 || other > MaxClients)
    {
        return;
    }

    if (!IsClientInGame(other))
    {
        return;
    }

    // --------------------------------------------------------
    // No current trigger.
    // --------------------------------------------------------

    if (g_CurrentTriggerIndex < 0 ||
        g_CurrentTriggerIndex >= g_Triggers.Length)
    {
        return;
    }

    // --------------------------------------------------------
    // Get current trigger.
    // --------------------------------------------------------

    TriggerData currentTrigger;

    g_Triggers.GetArray(
        g_CurrentTriggerIndex,
        currentTrigger
    );

    // --------------------------------------------------------
    // Ignore all triggers except current trigger.
    // --------------------------------------------------------

    if (entity != currentTrigger.entity)
    {
        return;
    }

    // --------------------------------------------------------
    // Current trigger entered.
    // --------------------------------------------------------

    PrintToChat(
        other,
        "[Trigger] ENTER: %s",
        currentTrigger.name
    );

    PrintToServer(
        "[Trigger] Client %d ENTERED \"%s\" (entity %d)",
        other,
        currentTrigger.name,
        entity
    );
}


// ============================================================
// End Touch
// ============================================================

public void OnTriggerEndTouch(
    int entity,
    int other
)
{
    // --------------------------------------------------------
    // Only players.
    // --------------------------------------------------------

    if (other < 1 || other > MaxClients)
    {
        return;
    }

    if (!IsClientInGame(other))
    {
        return;
    }

    // --------------------------------------------------------
    // No current trigger.
    // --------------------------------------------------------

    if (g_CurrentTriggerIndex < 0 ||
        g_CurrentTriggerIndex >= g_Triggers.Length)
    {
        return;
    }

    // --------------------------------------------------------
    // Get current trigger.
    // --------------------------------------------------------

    TriggerData currentTrigger;

    g_Triggers.GetArray(
        g_CurrentTriggerIndex,
        currentTrigger
    );

    // --------------------------------------------------------
    // Ignore all triggers except current trigger.
    // --------------------------------------------------------

    if (entity != currentTrigger.entity)
    {
        return;
    }

    // --------------------------------------------------------
    // Current trigger exited.
    // --------------------------------------------------------

    PrintToChat(
        other,
        "[Trigger] EXIT: %s",
        currentTrigger.name
    );

    PrintToServer(
        "[Trigger] Client %d EXITED \"%s\" (entity %d)",
        other,
        currentTrigger.name,
        entity
    );
}