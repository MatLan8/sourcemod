#if defined _courseruntime_course_included
    #endinput
#endif

#define _courseruntime_course_included

bool SetCourse(
    int client = 0,
    const char[] courseName = "",
    int courseIndex = -1
)
{
    bool hasCourseName =
        strlen(courseName) > 0;

    bool hasCourseIndex =
        courseIndex >= 0;


    if (
        hasCourseName &&
        hasCourseIndex
    )
    {
        PrintToServer(
            "[CourseRuntime] ERROR: SetCourse cannot use both course name \"%s\" and course index %d.",
            courseName,
            courseIndex
        );

        if (client > 0 && IsClientInGame(client))
        {
            PrintToChat(
                client,
                "[CourseRuntime] Error: provide either a course name or course index, not both."
            );
        }

        return false;
    }

    if (
        g_AllCoursesCheckpoints == null ||
        g_AllCoursesCheckpoints.Length == 0
    )
    {
        PrintToServer(
            "[CourseRuntime] ERROR: No courses are loaded."
        );

        if (client > 0 && IsClientInGame(client))
        {
            PrintToChat(
                client,
                "[CourseRuntime] Error: no courses are loaded."
            );
        }

        return false;
    }

    int selectedCourseIndex = -1;

    if (hasCourseName)
    {
        for (
            int i = 0;
            i < g_AllCoursesCheckpoints.Length;
            i++
        )
        {
            CourseCheckpoints course;

            g_AllCoursesCheckpoints.GetArray(
                i,
                course
            );

            if (
                StrEqual(
                    course.courseName,
                    courseName,
                    false
                )
            )
            {
                selectedCourseIndex = i;
                break;
            }
        }


        if (selectedCourseIndex == -1)
        {
            PrintToServer(
                "[CourseRuntime] ERROR: Course \"%s\" was not found.",
                courseName
            );

            if (client > 0 && IsClientInGame(client))
            {
                PrintToChat(
                    client,
                    "[CourseRuntime] Error: course \"%s\" was not found.",
                    courseName
                );
            }

            return false;
        }
    }

    else if (hasCourseIndex)
    {
        if (
            courseIndex < 0 ||
            courseIndex >= g_AllCoursesCheckpoints.Length
        )
        {
            PrintToServer(
                "[CourseRuntime] ERROR: Course index %d is out of range. Available courses: %d.",
                courseIndex,
                g_AllCoursesCheckpoints.Length
            );

            if (client > 0 && IsClientInGame(client))
            {
                PrintToChat(
                    client,
                    "[CourseRuntime] Error: course index %d is invalid.",
                    courseIndex
                );
            }

            return false;
        }

        selectedCourseIndex = courseIndex;
    }
    else
    {
        selectedCourseIndex = 0;
    }

    if (
        g_AllCoursesTriggers == null ||
        selectedCourseIndex >= g_AllCoursesTriggers.Length
    )
    {
        PrintToServer(
            "[CourseRuntime] ERROR: Trigger course data does not contain course index %d.",
            selectedCourseIndex
        );

        if (client > 0 && IsClientInGame(client))
        {
            PrintToChat(
                client,
                "[CourseRuntime] Error: trigger data is inconsistent."
            );
        }

        return false;
    }


    // ========================================================
    // Set current course.
    // ========================================================

    g_CurrentCourseIndex =
        selectedCourseIndex;


    // ========================================================
    // Reset current checkpoint.
    // ========================================================

    CourseCheckpoints checkpointCourse;

    g_AllCoursesCheckpoints.GetArray(
        g_CurrentCourseIndex,
        checkpointCourse
    );


    if (
        checkpointCourse.checkpoints != null &&
        checkpointCourse.checkpoints.Length > 0
    )
    {
        g_CurrentCheckpointIndex = 1;

        checkpointCourse.checkpoints.GetArray(
            g_CurrentCheckpointIndex,
            g_CurrentCheckpoint
        );

        CheckpointData firstCheckpoint;
        checkpointCourse.checkpoints.GetArray(
            0,
            firstCheckpoint
        );

        TeleportEntity(
            client,
            firstCheckpoint.coordinates,
            NULL_VECTOR,
            NULL_VECTOR
        );
    }
    else
    {
        g_CurrentCheckpointIndex = -1;

        g_CurrentCheckpoint.name[0] = '\0';

        g_CurrentCheckpoint.coordinates[0] = -1.0;
        g_CurrentCheckpoint.coordinates[1] = -1.0;
        g_CurrentCheckpoint.coordinates[2] = -1.0;
    }


    // ========================================================
    // Reset current trigger.
    // ========================================================

    CourseTriggers triggerCourse;

    g_AllCoursesTriggers.GetArray(
        g_CurrentCourseIndex,
        triggerCourse
    );


    if (
        triggerCourse.triggers != null &&
        triggerCourse.triggers.Length > 0
    )
    {
        g_CurrentTriggerIndex = 0;

        triggerCourse.triggers.GetArray(
            g_CurrentTriggerIndex,
            g_CurrentTrigger
        );
    }
    else
    {
        g_CurrentTriggerIndex = -1;

        g_CurrentTrigger.name[0] = '\0';

        g_CurrentTrigger.worldMins[0] = 0.0;
        g_CurrentTrigger.worldMins[1] = 0.0;
        g_CurrentTrigger.worldMins[2] = 0.0;

        g_CurrentTrigger.worldMaxs[0] = 0.0;
        g_CurrentTrigger.worldMaxs[1] = 0.0;
        g_CurrentTrigger.worldMaxs[2] = 0.0;

        g_CurrentTrigger.entity = -1;
    }

    // ========================================================
    // Reset current capture trigger.
    // ========================================================

    g_CurrentCaptureTrigger.entity = -1;
    g_CurrentCaptureTrigger.name[0] = '\0';

    if (
        g_CaptureTriggers != null &&
        g_CurrentCourseIndex >= 0 &&
        g_CurrentCourseIndex < g_CaptureTriggers.Length
    )
    {
        g_CaptureTriggers.GetArray(
            g_CurrentCourseIndex,
            g_CurrentCaptureTrigger
        );
    }

    PrintToServer(
        "[CourseRuntime] Set current course to %d \"%s\". Checkpoint: %d \"%s\". Trigger: %d \"%s\". Capture trigger: \"%s\". ",
        g_CurrentCourseIndex,
        checkpointCourse.courseName,
        g_CurrentCheckpointIndex,
        g_CurrentCheckpoint.name,
        g_CurrentTriggerIndex,
        g_CurrentTrigger.name,
        g_CurrentCaptureTrigger.name
    );


    if (client > 0 && IsClientInGame(client))
    {
        PrintToChat(
            client,
            "[CourseRuntime] Course set to %d \"%s\".",
            g_CurrentCourseIndex,
            checkpointCourse.courseName
        );
    }


    // ========================================================
    // Notify consumers.
    // ========================================================

    return true;
}

