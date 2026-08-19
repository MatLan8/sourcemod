#if defined _command_courseRuntimeMenu_included
    #endinput
#endif
#define _command_courseRuntimeMenu_included

void ShowCourseRuntimeMenu(int client)
{
    Menu menu = new Menu(MenuHandler_CourseRuntime);

    menu.SetTitle("Course Runtime");

    menu.AddItem(
        "set_active_course",
        "Set active course"
    );

    menu.AddItem(
        "select_checkpoint",
        "Select current checkpoint"
    );

    menu.ExitBackButton = true;

    menu.Display(
        client,
        MENU_TIME_FOREVER
    );
}

public int MenuHandler_CourseRuntime(
    Menu menu,
    MenuAction action,
    int client,
    int item
)
{
    if (action == MenuAction_End)
    {
        delete menu;
        return 0;
    }

    if (
        action == MenuAction_Cancel &&
        item == MenuCancel_ExitBack
    )
    {
        ShowMainMenu(client);
        return 0;
    }

    if (action != MenuAction_Select)
    {
        return 0;
    }

    char info[64];

    menu.GetItem(
        item,
        info,
        sizeof(info)
    );

    if (StrEqual(info, "set_active_course"))
    {
        ShowCourseMenu(client);
        return 0;
    }

    if (StrEqual(info, "select_checkpoint"))
    {
        ShowCurrentCheckpointMenu(client);
        return 0;
    }

    return 0;
}

void ShowCurrentCheckpointMenu(int client)
{
    if (g_CurrentCourseCheckpoints == null || g_CurrentCourseCheckpoints.Length < 2)
    {
        PrintToChat(
            client,
            "[CourseRuntime] No selectable checkpoints on the current course."
        );
        ShowCourseRuntimeMenu(client);
        return;
    }

    Menu menu = new Menu(MenuHandler_CurrentCheckpoint);

    menu.SetTitle("Select current checkpoint");

    for (int i = 1; i < g_CurrentCourseCheckpoints.Length; i++)
    {
        CheckpointData checkpoint;

        g_CurrentCourseCheckpoints.GetArray(
            i,
            checkpoint
        );

        char info[16];

        IntToString(
            i,
            info,
            sizeof(info)
        );

        menu.AddItem(
            info,
            checkpoint.name
        );
    }

    menu.ExitBackButton = true;

    menu.Display(
        client,
        MENU_TIME_FOREVER
    );
}

public int MenuHandler_CurrentCheckpoint(
    Menu menu,
    MenuAction action,
    int client,
    int item
)
{
    if (action == MenuAction_End)
    {
        delete menu;
        return 0;
    }

    if (
        action == MenuAction_Cancel &&
        item == MenuCancel_ExitBack
    )
    {
        ShowCourseRuntimeMenu(client);
        return 0;
    }

    if (action != MenuAction_Select)
    {
        return 0;
    }

    char info[16];

    menu.GetItem(
        item,
        info,
        sizeof(info)
    );

    int checkpointIndex = StringToInt(info);

    if (
        g_CurrentCourseCheckpoints == null ||
        checkpointIndex < 1 ||
        checkpointIndex >= g_CurrentCourseCheckpoints.Length
    )
    {
        ShowCourseRuntimeMenu(client);
        return 0;
    }

    FakeClientCommand(
        client,
        "sm_courseRuntime advanceCheckpoint %d",
        checkpointIndex
    );

    CheckpointData previousCheckpoint;

    g_CurrentCourseCheckpoints.GetArray(
        checkpointIndex - 1,
        previousCheckpoint
    );

    TeleportEntity(
        client,
        previousCheckpoint.coordinates,
        NULL_VECTOR,
        NULL_VECTOR
    );

    ShowCurrentCheckpointMenu(client);

    return 0;
}
