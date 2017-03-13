## =============================================================================
# ABS CMake File
## -----------------------------------------------------------------------------

# program objects --------------------------------------------------------------
SET(SORT_NODES_SRC
        SortNodes.cpp
        SortNodesOptions.cpp
        )

# final build ------------------------------------------------------------------
ADD_EXECUTABLE(abs-sort-nodes ${SORT_NODES_SRC})

TARGET_LINK_LIBRARIES(abs-sort-nodes ${ABS_LIBS})

INSTALL(TARGETS
            abs-sort-nodes
        RUNTIME DESTINATION
            sbin
        )

