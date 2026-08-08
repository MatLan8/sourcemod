#if defined _command_mainMenu_included
    #endinput
#endif
#define _command_mainMenu_included


void ShowMainMenu(int client)
{
    Menu menu = new Menu(MenuHandler_Main);

    menu.SetTitle(
        "Main Menu"
    );

    menu.AddItem(
        "noclip",
        "Noclip"
    );

    menu.AddItem(
        "debug",
        "Debug"
    );

    menu.AddItem(
        "checkpoints",
        "Checkpoints"
    );

    menu.AddItem(
        "triggers",
        "Triggers"
    );

    menu.Display(
        client,
        MENU_TIME_FOREVER
    );
}


// ============================================================
// Main menu handler
// ============================================================

public int MenuHandler_Main(
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

    if (action != MenuAction_Select)
    {
        return 0;
    }

    char info[32];

    menu.GetItem(
        item,
        info,
        sizeof(info)
    );


    if (StrEqual(info, "noclip"))
    {
        ToggleNoclip(client);
        ShowMainMenu(client);
    }

    if (StrEqual(info, "debug"))
    {
        ShowDebugMenu(client);
        return 0;
    }

    if (StrEqual(info, "checkpoints"))
    {
        ShowCourseMenu(client);
        return 0;
    }

    // if (StrEqual(info, "triggers"))
    // {
    //     ShowTriggerMenu(client);
    //     return 0;
    // }

    return 0;
}


void ToggleNoclip(int client)
{
    MoveType moveType = GetEntityMoveType(client);

    if (moveType == MOVETYPE_NOCLIP)
    {
        SetEntityMoveType(
            client,
            MOVETYPE_WALK
        );
    }
    else
    {
        SetEntityMoveType(
            client,
            MOVETYPE_NOCLIP
        );
    }
}
