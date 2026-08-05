#include <sourcemod>

public Plugin myinfo = 
{
    name = "Console filter",
    author = "Server Admin",
    description = "filter",
    version = "1.0"
};

public void OnPluginStart()
{
    AddServerConsoleHook(OnServerConsolePrint);
}

public Action OnServerConsolePrint(const char[] message, int length)
{
    if (StrContains(message, "Unhandled animation event") != -1 ||
        StrContains(message, "Updating physics on object in hierarchy") != -1 ||
        StrContains(message, "has an invalid spotlight width") != -1 ||
        StrContains(message, "Could not find lighting origin entity") != -1)
    {
        return Plugin_Handled;
    }
    
    return Plugin_Continue;
}