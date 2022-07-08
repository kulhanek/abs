# ==============================================================================
# ABS CMake File
# ==============================================================================

# objects in library -----------------------------------------------------------
SET(ABS_SRC
        ABSMainHeader.cpp
        ABSModule.cpp
        GlobalObjects.cpp

    # boost/1.65.1 -------------------------------
        mboost/path.cpp
        mboost/operations.cpp
        mboost/error_code.cpp

    # system -------------------------------------
        system/ABSConfig.cpp
        system/ABSCompletion.cpp
        system/CategoryUUID.cpp
        system/TicketChecker.cpp
        system/JobType.cpp
        system/BatchServer.cpp
        system/BatchServers.cpp

    # resources ----------------------------------
        resources/ResourceValue.cpp
        resources/ResourceList.cpp
        resources/default/RVEmail.cpp
        resources/default/RVAppMem.cpp
        resources/default/RVAppMemPerNode.cpp
        resources/default/RVMem.cpp
        resources/default/RVMemPerCPU.cpp
        resources/default/RVNCPUs.cpp
        resources/default/RVNCPUsPerNode.cpp
        resources/default/RVNCPUsPerNUMANode.cpp
        resources/default/RVNGPUs.cpp
        resources/default/RVNNodes.cpp
        resources/default/RVWorkDir.cpp
        resources/default/RVWorkSize.cpp
        resources/default/RVWorkSizePerCPU.cpp
        resources/default/RVDataIn.cpp
        resources/default/RVDataOut.cpp
        resources/default/RVStorageGroup.cpp
        resources/default/RVFixPerms.cpp
        resources/default/RVMPISlotsPerNode.cpp
        resources/default/RVRandomize.cpp

    # jobs ---------------------------------------
        jobs/search/nodes/NodeMask.tab.c
        jobs/search/nodes/NodeMask.yy.c
        jobs/search/jobs/JobMask.tab.c
        jobs/search/jobs/JobMask.yy.c
        jobs/search/CommonParser.cpp

        jobs/Alias.cpp
        jobs/AliasList.cpp
        jobs/Job.cpp
        jobs/JobCompat.cpp
        jobs/JobList.cpp
        jobs/Node.cpp
        jobs/NodeList.cpp
        jobs/Queue.cpp
        jobs/QueueList.cpp
        )

# create shared library --------------------------------------------------------
ADD_DEFINITIONS(-DABS_BUILDING_DLL)

ADD_LIBRARY(abs_shared SHARED ${ABS_SRC})
ADD_DEPENDENCIES(abs_shared plusula_shared)

SET_TARGET_PROPERTIES(abs_shared PROPERTIES
                        OUTPUT_NAME abs
                        CLEAN_DIRECT_OUTPUT 1
                        VERSION ${ABS_VERSION})

TARGET_LINK_LIBRARIES(abs_shared ${ABS_EXT_LIBS})

INSTALL(TARGETS
            abs_shared
        LIBRARY DESTINATION
            lib)

# ------------------------------------------------------------------------------
# test
#SET(TEST_SRC
#        jobs/search/nodes/NodeMask.tab.c
#        jobs/search/nodes/NodeMask.yy.c
#        jobs/search/nodes/NodeParser.cpp
#        )

#ADD_EXECUTABLE(mask-test ${TEST_SRC})

#TARGET_LINK_LIBRARIES(mask-test ${ABS_LIBS})

