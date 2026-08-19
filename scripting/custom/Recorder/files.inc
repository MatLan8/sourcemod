#if defined _recorder_files_included
    #endinput
#endif
#define _recorder_files_included

#define RECORDINGS_ROOT "data/recordings"
#define RECORDINGS_OTHER "other"
#define RECORDING_NUMBER_MAX 999

public Action Command_SetRecordingType(int client, int args)
{
    if (args < 1)
    {
        if (g_RecordingType[0] == '\0')
        {
            ReplyToCommand(client, "[Recorder] Recording type is not set. Usage: sm_setrecordingtype <type>");
        }
        else
        {
            ReplyToCommand(client, "[Recorder] Recording type: %s", g_RecordingType);
        }

        return Plugin_Handled;
    }

    char recordingType[64];
    GetCmdArg(1, recordingType, sizeof(recordingType));

    if (!IsSafeRecordingName(recordingType))
    {
        ReplyToCommand(client, "[Recorder] Type must be letters, digits, '_' or '-'.");
        return Plugin_Handled;
    }

    strcopy(g_RecordingType, sizeof(g_RecordingType), recordingType);
    ReplyToCommand(client, "[Recorder] Recording type set to \"%s\".", g_RecordingType);
    return Plugin_Handled;
}

public Action Command_StartRecording(int client, int args)
{
    return StartRecording(client, args, false);
}

public Action Command_RerecordLastClip(int client, int args)
{
    if (args != 0)
    {
        ReplyToCommand(client, "[Recorder] Usage: sm_rerecord");
        return Plugin_Handled;
    }

    return StartRecording(client, 0, true);
}

Action StartRecording(int client, int args, bool rerecordLast)
{
    if (g_Recording)
    {
        ReplyToCommand(client, "[Recorder] Already recording!");
        return Plugin_Handled;
    }

    char csvPath[PLATFORM_MAX_PATH];
    char binPath[PLATFORM_MAX_PATH];

    if (rerecordLast)
    {
        if (g_LastRecordingCsvPath[0] == '\0' || g_LastMapDataPath[0] == '\0')
        {
            ReplyToCommand(client, "[Recorder] No previous recording to rerecord.");
            return Plugin_Handled;
        }

        strcopy(csvPath, sizeof(csvPath), g_LastRecordingCsvPath);
        strcopy(binPath, sizeof(binPath), g_LastMapDataPath);
    }
    else
    {
        char dir[PLATFORM_MAX_PATH];
        char fileStem[PLATFORM_MAX_PATH];

        if (args >= 1)
        {
            char recordName[128];
            GetCmdArg(1, recordName, sizeof(recordName));

            if (!IsSafeRecordingName(recordName))
            {
                ReplyToCommand(client, "[Recorder] Name must be letters, digits, '_' or '-'.");
                return Plugin_Handled;
            }

            if (!EnsureRecordingsFolder(client, RECORDINGS_OTHER, dir, sizeof(dir)))
            {
                return Plugin_Handled;
            }

            strcopy(fileStem, sizeof(fileStem), recordName);
        }
        else
        {
            if (g_RecordingType[0] == '\0')
            {
                ReplyToCommand(client, "[Recorder] No recording type set. Use sm_setrecordingtype <type> or sm_record <name>.");
                return Plugin_Handled;
            }

            if (!EnsureRecordingsFolder(client, g_RecordingType, dir, sizeof(dir)))
            {
                return Plugin_Handled;
            }

            char mapName[128];
            GetRecordingMapName(mapName, sizeof(mapName));

            int nextNumber = FindNextRecordingNumber(dir, mapName);

            if (nextNumber > RECORDING_NUMBER_MAX)
            {
                ReplyToCommand(client, "[Recorder] No free recording number left for this map (001-999).");
                return Plugin_Handled;
            }

            Format(fileStem, sizeof(fileStem), "%s__%03d", mapName, nextNumber);
        }

        Format(csvPath, sizeof(csvPath), "%s/%s.csv", dir, fileStem);
        Format(binPath, sizeof(binPath), "%s/%s.map.bin", dir, fileStem);
    }

    // Rerecorded and named takes replace existing files ("w" / "wb" truncate).
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

    strcopy(g_LastRecordingCsvPath, sizeof(g_LastRecordingCsvPath), csvPath);
    strcopy(g_LastMapDataPath, sizeof(g_LastMapDataPath), binPath);

    ResetRocketSlots();
    ResetRocketFiredEvents();
    g_CheckpointReached = false;
    g_CourseComplete = false;
    WriteRecordingHeader();
    g_RecordRowNumber = 0;

    g_Recording = true;

    ResetRecordingProfile();
    SyncRecorderCheckpoint();

    ReplyToCommand(
        client,
        rerecordLast
            ? "[Recorder] Rerecording: %s"
            : "[Recorder] Recording started: %s",
        csvPath
    );
    return Plugin_Handled;
}

bool IsSafeRecordingName(const char[] name)
{
    if (name[0] == '\0')
    {
        return false;
    }

    for (int i = 0; name[i] != '\0'; i++)
    {
        int ch = name[i];

        if (
            (ch >= 'a' && ch <= 'z') ||
            (ch >= 'A' && ch <= 'Z') ||
            (ch >= '0' && ch <= '9') ||
            ch == '_' ||
            ch == '-'
        )
        {
            continue;
        }

        return false;
    }

    return true;
}

