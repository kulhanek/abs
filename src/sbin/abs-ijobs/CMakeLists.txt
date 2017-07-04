## =============================================================================
# ABS CMake File
## -----------------------------------------------------------------------------

# program objects --------------------------------------------------------------
SET(IJOBS_SRC
        IJobs.cpp
        IJobsOptions.cpp
        )

# final build ------------------------------------------------------------------
ADD_EXECUTABLE(abs-ijobs ${IJOBS_SRC})
ADD_DEPENDENCIES(abs-ijobs abs_shared)

TARGET_LINK_LIBRARIES(abs-ijobs ${ABS_LIBS})

INSTALL(TARGETS
            abs-ijobs
        RUNTIME DESTINATION
            sbin
        )