bool AdvanceCheckpoint(
    int client = 0,
    const char[] checkpointName = "",
    int checkpointIndex = -1
)
{
    if (!ValidateAdvanceCheckpointArguments(client, checkpointName, checkpointIndex))
    {
        return false;
    }

    if (!ValidateCourseData(client))
    {
        return false;
    }

    CourseCheckpoints checkpointCourse;
    CourseTriggers triggerCourse;

    g_AllCoursesCheckpoints.GetArray(
        g_CurrentCourseIndex,
        checkpointCourse
    );

    g_AllCoursesTriggers.GetArray(
        g_CurrentCourseIndex,
        triggerCourse
    );


    if (!ValidateCurrentCourseData(client, checkpointCourse, triggerCourse))
    {
        return false;
    }


    int targetCheckpointIndex = GetTargetCheckpointIndex(client, checkpointCourse, checkpointName, checkpointIndex);
    if (targetCheckpointIndex < 0)
    {
        return false;
    }
    int targetTriggerIndex = targetCheckpointIndex - 1;

    // ========================================================
    // Get target checkpoint.
    // ========================================================

    CheckpointData targetCheckpoint;

    checkpointCourse.checkpoints.GetArray(
        targetCheckpointIndex,
        targetCheckpoint
    );

    // ========================================================
    // Commit checkpoint.
    // ========================================================

    g_CurrentCheckpointIndex =
        targetCheckpointIndex;

    g_CurrentCheckpoint =
        targetCheckpoint;


    bool isFinalTrigger =
            targetTriggerIndex >=
            triggerCourse.triggers.Length;
    

    if (isFinalTrigger)
    {
        g_CurrentTriggerIndex = -1;

        g_CurrentTrigger.name[0] = '\0';

        g_CurrentTrigger.worldMins[0] = 0.0;
        g_CurrentTrigger.worldMins[1] = 0.0;
        g_CurrentTrigger.worldMins[2] = 0.0;

        g_CurrentTrigger.worldMaxs[0] = 0.0;
        g_CurrentTrigger.worldMaxs[1] = 0.0;
        g_CurrentTrigger.worldMaxs[2] = 0.0;

        g_CurrentTrigger.entity = -1;
    }
    else
    {
        triggerCourse.triggers.GetArray(
            targetTriggerIndex,
            g_CurrentTrigger
        );

        g_CurrentTriggerIndex = targetTriggerIndex;
    }


    // ========================================================
    // Debug output.
    // ========================================================

    PrintToServer(
        "[CourseRuntime] Advanced course %d \"%s\". Checkpoint: %d \"%s\". Trigger: %d \"%s\".",
        g_CurrentCourseIndex,
        checkpointCourse.courseName,
        g_CurrentCheckpointIndex,
        g_CurrentCheckpoint.name,
        g_CurrentTriggerIndex,
        g_CurrentTrigger.name
    );


    if (
        client > 0 &&
        IsClientInGame(client)
    )
    {
        PrintToChat(
            client,
            "[CourseRuntime] Advanced to checkpoint %d \"%s\".",
            g_CurrentCheckpointIndex,
            g_CurrentCheckpoint.name
        );
    }

    return true;

}


