# ==============================================================================
# Infinity CMake File
# ==============================================================================

# program objects --------------------------------------------------------------
SET(KILL_ALL_SRC
        KillAll.cpp
        KillAllOptions.cpp
        )

# final build ------------------------------------------------------------------
ADD_EXECUTABLE(pkillall ${KILL_ALL_SRC})

TARGET_LINK_LIBRARIES(pkillall ${ABS_LIBS})

INSTALL(TARGETS
            pkillall
        RUNTIME DESTINATION
            bin
        )

