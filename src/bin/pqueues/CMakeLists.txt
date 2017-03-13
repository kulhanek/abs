# ==============================================================================
# Infinity CMake File
# ==============================================================================

# program objects --------------------------------------------------------------
SET(QUEUES_SRC
        Queues.cpp
        QueuesOptions.cpp
        )

# final build ------------------------------------------------------------------
ADD_EXECUTABLE(pqueues ${QUEUES_SRC})
ADD_DEPENDENCIES(pqueues abs_shared)

TARGET_LINK_LIBRARIES(pqueues ${ABS_LIBS})

INSTALL(TARGETS
            pqueues
        RUNTIME DESTINATION
            bin
        )

