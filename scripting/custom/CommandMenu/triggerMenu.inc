#if defined _command_triggerMenu_included
    #endinput
#endif
#define _command_triggerMenu_included

void ShowTriggerMenu(int client)
{
    Menu menu = new Menu(MenuHandler_Trigger);

    menu.SetTitle("Trigger");

    menu.AddItem(
        "set_point",
        "Set Point"
    );

    menu.AddItem(
        "reset",
        "Reset Points"
    );

    menu.AddItem(
        "save_next_trigger",
        "Save Next Trigger"
    );



    menu.ExitBackButton = true;

    menu.Display(
        client,
        MENU_TIME_FOREVER
    );
}

public int MenuHandler_Trigger(
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

    if (StrEqual(info, "set_point"))
    {
        FakeClientCommand(client, "sm_trigger setpoint");
    }
    else if (StrEqual(info, "reset"))
    {
        FakeClientCommand(client, "sm_trigger reset");
    }
    else if (StrEqual(info, "save_next_trigger"))
    {
        FakeClientCommand(client, "sm_trigger saveNextCourseTrigger");
    }

    ShowTriggerMenu(client);

    return 0;
}