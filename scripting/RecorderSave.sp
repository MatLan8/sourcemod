#include <sourcemod>
#include <sdktools>
#include <sdkhooks>


public Plugin myinfo =
{
    name = "Rocket Jump Recorder",
    author = "YourName",
    description = "Records TF2 movement data for AI training",
    version = "1.0"
};


bool g_Recording = false;


Handle g_PositionFile = INVALID_HANDLE;
Handle g_VelocityFile = INVALID_HANDLE;
Handle g_AnglesFile = INVALID_HANDLE;
Handle g_MovementFile = INVALID_HANDLE;
Handle g_StateFile = INVALID_HANDLE;
Handle g_RocketFile = INVALID_HANDLE;


bool g_RocketFired[MAXPLAYERS + 1];


enum struct RocketData
{
    int entity;
    int id;
    int owner;

    int spawnTick;

    bool exploded;

    int explosionTick;

    float explosionPos[3];
}



RocketData g_Rockets[2048];

int g_RocketCount = 0;
int g_NextRocketID = 1;

public void OnPluginStart()
{
    RegConsoleCmd("sm_record", Command_StartRecording);
    RegConsoleCmd("sm_stoprecord", Command_StopRecording);


    HookEvent(
        "player_spawn",
        Event_PlayerSpawn
    );


    PrintToServer("[Recorder] Loaded");
}


public Action Command_StartRecording(int client, int args)
{
    if(g_Recording)
    {
        ReplyToCommand(client, "[Recorder] Already recording!");
        return Plugin_Handled;
    }


    g_PositionFile = OpenFile("addons/sourcemod/data/position.csv", "w");
    g_VelocityFile = OpenFile("addons/sourcemod/data/velocity.csv", "w");
    g_AnglesFile = OpenFile("addons/sourcemod/data/angles.csv", "w");
    g_MovementFile = OpenFile("addons/sourcemod/data/movement.csv", "w");
    g_StateFile = OpenFile("addons/sourcemod/data/state.csv", "w");
    g_RocketFile = OpenFile("addons/sourcemod/data/rocket.csv", "w");



    if(
        g_PositionFile == INVALID_HANDLE ||
        g_VelocityFile == INVALID_HANDLE ||
        g_AnglesFile == INVALID_HANDLE ||
        g_MovementFile == INVALID_HANDLE ||
        g_StateFile == INVALID_HANDLE ||
        g_RocketFile == INVALID_HANDLE
    )
    {
        ReplyToCommand(client, "[Recorder] Failed opening files");
        return Plugin_Handled;
    }



    WriteFileLine(g_PositionFile,
        "tick,x,y,z");

    WriteFileLine(g_VelocityFile,
        "tick,vx,vy,vz");

    WriteFileLine(g_AnglesFile,
        "tick,pitch,yaw");

    WriteFileLine(g_MovementFile,
        "tick,forward,left,jump,duck,attack");

    WriteFileLine(g_StateFile,
        "tick,grounded,water,ammo,rocket_fired");

    WriteFileLine(g_RocketFile,
        "tick,rocket_id,spawn_tick,owner,x,y,z,vx,vy,vz,exploded,explosion_tick,explosion_x,explosion_y,explosion_z");



    g_Recording = true;


    ReplyToCommand(client,
        "[Recorder] Recording started!");

    return Plugin_Handled;
}


public Action Command_StopRecording(int client, int args)
{
    if(!g_Recording)
    {
        ReplyToCommand(client, "[Recorder] Not recording!");
        return Plugin_Handled;
    }


    g_Recording = false;


    CloseFiles();


    ReplyToCommand(client,
        "[Recorder] Recording stopped!");


    return Plugin_Handled;
}


void CloseFiles()
{
    if(g_PositionFile != INVALID_HANDLE)
    {
        CloseHandle(g_PositionFile);
        g_PositionFile = INVALID_HANDLE;
    }


    if(g_VelocityFile != INVALID_HANDLE)
    {
        CloseHandle(g_VelocityFile);
        g_VelocityFile = INVALID_HANDLE;
    }


    if(g_AnglesFile != INVALID_HANDLE)
    {
        CloseHandle(g_AnglesFile);
        g_AnglesFile = INVALID_HANDLE;
    }


    if(g_MovementFile != INVALID_HANDLE)
    {
        CloseHandle(g_MovementFile);
        g_MovementFile = INVALID_HANDLE;
    }


    if(g_StateFile != INVALID_HANDLE)
    {
        CloseHandle(g_StateFile);
        g_StateFile = INVALID_HANDLE;
    }


    if(g_RocketFile != INVALID_HANDLE)
    {
        CloseHandle(g_RocketFile);
        g_RocketFile = INVALID_HANDLE;
    }
}


