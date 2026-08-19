#if defined _courseruntime_events_included
    #endinput
#endif
#define _courseruntime_events_included

void FireCourseDataUpdated()
{
    if (g_OnCourseDataUpdated == null)
    {
        return;
    }

    Call_StartForward(
        g_OnCourseDataUpdated
    );

    Call_Finish();
}

void FireCheckpointEntered()
{
    if (g_OnCheckpointEntered == null)
    {
        return;
    }

    Call_StartForward(
        g_OnCheckpointEntered
    );

    Call_Finish();
}

void FireCourseComplete()
{
    if (g_OnCourseComplete == null)
    {
        return;
    }

    Call_StartForward(
        g_OnCourseComplete
    );

    Call_Finish();
}