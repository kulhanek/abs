# ==============================================================================
# ABS CMake File
# ==============================================================================

INCLUDE_DIRECTORIES(. SYSTEM)
INCLUDE_DIRECTORIES(pbspro SYSTEM)
INCLUDE_DIRECTORIES(resources SYSTEM)

# objects in library -----------------------------------------------------------
SET(PBSPRO_SRC
        PBSProMainHeader.cpp
        PBSProModule.cpp
        PBSProServer.cpp
        PBSProAttr.cpp
        PBSProQueue.cpp
        PBSProNode.cpp
        PBSProJob.cpp
        resources/RVSelect.cpp
        resources/RVAccount.cpp
        resources/RVWalltime.cpp
        resources/RVPBSProGeneric.cpp
        resources/RVBatchGroup.cpp
        resources/RVUMask.cpp
        resources/RVPlace.cpp
        resources/RVProps.cpp
        resources/RVGPUCap.cpp
        )

# create shared library --------------------------------------------------------
ADD_LIBRARY(pbspro_shared SHARED ${PBSPRO_SRC})
ADD_DEPENDENCIES(pbspro_shared abs_shared)

SET_TARGET_PROPERTIES(pbspro_shared PROPERTIES
                        OUTPUT_NAME PBSPro
                        CLEAN_DIRECT_OUTPUT 1
                        VERSION ${ABS_VERSION})

TARGET_LINK_LIBRARIES(pbspro_shared ${ABS_LIBS})

INSTALL(TARGETS
            pbspro_shared
        DESTINATION
            lib
        )