public void OnPluginEnd()
{
    CloseFiles();
}


public void OnGameFrame()
{
    if(!g_Recording)
        return;

    int client = GetRecordingPlayer();

    if(client == 0)
        return;

    float pos[3];
    float vel[3];
    float ang[3];

    GetClientAbsOrigin(client,pos);

    GetEntPropVector(
        client,
        Prop_Data,
        "m_vecVelocity",
        vel
    );

    GetClientEyeAngles(
        client,
        ang
    );

    int buttons = GetClientButtons(client);

    bool isForward = (buttons & IN_FORWARD) != 0;
    bool isLeft = (buttons & IN_MOVELEFT) != 0;
    bool isJump = (buttons & IN_JUMP) != 0;
    bool isDuck = (buttons & IN_DUCK) != 0;
    bool isAttack = (buttons & IN_ATTACK) != 0;


    int grounded = IsPlayerGrounded(client);
    int water = GetEntProp(
        client,
        Prop_Send,
        "m_nWaterLevel"
    );
    int ammo = 0;

    int weapon = GetEntPropEnt(
        client,
        Prop_Send,
        "m_hActiveWeapon"
    );


    if(weapon != -1)
    {
        ammo = GetEntProp(
            weapon,
            Prop_Send,
            "m_iClip1"
        );
    }
    


    int tick = GetGameTickCount();

    WriteFileLine(
        g_PositionFile,
        "%d,%.3f,%.3f,%.3f",
        tick,
        pos[0],
        pos[1],
        pos[2]
    );

    WriteFileLine(
        g_VelocityFile,
        "%d,%.3f,%.3f,%.3f",
        tick,
        vel[0],
        vel[1],
        vel[2]
    );

    WriteFileLine(
        g_AnglesFile,
        "%d,%.3f,%.3f",
        tick,
        ang[0],
        ang[1]
    );

    WriteFileLine(
        g_MovementFile,
        "%d,%d,%d,%d,%d,%d",
        tick,
        isForward,
        isLeft,
        isJump,
        isDuck,
        isAttack
    );

    WriteFileLine(
        g_StateFile,
        "%d,%d,%d,%d,%d",
        tick,
        grounded,
        water,
        ammo,
        g_RocketFired[client]
    );

    FlushFile(g_PositionFile);
    FlushFile(g_VelocityFile);
    FlushFile(g_AnglesFile);
    FlushFile(g_MovementFile);
    FlushFile(g_StateFile);

    g_RocketFired[client] = false;
    RecordRockets();
}


int GetRecordingPlayer()
{
    for(int i = 1; i <= MaxClients; i++)
    {
        if(IsClientInGame(i) && IsPlayerAlive(i))
        {
            return i;
        }
    }

    return 0;
}


bool IsPlayerGrounded(int client)
{
    return (GetEntityFlags(client) & FL_ONGROUND) != 0;
}


public void Event_PlayerSpawn(
    Event event,
    const char[] name,
    bool dontBroadcast
)
{
    int client = GetClientOfUserId(
        event.GetInt("userid")
    );


    if(client > 0)
    {
        g_RocketFired[client] = false;
    }
}


public void OnEntityCreated(int entity, const char[] classname)
{
    if(!g_Recording)
        return;


    if(!StrEqual(classname, "tf_projectile_rocket"))
        return;


    SDKHook(
        entity,
        SDKHook_SpawnPost,
        RocketSpawned
    );
}

public void RocketSpawned(int entity)
{
    if(!IsValidEntity(entity))
        return;


    if(g_RocketCount >= 2048)
        return;


    int owner = GetEntPropEnt(
        entity,
        Prop_Send,
        "m_hOwnerEntity"
    );


    // mark player shot this tick
    if(owner > 0 && owner <= MaxClients)
    {
        if(IsClientInGame(owner))
        {
            g_RocketFired[owner] = true;
        }
    }



    // hook collision to detect explosion position
    SDKHook(
        entity,
        SDKHook_StartTouch,
        RocketTouch
    );



    // store rocket data
    g_Rockets[g_RocketCount].entity =
        EntIndexToEntRef(entity);


    g_Rockets[g_RocketCount].id =
        g_NextRocketID++;


    g_Rockets[g_RocketCount].owner =
        owner;


    g_Rockets[g_RocketCount].spawnTick =
        GetGameTickCount();


    g_Rockets[g_RocketCount].exploded = false;


    g_Rockets[g_RocketCount].explosionTick = 0;


    g_Rockets[g_RocketCount].explosionPos[0] = 0.0;
    g_Rockets[g_RocketCount].explosionPos[1] = 0.0;
    g_Rockets[g_RocketCount].explosionPos[2] = 0.0;


    g_RocketCount++;
}

