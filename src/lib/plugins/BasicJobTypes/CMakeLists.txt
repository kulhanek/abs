# ==============================================================================
# ABS CMake File
# ==============================================================================

# objects in library -----------------------------------------------------------
SET(BASICJOBTYPES_SRC
        BasicJobTypesMainHeader.cpp
        BasicJobTypesModule.cpp
        GaussianJobType.cpp
        TurbomoleJobType.cpp
        PRecycleJobType.cpp
        PEquiJobType.cpp
        NWChemJobType.cpp
        PRecycleCPMDJobType.cpp
        ICollectionJobType.cpp
        ICollectionJobTypeOptions.cpp
        STMServerJobType.cpp
        STMServerJobTypeOptions.cpp
        STMClientJobType.cpp
        STMClientJobTypeOptions.cpp
        LoopJobType.cpp
        MultiWfnJobType.cpp
        OrcaJobType.cpp
        )

# create shared library --------------------------------------------------------
ADD_LIBRARY(basicjobtypes_shared SHARED ${BASICJOBTYPES_SRC})
ADD_DEPENDENCIES(basicjobtypes_shared abs_shared)

SET_TARGET_PROPERTIES(basicjobtypes_shared PROPERTIES
                        OUTPUT_NAME BasicJobTypes
                        CLEAN_DIRECT_OUTPUT 1
                        VERSION ${ABS_VERSION})

TARGET_LINK_LIBRARIES(basicjobtypes_shared ${ABS_LIBS})

INSTALL(TARGETS
            basicjobtypes_shared
        DESTINATION
            lib
        )

