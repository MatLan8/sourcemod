#include <sourcemod>
#include <sdktools>

#include "Shared/courseRuntime.inc"
#include "CommandMenu/globals.inc"
#include "CommandMenu/mainMenu.inc"
#include "CommandMenu/debugMenu.inc"
#include "CommandMenu/checkpointCourseMenu.inc"
#include "CommandMenu/checkpointMenu.inc"
#include "CommandMenu/triggerMenu.inc"
#include "CommandMenu/courseRuntimeMenu.inc"

public Plugin myinfo =
{
    name = "Debug Menu",
    author = "MatLan8",
    description = "Basic in-game debug menu",
    version = "1.0"
};

public void OnPluginStart()
{
    RegConsoleCmd(
        "sm_menu",
        Command_Menu
    );
}

public Action Command_Menu(
    int client,
    int args
)
{
    if (client <= 0 || !IsClientInGame(client))
    {
        return Plugin_Handled;
    }

    ShowMainMenu(client);

    return Plugin_Handled;
}

public void CourseRuntime_OnCourseDataUpdated()
{
    if (g_CourseNames != null)
    {
        if (IsValidHandle(g_CourseNames))
        {
            delete g_CourseNames;
        }

        g_CourseNames = null;
    }
    g_AllCoursesCheckpoints = CourseRuntime_GetAllCoursesCheckpoints()
    g_CourseNames = CourseRuntime_GetCourseNames();
}