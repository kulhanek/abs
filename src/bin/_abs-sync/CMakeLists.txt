# ==============================================================================
# ABS CMake File
# ==============================================================================

# program objects --------------------------------------------------------------
SET(SYNC_SRC
        Sync.cpp
        SyncOptions.cpp
        )

# final build ------------------------------------------------------------------
ADD_EXECUTABLE(_abs-sync ${SYNC_SRC})
ADD_DEPENDENCIES(_abs-sync abs_shared)

TARGET_LINK_LIBRARIES(_abs-sync ${ABS_LIBS})

INSTALL(TARGETS
            _abs-sync
        RUNTIME DESTINATION
            bin
        )

