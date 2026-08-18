#if defined _mapdatadebug_raycasts_included
    #endinput
#endif

#define _mapdatadebug_raycasts_included

void FibonacciWorldDir(int index, float yawDegrees, float dir[3])
{
    float y = 1.0 - ((float(index) + 0.5) / float(RAY_COUNT_OMNI)) * 2.0;
    float radiusSq = 1.0 - y * y;
    float radius = (radiusSq > 0.0) ? SquareRoot(radiusSq) : 0.0;
    float theta = float(index) * FLOAT_PI * (3.0 - SquareRoot(5.0));

    float lx = Cosine(theta) * radius;
    float ly = Sine(theta) * radius;
    float lz = y;

    float yaw = DegToRad(yawDegrees);
    float cy = Cosine(yaw);
    float sy = Sine(yaw);

    dir[0] = sy * lx + cy * ly;
    dir[1] = -cy * lx + sy * ly;
    dir[2] = lz;
}

void CaptureRaySnapshot(
    const float origin[3],
    const float angles[3],
    const float eyeOrigin[3]
)
{
    float omniOrigin[3];
    omniOrigin[0] = origin[0];
    omniOrigin[1] = origin[1];
    omniOrigin[2] = origin[2] + RAY_OFFSET_Z;

    for (int i = 0; i < RAY_COUNT_OMNI; i++)
    {
        float dir[3];
        FibonacciWorldDir(i, angles[1], dir);

        g_RayStart[i][0] = omniOrigin[0];
        g_RayStart[i][1] = omniOrigin[1];
        g_RayStart[i][2] = omniOrigin[2];

        float length = g_Rays[i].geomDistance;

        g_RayEnd[i][0] = omniOrigin[0] + dir[0] * length;
        g_RayEnd[i][1] = omniOrigin[1] + dir[1] * length;
        g_RayEnd[i][2] = omniOrigin[2] + dir[2] * length;
    }

    float lookDir[3];
    GetAngleVectors(angles, lookDir, NULL_VECTOR, NULL_VECTOR);

    g_RayStart[RAY_LOOK_INDEX][0] = eyeOrigin[0];
    g_RayStart[RAY_LOOK_INDEX][1] = eyeOrigin[1];
    g_RayStart[RAY_LOOK_INDEX][2] = eyeOrigin[2];

    float lookLength = g_Rays[RAY_LOOK_INDEX].geomDistance;

    g_RayEnd[RAY_LOOK_INDEX][0] = eyeOrigin[0] + lookDir[0] * lookLength;
    g_RayEnd[RAY_LOOK_INDEX][1] = eyeOrigin[1] + lookDir[1] * lookLength;
    g_RayEnd[RAY_LOOK_INDEX][2] = eyeOrigin[2] + lookDir[2] * lookLength;
}

void PrintRayLine(int client, int index)
{
    char label[8];

    if (index == RAY_LOOK_INDEX)
    {
        strcopy(label, sizeof(label), "look");
    }
    else
    {
        IntToString(index, label, sizeof(label));
    }

    char line[256];
    Format(
        line,
        sizeof(line),
        "[MapDataDebug] ray %s flags=%d geom=%.1f n=(%.2f %.2f %.2f) ng=%.1f tp=%.1f",
        label,
        g_Rays[index].flags,
        g_Rays[index].geomDistance,
        g_Rays[index].geomNormal[0],
        g_Rays[index].geomNormal[1],
        g_Rays[index].geomNormal[2],
        g_Rays[index].noGrenadesDistance,
        g_Rays[index].teleportDistance
    );

    PrintToServer("%s", line);

    if (client > 0 && IsClientInGame(client))
    {
        PrintToChat(client, "%s", line);
        PrintToConsole(client, "%s", line);
    }
}

Action Command_RayTest(int client)
{
    int target = ResolveMapDataTarget(client);

    if (target == 0)
    {
        ReplyToCommand(client, "[MapDataDebug] No in-game player. Add a fake client or use this in-game.");
        return Plugin_Handled;
    }

    float origin[3];
    float angles[3];
    float eyeOrigin[3];

    GetClientAbsOrigin(target, origin);
    GetClientEyeAngles(target, angles);
    GetClientEyePosition(target, eyeOrigin);

    StartNativeTimer();

    bool success = MapData_TraceRays(
        origin,
        angles,
        eyeOrigin,
        g_Rays,
        sizeof(g_Rays) * sizeof(g_Rays[0])
    );

    float elapsedMs = StopNativeTimerMs();

    g_RaysReady = success;

    if (success)
    {
        CaptureRaySnapshot(origin, angles, eyeOrigin);
    }

    int geomHits = 0;
    int clipHits = 0;
    int waterHits = 0;
    int noGrenadeHits = 0;
    int teleportHits = 0;
    float minGeom = RAY_MAX_RANGE;

    for (int i = 0; i < RAY_COUNT; i++)
    {
        int flags = g_Rays[i].flags;

        if (flags & GEOM_SOLID)
        {
            geomHits++;
        }

        if (flags & GEOM_PLAYERCLIP)
        {
            clipHits++;
        }

        if (flags & GEOM_WATER)
        {
            waterHits++;
        }

        if (flags & GEOM_NOGRENADES)
        {
            noGrenadeHits++;
        }

        if (flags & GEOM_TELEPORT)
        {
            teleportHits++;
        }

        if (g_Rays[i].geomDistance < minGeom)
        {
            minGeom = g_Rays[i].geomDistance;
        }
    }

    PrintToServer(
        "[MapDataDebug] rays success=%d time=%.4fms geom=%d clip=%d water=%d ng=%d tp=%d minGeom=%.1f look flags=%d geom=%.1f ng=%.1f tp=%.1f",
        success,
        elapsedMs,
        geomHits,
        clipHits,
        waterHits,
        noGrenadeHits,
        teleportHits,
        minGeom,
        g_Rays[RAY_LOOK_INDEX].flags,
        g_Rays[RAY_LOOK_INDEX].geomDistance,
        g_Rays[RAY_LOOK_INDEX].noGrenadesDistance,
        g_Rays[RAY_LOOK_INDEX].teleportDistance
    );

    ReplyToCommand(
        client,
        "[MapDataDebug] rays %.4fms geom %d clip %d water %d ng %d tp %d min %.0f look g=%.0f ng=%.0f tp=%.0f",
        elapsedMs,
        geomHits,
        clipHits,
        waterHits,
        noGrenadeHits,
        teleportHits,
        minGeom,
        g_Rays[RAY_LOOK_INDEX].geomDistance,
        g_Rays[RAY_LOOK_INDEX].noGrenadesDistance,
        g_Rays[RAY_LOOK_INDEX].teleportDistance
    );

    return Plugin_Handled;
}

Action Command_PrintRays(int client)
{
    if (!g_RaysReady)
    {
        ReplyToCommand(client, "[MapDataDebug] No ray data. Use sm_mapdata rays first.");
        return Plugin_Handled;
    }

    for (int i = 0; i < RAY_COUNT; i++)
    {
        PrintRayLine(client, i);
    }

    return Plugin_Handled;
}