public Action RocketTouch(
    int rocket,
    int other
)
{
    for(int i = 0; i < g_RocketCount; i++)
    {
        int entity = EntRefToEntIndex(
            g_Rockets[i].entity
        );


        if(entity == rocket)
        {
            GetEntPropVector(
                rocket,
                Prop_Send,
                "m_vecOrigin",
                g_Rockets[i].explosionPos
            );


            g_Rockets[i].exploded = true;

            g_Rockets[i].explosionTick =
                GetGameTickCount();

            break;
        }
    }


    return Plugin_Continue;
}

public Action Timer_ProcessRocket(
    Handle timer,
    int ref
)
{
    int entity = EntRefToEntIndex(ref);


    if(entity == INVALID_ENT_REFERENCE)
        return Plugin_Stop;



    if(g_RocketFile == INVALID_HANDLE)
        return Plugin_Stop;


    int owner = GetEntPropEnt(
        entity,
        Prop_Send,
        "m_hOwnerEntity"
    );


    if(owner <= 0 || owner > MaxClients)
        return Plugin_Stop;


    if(!IsClientInGame(owner))
        return Plugin_Stop;

    float pos[3];
    float vel[3];

    GetEntPropVector(
        entity,
        Prop_Send,
        "m_vecOrigin",
        pos
    );

    GetEntPropVector(
        entity,
        Prop_Data,
        "m_vecVelocity",
        vel
    );

    WriteFileLine(
        g_RocketFile,
        "%d,%.2f,%.2f,%.2f,%.2f,%.2f,%.2f,%d",
        GetGameTickCount(),

        pos[0],
        pos[1],
        pos[2],

        vel[0],
        vel[1],
        vel[2],

        owner
    );

    FlushFile(g_RocketFile);

    g_RocketFired[owner] = true;

    return Plugin_Stop;
}

void RecordRockets()
{
    if(g_RocketFile == INVALID_HANDLE)
        return;


    int tick = GetGameTickCount();


    for(int i = 0; i < g_RocketCount; i++)
    {
        int entity = EntRefToEntIndex(
            g_Rockets[i].entity
        );


        // rocket disappeared
        if(entity == INVALID_ENT_REFERENCE)
        {
            if(!g_Rockets[i].exploded)
            {
                g_Rockets[i].exploded = true;
                g_Rockets[i].explosionTick = tick;
            }


            WriteExplosionFrame(i);

            RemoveRocket(i);
            i--;

            continue;
        }



        float pos[3];
        float vel[3];


        GetEntPropVector(
            entity,
            Prop_Send,
            "m_vecOrigin",
            pos
        );


        GetEntPropVector(
            entity,
            Prop_Data,
            "m_vecVelocity",
            vel
        );



        WriteFileLine(
            g_RocketFile,

            "%d,%d,%d,%d,%.3f,%.3f,%.3f,%.3f,%.3f,%.3f,%d,%d,%.3f,%.3f,%.3f",

            tick,

            g_Rockets[i].id,

            g_Rockets[i].spawnTick,

            g_Rockets[i].owner,


            pos[0],
            pos[1],
            pos[2],


            vel[0],
            vel[1],
            vel[2],


            0,

            0,

            0.0,
            0.0,
            0.0
        );
    }


    FlushFile(g_RocketFile);
}

void RemoveRocket(int index)
{
    for(int i = index; i < g_RocketCount - 1; i++)
    {
        g_Rockets[i] = g_Rockets[i+1];
    }

    g_RocketCount--;
}

void WriteExplosionFrame(int index)
{
    WriteFileLine(
        g_RocketFile,

        "%d,%d,%d,%d,0,0,0,0,0,0,%d,%d,%.3f,%.3f,%.3f",

        GetGameTickCount(),

        g_Rockets[index].id,

        g_Rockets[index].spawnTick,

        g_Rockets[index].owner,


        1,

        g_Rockets[index].explosionTick,


        g_Rockets[index].explosionPos[0],

        g_Rockets[index].explosionPos[1],

        g_Rockets[index].explosionPos[2]
    );
}