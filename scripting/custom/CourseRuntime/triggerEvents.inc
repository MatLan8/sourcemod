#if defined _courseruntime_triggerEvents_included
    #endinput
#endif
#define _courseruntime_triggerEvents_included

#include <sdkhooks>

public void OnTriggerStartTouch(
    int entity,
    int other
)
{
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

    if (g_CurrentTrigger.entity < 0)
    {
        return;
    }


    if (entity != g_CurrentTrigger.entity)
    {
        return;
    }

    // --------------------------------------------------------
    // Current trigger entered.
    // --------------------------------------------------------

    PrintToChat(
        other,
        "[Trigger] ENTER: %s",
        g_CurrentTrigger.name
    );

    PrintToServer(
        "[Trigger] Client %d ENTERED \"%s\" (entity %d)",
        other,
        g_CurrentTrigger.name,
        entity
    );

    if (AdvanceCheckpoint(other))
    {
        FireCheckpointEntered();
    }
}