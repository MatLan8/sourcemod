#if defined _recorder_files_included
    #endinput
#endif
#define _recorder_files_included

public Action Command_StartRecording(int client, int args)
{
    char recordName[128];
    if (g_Recording)
    {
        ReplyToCommand(client, "[Recorder] Already recording!");
        return Plugin_Handled;
    }
    if (args < 1)
    {
        ReplyToCommand(client, "[Recorder] Usage: sm_record <name>");
        return Plugin_Handled;
    }
    GetCmdArg(
        1,
        recordName,
        sizeof(recordName)
    );

    char dir[PLATFORM_MAX_PATH];
    char csvPath[PLATFORM_MAX_PATH];
    char binPath[PLATFORM_MAX_PATH];

    BuildPath(
        Path_SM,
        dir,
        sizeof(dir),
        "data/%s",
        recordName
    );

    if (!DirExists(dir) && !CreateDirectory(dir))
    {
        ReplyToCommand(client, "[Recorder] Failed creating recording folder!");
        return Plugin_Handled;
    }

    BuildPath(
        Path_SM,
        csvPath,
        sizeof(csvPath),
        "data/%s/%s.csv",
        recordName,
        recordName
    );

    BuildPath(
        Path_SM,
        binPath,
        sizeof(binPath),
        "data/%s/%s.map.bin",
        recordName,
        recordName
    );

    g_RecordingFile = OpenFile(csvPath, "w");

    if (g_RecordingFile == INVALID_HANDLE)
    {
        ReplyToCommand(client, "[Recorder] Failed opening recording csv!");
        return Plugin_Handled;
    }

    g_MapDataFile = OpenFile(binPath, "wb");

    if (g_MapDataFile == INVALID_HANDLE)
    {
        CloseHandle(g_RecordingFile);
        g_RecordingFile = INVALID_HANDLE;
        ReplyToCommand(client, "[Recorder] Failed opening map binary!");
        return Plugin_Handled;
    }

    ResetRocketSlots();
    ResetRocketFiredEvents();
    WriteRecordingHeader();
    g_RecordRowNumber = 0;

    g_Recording = true;

    ResetRecordingProfile();
    SyncRecorderCheckpoint();

    ReplyToCommand(client, "[Recorder] Recording started!");
    return Plugin_Handled;
}

public Action Command_StopRecording(int client, int args)
{
    if (!g_Recording)
    {
        ReplyToCommand(client, "[Recorder] Not recording!");
        return Plugin_Handled;
    }

    g_Recording = false;
    CloseFiles();
    PrintRecordingProfile(client);

    ReplyToCommand(client, "[Recorder] Recording stopped!");
    return Plugin_Handled;
}

void CloseFiles()
{
    if (g_RecordingFile != INVALID_HANDLE)
    {
        FlushFile(g_RecordingFile);
        CloseHandle(g_RecordingFile);
        g_RecordingFile = INVALID_HANDLE;
    }

    if (g_MapDataFile != INVALID_HANDLE)
    {
        FlushFile(g_MapDataFile);
        CloseHandle(g_MapDataFile);
        g_MapDataFile = INVALID_HANDLE;
    }
}

void WriteRecordingHeader()
{
    char header[8192];
    char rocketHeader[512];

    strcopy(header, sizeof(header), "row,tick,player_position_x,player_position_y,player_position_z,checkpoint_x,checkpoint_y,checkpoint_z,player_velocity_x,player_velocity_y,player_velocity_z,player_view_pitch,player_view_yaw,player_regenerate,player_grounded,player_water_level,player_ammo,input_forward,input_backward,input_left,input_right,input_jump,input_duck,input_attack,event_rocket_fired");

    for (int i = 0; i < MAX_ROCKET_SLOTS; i++)
    {
        Format(rocketHeader, sizeof(rocketHeader), ",rocket_%d_exists,rocket_%d_id,rocket_%d_spawn_tick,rocket_%d_position_x,rocket_%d_position_y,rocket_%d_position_z,rocket_%d_velocity_x,rocket_%d_velocity_y,rocket_%d_velocity_z,rocket_%d_spawn_pitch,rocket_%d_spawn_yaw,rocket_%d_exploded,rocket_%d_explosion_position_x,rocket_%d_explosion_position_y,rocket_%d_explosion_position_z", i, i, i, i, i, i, i, i, i, i, i, i, i, i, i);
        StrCat(header, sizeof(header), rocketHeader);
    }

    WriteFileLine(g_RecordingFile, "%s", header);
}

