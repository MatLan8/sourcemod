#include <sourcemod>

public Plugin myinfo =
{
    name = "Hello TF2",
    author = "YourName",
    description = "First test plugin",
    version = "1.0"
};


public void OnPluginStart()
{
    PrintToServer("=================================");
    PrintToServer("Hello! My TF2 plugin is running!");
    PrintToServer("=================================");
}

public void OnClientPutInServer(int client)
{
    PrintToServer("=================================");
    PrintToServer("Hello! My TF2 plugin is running!");
    PrintToServer("=================================");
	
	PrintToChatAll("=================================");
    PrintToChatAll("Hello! My TF2 plugin is running!");
    PrintToChatAll("=================================");
}