bool ValidateAdvanceCheckpointArguments(
    int client,
    const char[] checkpointName,
    int checkpointIndex
)
{
    bool hasCheckpointName =
        strlen(checkpointName) > 0;

    bool hasCheckpointIndex =
        checkpointIndex >= 0;

    if (
        hasCheckpointName &&
        hasCheckpointIndex
    )
    {
        PrintToServer(
            "[CourseRuntime] ERROR: AdvanceCheckpoint cannot use both checkpoint name \"%s\" and checkpoint index %d.",
            checkpointName,
            checkpointIndex
        );

        if (
            client > 0 &&
            IsClientInGame(client)
        )
        {
            PrintToChat(
                client,
                "[CourseRuntime] Error: provide either a checkpoint name or checkpoint index, not both."
            );
        }

        return false;
    }

    return true;
}


bool ValidateCourseData(
    int client
)
{
    // ----------------------------------------------------
    // Checkpoints.
    // ----------------------------------------------------

    if (
        g_AllCoursesCheckpoints == null ||
        g_AllCoursesCheckpoints.Length == 0
    )
    {
        PrintToServer(
            "[CourseRuntime] ERROR: No checkpoint courses are loaded."
        );

        if (
            client > 0 &&
            IsClientInGame(client)
        )
        {
            PrintToChat(
                client,
                "[CourseRuntime] Error: no checkpoint courses are loaded."
            );
        }

        return false;
    }

    // ----------------------------------------------------
    // Triggers.
    // ----------------------------------------------------

    if (
        g_AllCoursesTriggers == null ||
        g_AllCoursesTriggers.Length == 0
    )
    {
        PrintToServer(
            "[CourseRuntime] ERROR: No trigger courses are loaded."
        );

        if (
            client > 0 &&
            IsClientInGame(client)
        )
        {
            PrintToChat(
                client,
                "[CourseRuntime] Error: no trigger courses are loaded."
            );
        }

        return false;
    }

    if (
        g_CurrentCourseIndex < 0 ||
        g_CurrentCourseIndex >= g_AllCoursesCheckpoints.Length
    )
    {
        PrintToServer(
            "[CourseRuntime] ERROR: Current course index %d is invalid.",
            g_CurrentCourseIndex
        );

        if (
            client > 0 &&
            IsClientInGame(client)
        )
        {
            PrintToChat(
                client,
                "[CourseRuntime] Error: current course is invalid."
            );
        }

        return false;
    }

    return true;
}