void WriteRecordingRow(int tick, PlayerSnapshot player, PlayerInput input, bool rocketFired)
{
    char row[8192];
    char rocketRow[512];
    g_RecordRowNumber++;

    Format(row, sizeof(row), "%d,%d,%.3f,%.3f,%.3f,%.3f,%.3f,%.3f,%.3f,%.3f,%.3f,%.3f,%.3f,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d", g_RecordRowNumber, tick, player.position[0], player.position[1], player.position[2], g_CurrentCheckpoint.coordinates[0], g_CurrentCheckpoint.coordinates[1], g_CurrentCheckpoint.coordinates[2], player.velocity[0], player.velocity[1], player.velocity[2], player.viewAngles[0], player.viewAngles[1], player.regenerate, player.grounded, player.waterLevel, player.ammo, input.pressedForward, input.pressedBackward, input.pressedLeft, input.pressedRight, input.pressedJump, input.pressedDuck, input.pressedAttack, rocketFired);

    for (int i = 0; i < MAX_ROCKET_SLOTS; i++)
    {
        if (g_RocketSlots[i].exists)
        {
            Format(rocketRow, sizeof(rocketRow), ",%d,%d,%d,%.3f,%.3f,%.3f,%.3f,%.3f,%.3f,%.3f,%.3f,%d,%.3f,%.3f,%.3f", 1, g_RocketSlots[i].id, g_RocketSlots[i].spawnTick, g_RocketSlots[i].position[0], g_RocketSlots[i].position[1], g_RocketSlots[i].position[2], g_RocketSlots[i].velocity[0], g_RocketSlots[i].velocity[1], g_RocketSlots[i].velocity[2], g_RocketSlots[i].spawnPitch, g_RocketSlots[i].spawnYaw, g_RocketSlots[i].exploded, g_RocketSlots[i].explosionPosition[0], g_RocketSlots[i].explosionPosition[1], g_RocketSlots[i].explosionPosition[2]);
        }
        else
        {
            strcopy(rocketRow, sizeof(rocketRow), ",0,0,0,0,0,0,0,0,0,0,0,0,0,0,0");
        }

        StrCat(row, sizeof(row), rocketRow);
    }

    WriteFileLine(g_RecordingFile, "%s", row);
}

void WriteMapDataBinary(int tick)
{
    if (g_MapDataFile == INVALID_HANDLE)
    {
        return;
    }

    WriteFileCell(g_MapDataFile, tick, 4);
    WriteFile(g_MapDataFile, g_VoxelFlags, VOXEL_COUNT, 4);

    int rayCells[7];

    for (int i = 0; i < RAY_COUNT; i++)
    {
        rayCells[0] = g_Rays[i].flags;
        rayCells[1] = view_as<int>(g_Rays[i].geomDistance);
        rayCells[2] = view_as<int>(g_Rays[i].geomNormal[0]);
        rayCells[3] = view_as<int>(g_Rays[i].geomNormal[1]);
        rayCells[4] = view_as<int>(g_Rays[i].geomNormal[2]);
        rayCells[5] = view_as<int>(g_Rays[i].noGrenadesDistance);
        rayCells[6] = view_as<int>(g_Rays[i].teleportDistance);

        WriteFile(g_MapDataFile, rayCells, 7, 4);
    }
}
