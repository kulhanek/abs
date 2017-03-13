## =============================================================================
# ABS CMake File
## -----------------------------------------------------------------------------

# program objects --------------------------------------------------------------
SET(RESTORE_ENV_SRC
        EnvRestore.cpp
        EnvRestoreOptions.cpp
        )

# final build ------------------------------------------------------------------
ADD_EXECUTABLE(abs-restore-env ${RESTORE_ENV_SRC})

TARGET_LINK_LIBRARIES(abs-restore-env ${ABS_LIBS})

INSTALL(TARGETS
            abs-restore-env
        RUNTIME DESTINATION
            sbin
        )

