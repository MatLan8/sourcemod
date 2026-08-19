#if defined _command_recorderMenu_included
    #endinput
#endif
#define _command_recorderMenu_included

void ShowRecorderMenu(int client)
{
    Menu menu = new Menu(MenuHandler_Recorder);

    menu.SetTitle("Recorder menu");

    menu.AddItem(
        "set_recorder_type",
        "Set recorder type"
    );

    menu.AddItem(
        "start_recording",
        "Start recording"
    );

    menu.AddItem(
        "stop_recording",
        "Stop recording"
    );

    menu.AddItem(
        "rerecord_last_clip",
        "Rerecord last clip"
    );

    menu.ExitBackButton = true;

    menu.Display(
        client,
        MENU_TIME_FOREVER
    );
}

public int MenuHandler_Recorder(
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

    if (StrEqual(info, "set_recorder_type"))
    {
        ShowRecorderTypesMenu(client);
        return 0;
    }

    if (StrEqual(info, "start_recording")) {

        FakeClientCommand(
            client,
            "sm_record"
        );

        ShowRecorderMenu(client);
        return 0;
    }

    if (StrEqual(info, "stop_recording")) {

        FakeClientCommand(
            client,
            "sm_stoprecord"
        );

        ShowRecorderMenu(client);
        return 0;
    }

    if (StrEqual(info, "rerecord_last_clip")) {

        FakeClientCommand(
            client,
            "sm_rerecord"
        );

        ShowRecorderMenu(client);
        return 0;
    }

    return 0;
}


void ShowRecorderTypesMenu(int client)
{
    Menu menu = new Menu(MenuHandler_RecorderTypes);

    menu.SetTitle("Select recorder type");

    menu.AddItem(
        "basic",
        "Basic"
    );

    menu.AddItem(
        "rocketJump",
        "Rocket jump"
    );

    menu.AddItem(
        "wallJump",
        "Wall jump"
    );

    menu.AddItem(
        "pogo",
        "Pogo"
    );

    menu.AddItem(
        "wallpogo",
        "Wallpogo"
    );

    menu.AddItem(
        "doubleSync",
        "Double sync"
    );

    menu.AddItem(
        "tripleSync",
        "Triple sync"
    );

    menu.AddItem(
        "jurf",
        "Jurf"
    );

    menu.AddItem(
        "speedshot",
        "Speedshot"
    );

    menu.AddItem(
        "advancedSync",
        "Advanced sync"
    );

    menu.AddItem(
        "combined",
        "Combined"
    );

    menu.ExitBackButton = true;

    menu.Display(
        client,
        MENU_TIME_FOREVER
    );
}

public int MenuHandler_RecorderTypes(
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
        ShowRecorderMenu(client);
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

    FakeClientCommand(
        client,
        "sm_setrecordingtype %s",
        info
    );

    ShowRecorderMenu(client);

    return 0;
}