void GetRecordingMapName(char[] mapName, int maxlength)
{
    GetCurrentMap(mapName, maxlength);

    int start = 0;

    for (int i = 0; mapName[i] != '\0'; i++)
    {
        if (mapName[i] == '/' || mapName[i] == '\\')
        {
            start = i + 1;
        }
    }

    if (start > 0)
    {
        char baseName[128];
        strcopy(baseName, sizeof(baseName), mapName[start]);
        strcopy(mapName, maxlength, baseName);
    }

    for (int i = 0; mapName[i] != '\0'; i++)
    {
        int ch = mapName[i];

        if (
            (ch >= 'a' && ch <= 'z') ||
            (ch >= 'A' && ch <= 'Z') ||
            (ch >= '0' && ch <= '9') ||
            ch == '_' ||
            ch == '-'
        )
        {
            continue;
        }

        mapName[i] = '_';
    }
}

bool EnsureRecordingsFolder(int client, const char[] typeName, char[] dir, int maxlength)
{
    char recordingsRoot[PLATFORM_MAX_PATH];

    BuildPath(Path_SM, recordingsRoot, sizeof(recordingsRoot), RECORDINGS_ROOT);

    if (!DirExists(recordingsRoot) && !CreateDirectory(recordingsRoot, 511))
    {
        ReplyToCommand(client, "[Recorder] Failed creating folder: %s", RECORDINGS_ROOT);
        return false;
    }

    BuildPath(Path_SM, dir, maxlength, "%s/%s", RECORDINGS_ROOT, typeName);

    if (!DirExists(dir) && !CreateDirectory(dir, 511))
    {
        ReplyToCommand(client, "[Recorder] Failed creating folder: %s/%s", RECORDINGS_ROOT, typeName);
        return false;
    }

    return true;
}

int ParseRecordingNumber(const char[] suffix)
{
    if (
        strlen(suffix) < 5 ||
        !IsCharNumeric(suffix[0]) ||
        !IsCharNumeric(suffix[1]) ||
        !IsCharNumeric(suffix[2]) ||
        suffix[3] != '.'
    )
    {
        return 0;
    }

    if (
        StrEqual(suffix[3], ".csv") ||
        StrEqual(suffix[3], ".map.bin")
    )
    {
        return StringToInt(suffix);
    }

    return 0;
}

int FindNextRecordingNumber(const char[] dir, const char[] mapName)
{
    DirectoryListing listing = OpenDirectory(dir);

    if (listing == null)
    {
        return 1;
    }

    char fileName[PLATFORM_MAX_PATH];
    char prefix[PLATFORM_MAX_PATH];
    FileType fileType;
    int maxNumber = 0;

    Format(prefix, sizeof(prefix), "%s__", mapName);
    int prefixLen = strlen(prefix);

    while (listing.GetNext(fileName, sizeof(fileName), fileType))
    {
        if (fileType != FileType_File)
        {
            continue;
        }

        if (strncmp(fileName, prefix, prefixLen, false) != 0)
        {
            continue;
        }

        int number = ParseRecordingNumber(fileName[prefixLen]);

        if (number > maxNumber)
        {
            maxNumber = number;
        }
    }

    delete listing;
    return maxNumber + 1;
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

// WriteFileLine formats through a 2048-byte buffer and truncates.
// Header (~4k) and busy rocket rows can exceed that; write the string directly.
void WriteCsvLine(const char[] line)
{
    WriteFileString(g_RecordingFile, line, false);
    WriteFileString(g_RecordingFile, "\n", false);
}

void WriteRecordingHeader()
{
    char header[8192];
    char rocketHeader[512];

    strcopy(header, sizeof(header), "row,tick,player_position_x,player_position_y,player_position_z,checkpoint_x,checkpoint_y,checkpoint_z,player_velocity_x,player_velocity_y,player_velocity_z,player_view_pitch,player_view_yaw,player_regenerate,player_grounded,player_water_level,player_ammo,input_forward,input_backward,input_left,input_right,input_jump,input_duck,input_attack,event_rocket_fired,event_checkpoint_reached,event_course_complete");

    for (int i = 0; i < MAX_ROCKET_SLOTS; i++)
    {
        Format(rocketHeader, sizeof(rocketHeader), ",rocket_%d_exists,rocket_%d_id,rocket_%d_spawn_tick,rocket_%d_position_x,rocket_%d_position_y,rocket_%d_position_z,rocket_%d_velocity_x,rocket_%d_velocity_y,rocket_%d_velocity_z,rocket_%d_spawn_pitch,rocket_%d_spawn_yaw,rocket_%d_exploded,rocket_%d_explosion_position_x,rocket_%d_explosion_position_y,rocket_%d_explosion_position_z", i, i, i, i, i, i, i, i, i, i, i, i, i, i, i);
        StrCat(header, sizeof(header), rocketHeader);
    }

    WriteCsvLine(header);
}

void WriteRecordingRow(int tick, PlayerSnapshot player, PlayerInput input, bool rocketFired, bool checkpointReached, bool courseComplete)
{
    char row[8192];
    char rocketRow[512];
    g_RecordRowNumber++;

    Format(row, sizeof(row), "%d,%d,%.3f,%.3f,%.3f,%.3f,%.3f,%.3f,%.3f,%.3f,%.3f,%.3f,%.3f,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d", g_RecordRowNumber, tick, player.position[0], player.position[1], player.position[2], g_CurrentCheckpoint.coordinates[0], g_CurrentCheckpoint.coordinates[1], g_CurrentCheckpoint.coordinates[2], player.velocity[0], player.velocity[1], player.velocity[2], player.viewAngles[0], player.viewAngles[1], player.regenerate, player.grounded, player.waterLevel, player.ammo, input.pressedForward, input.pressedBackward, input.pressedLeft, input.pressedRight, input.pressedJump, input.pressedDuck, input.pressedAttack, rocketFired, checkpointReached, courseComplete);

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

    WriteCsvLine(row);
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
