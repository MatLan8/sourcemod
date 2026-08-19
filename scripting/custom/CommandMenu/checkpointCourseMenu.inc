#if defined _command_checkpointCourseMenu_included
    #endinput
#endif
#define _command_checkpointCourseMenu_included

void ShowCheckpointCourseMenu(int client)
{
    if (g_AllCoursesCheckpoints == null)
    {
        return;
    }

    Menu menu = new Menu(MenuHandler_CheckpointCourse);

    menu.SetTitle(
        "Select Course"
    );

    for (int i = 0; i < g_AllCoursesCheckpoints.Length; i++)
    {
        CourseCheckpoints course;

        g_AllCoursesCheckpoints.GetArray(
            i,
            course
        );

        char info[16];

        IntToString(
            i,
            info,
            sizeof(info)
        );

        menu.AddItem(
            info,
            course.courseName
        );
    }

    menu.ExitBackButton = true;

    menu.Display(
        client,
        MENU_TIME_FOREVER
    );
}

public int MenuHandler_CheckpointCourse(
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

    char info[16];

    menu.GetItem(
        item,
        info,
        sizeof(info)
    );

    int courseIndex = StringToInt(info);

    ShowCheckpointMenu(
        client,
        courseIndex
    );

    return 0;
}