#if defined _recorder_input_included
    #endinput
#endif
#define _recorder_input_included

public Action OnPlayerRunCmd(int client, int &buttons, int &impulse, float vel[3], float angles[3], int &weapon, int &subtype, int &cmdnum, int &tickcount, int &seed, int mouse[2])
{
    if (client > 0 && client <= MaxClients)
    {
        g_PlayerButtons[client] = buttons;
    }

    return Plugin_Continue;
}

void GatherPlayerInput(int client, PlayerInput input)
{
    int buttons = g_PlayerButtons[client];

    input.pressedForward = (buttons & IN_FORWARD) != 0;
    input.pressedBackward = (buttons & IN_BACK) != 0;
    input.pressedLeft = (buttons & IN_MOVELEFT) != 0;
    input.pressedRight = (buttons & IN_MOVERIGHT) != 0;
    input.pressedJump = (buttons & IN_JUMP) != 0;
    input.pressedDuck = (buttons & IN_DUCK) != 0;
    input.pressedAttack = (buttons & IN_ATTACK) != 0;
}
