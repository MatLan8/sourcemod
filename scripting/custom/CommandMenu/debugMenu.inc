#if defined _command_debugMenu_included
    #endinput
#endif
#define _command_debugMenu_included

void ShowDebugMenu(int client)
{
    Menu menu = new Menu(MenuHandler_Debug);

    menu.SetTitle("Debug");

    menu.AddItem(
        "draw_checkpoints",
        "Draw Checkpoints"
    );

    menu.AddItem(
        "draw_triggers",
        "Draw Triggers"
    );

    menu.ExitBackButton = true;

    menu.Display(
        client,
        MENU_TIME_FOREVER
    );
}

public int MenuHandler_Debug(
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

    if (StrEqual(info, "draw_checkpoints"))
    {
        FakeClientCommand(client, "sm_debug drawCheckpoints");
        ShowDebugMenu(client);
    }
    else if (StrEqual(info, "draw_triggers"))
    {
        FakeClientCommand(client, "sm_debug drawTriggers");
        ShowDebugMenu(client);
    }


    return 0;
}