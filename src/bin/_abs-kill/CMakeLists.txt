# ==============================================================================
# ABS CMake File
# ==============================================================================

# program objects --------------------------------------------------------------
SET(KILL_SRC
        Kill.cpp
        KillOptions.cpp
        )

# final build ------------------------------------------------------------------
ADD_EXECUTABLE(_abs-kill ${KILL_SRC})

TARGET_LINK_LIBRARIES(_abs-kill ${ABS_LIBS})

INSTALL(TARGETS
            _abs-kill
        RUNTIME DESTINATION
            bin
        )

