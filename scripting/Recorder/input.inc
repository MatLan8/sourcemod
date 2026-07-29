#if defined _recorder_input_included
    #endinput
#endif
#define _recorder_input_included

void RecordPlayerInput(int client, int tick)
{
    int buttons = GetClientButtons(client);

    bool isForward = (buttons & IN_FORWARD) != 0;
    bool isLeft = (buttons & IN_MOVELEFT) != 0;
    bool isJump = (buttons & IN_JUMP) != 0;
    bool isDuck = (buttons & IN_DUCK) != 0;
    bool isAttack = (buttons & IN_ATTACK) != 0;

    WriteFileLine(g_MovementFile, "%d,%d,%d,%d,%d,%d", tick, isForward, isLeft, isJump, isDuck, isAttack);
    FlushFile(g_MovementFile);
}
