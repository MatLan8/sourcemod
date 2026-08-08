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
        g_CurrentCheckpointIndex = 0;

        checkpointCourse.checkpoints.GetArray(
            g_CurrentCheckpointIndex,
            g_CurrentCheckpoint
        );
    }
    else
    {
        g_CurrentCheckpointIndex = -1;

        g_CurrentCheckpoint.name[0] = '\0';

        g_CurrentCheckpoint.coordinates[0] = 0.0;
        g_CurrentCheckpoint.coordinates[1] = 0.0;
        g_CurrentCheckpoint.coordinates[2] = 0.0;
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

    PrintToServer(
        "[CourseRuntime] Set current course to %d \"%s\". Checkpoint: %d \"%s\". Trigger: %d \"%s\".",
        g_CurrentCourseIndex,
        checkpointCourse.courseName,
        g_CurrentCheckpointIndex,
        g_CurrentCheckpoint.name,
        g_CurrentTriggerIndex,
        g_CurrentTrigger.name
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

    FireCourseDataUpdated();


    return true;
}
