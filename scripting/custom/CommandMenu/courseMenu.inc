#if defined _command_courseMenu_included
    #endinput
#endif
#define _command_courseMenu_included

void ShowCourseMenu(int client)
{
    Menu menu = new Menu(MenuHandler_Course);

    menu.SetTitle("Set active course");

    for (int i = 0; i < g_CourseNames.Length; i++)
    {
        char courseName[128];

        g_CourseNames.GetString(
            i,
            courseName,
            sizeof(courseName)
        );

        char info[16];

        IntToString(
            i,
            info,
            sizeof(info)
        );

        menu.AddItem(
            info,
            courseName
        );
    }

    menu.ExitBackButton = true;

    menu.Display(
        client,
        MENU_TIME_FOREVER
    );
}

public int MenuHandler_Course(
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

    int courseIndex = StringToInt(info);

    FakeClientCommand(
        client,
        "sm_courseRuntime setCourse %d",
        courseIndex
    );

    ShowMainMenu(client);

    return 0;
}


