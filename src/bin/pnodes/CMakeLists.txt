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
ADD_DEPENDENCIES(pnodes abs_shared)

TARGET_LINK_LIBRARIES(pnodes ${ABS_LIBS})

INSTALL(TARGETS
            pnodes
        RUNTIME DESTINATION
            bin
        )