bool ValidateCurrentCourseData(
    int client,
    CourseCheckpoints checkpointCourse,
    CourseTriggers triggerCourse
)
{
    // ----------------------------------------------------
    // Validate checkpoint list.
    // ----------------------------------------------------

    if (
        checkpointCourse.checkpoints == null ||
        checkpointCourse.checkpoints.Length == 0
    )
    {
        PrintToServer(
            "[CourseRuntime] ERROR: Course \"%s\" has no checkpoints.",
            checkpointCourse.courseName
        );

        if (
            client > 0 &&
            IsClientInGame(client)
        )
        {
            PrintToChat(
                client,
                "[CourseRuntime] Error: course \"%s\" has no checkpoints.",
                checkpointCourse.courseName
            );
        }

        return false;
    }

    // ----------------------------------------------------
    // Validate trigger list.
    // ----------------------------------------------------

    if (
        triggerCourse.triggers == null ||
        triggerCourse.triggers.Length == 0
    )
    {
        PrintToServer(
            "[CourseRuntime] ERROR: Course \"%s\" has no triggers.",
            checkpointCourse.courseName
        );

        if (
            client > 0 &&
            IsClientInGame(client)
        )
        {
            PrintToChat(
                client,
                "[CourseRuntime] Error: course \"%s\" has no triggers.",
                checkpointCourse.courseName
            );
        }

        return false;
    }

    return true;
}



int GetTargetCheckpointIndex(
    int client,
    CourseCheckpoints checkpointCourse,
    const char[] checkpointName,
    int checkpointIndex
)
{
    bool hasCheckpointName =
        strlen(checkpointName) > 0;

    bool hasCheckpointIndex =
        checkpointIndex >= 0;

    // ====================================================
    // Checkpoint name.
    // ====================================================

    if (hasCheckpointName)
    {
        for (
            int i = 0;
            i < checkpointCourse.checkpoints.Length;
            i++
        )
        {
            CheckpointData checkpoint;

            checkpointCourse.checkpoints.GetArray(
                i,
                checkpoint
            );

            if (
                StrEqual(
                    checkpoint.name,
                    checkpointName,
                    false
                )
            )
            {
                return i;
            }
        }

        PrintToServer(
            "[CourseRuntime] ERROR: Checkpoint \"%s\" was not found in course \"%s\".",
            checkpointName,
            checkpointCourse.courseName
        );

        if (
            client > 0 &&
            IsClientInGame(client)
        )
        {
            PrintToChat(
                client,
                "[CourseRuntime] Error: checkpoint \"%s\" was not found.",
                checkpointName
            );
        }

        return -1;
    }

    // ====================================================
    // Checkpoint index.
    // ====================================================

    if (hasCheckpointIndex)
    {
        if (
            checkpointIndex < 0 ||
            checkpointIndex >= checkpointCourse.checkpoints.Length
        )
        {
            PrintToServer(
                "[CourseRuntime] ERROR: Checkpoint index %d is out of range. Available checkpoints: %d.",
                checkpointIndex,
                checkpointCourse.checkpoints.Length
            );

            if (
                client > 0 &&
                IsClientInGame(client)
            )
            {
                PrintToChat(
                    client,
                    "[CourseRuntime] Error: checkpoint index %d is invalid.",
                    checkpointIndex
                );
            }

            return -1;
        }

        return checkpointIndex;
    }

    // ====================================================
    // No checkpoint specified.
    //
    // Advance to the next checkpoint.
    // ====================================================

    int targetCheckpointIndex;

    if (g_CurrentCheckpointIndex < 0)
    {
        targetCheckpointIndex = 0;
    }
    else
    {
        targetCheckpointIndex =
            g_CurrentCheckpointIndex + 1;
    }

    // ----------------------------------------------------
    // Make sure there actually is a next checkpoint.
    // ----------------------------------------------------

    if (
        targetCheckpointIndex >=
        checkpointCourse.checkpoints.Length
    )
    {
        PrintToServer(
            "[CourseRuntime] ERROR: No next checkpoint exists after index %d in course \"%s\".",
            g_CurrentCheckpointIndex,
            checkpointCourse.courseName
        );

        if (
            client > 0 &&
            IsClientInGame(client)
        )
        {
            PrintToChat(
                client,
                "[CourseRuntime] Error: you are already at the final checkpoint."
            );
        }

        return -1;
    }

    return targetCheckpointIndex;
}