#if defined _command_checkpointMenu_included
    #endinput
#endif
#define _command_checkpointMenu_included

ArrayList g_Checkpoints[MAXPLAYERS + 1];
int g_MenuCourseIndex[MAXPLAYERS + 1];

void ShowCheckpointMenu(int client, int courseIndex)
{
    g_Checkpoints[client] = CourseRuntime_GetCourseCheckpoints(courseIndex);
    g_MenuCourseIndex[client] = courseIndex;
    if (g_Checkpoints[client] == null)
    {
        return;
    }

    Menu menu = new Menu(MenuHandler_Checkpoint);

    menu.SetTitle(
        "Select Checkpoint"
    );

    for (int i = 0; i < g_Checkpoints[client].Length; i++)
    {
        CheckpointData checkpoint;

        g_Checkpoints[client].GetArray(
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

public int MenuHandler_Checkpoint(
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
        ShowCheckpointCourseMenu(client);
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

    CheckpointData checkpoint;

    g_Checkpoints[client].GetArray(
        checkpointIndex,
        checkpoint
    );

    TeleportEntity(
        client,
        checkpoint.coordinates,
        NULL_VECTOR,
        NULL_VECTOR
    );
    
    ShowCheckpointMenu(
        client,
        g_MenuCourseIndex[client]
    );
    return 0;
}

