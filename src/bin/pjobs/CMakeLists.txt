# ==============================================================================
# Infinity CMake File
# ==============================================================================

# program objects --------------------------------------------------------------
SET(JOBS_SRC
        Jobs.cpp
        JobsOptions.cpp
        )

# final build ------------------------------------------------------------------
ADD_EXECUTABLE(pjobs ${JOBS_SRC})
ADD_DEPENDENCIES(pjobs abs_shared)

TARGET_LINK_LIBRARIES(pjobs ${ABS_LIBS})

INSTALL(TARGETS
            pjobs
        RUNTIME DESTINATION
            bin
        )

