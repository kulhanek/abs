# ==============================================================================
# Infinity CMake File
# ==============================================================================

# program objects --------------------------------------------------------------
SET(NODES_SRC
        Nodes.cpp
        NodesOptions.cpp
        )

# final build ------------------------------------------------------------------
ADD_EXECUTABLE(pnodes ${NODES_SRC})

TARGET_LINK_LIBRARIES(pnodes ${ABS_LIBS})

INSTALL(TARGETS
            pnodes
        RUNTIME DESTINATION
            bin
        )

