# ==============================================================================
# ABS CMake File
# ==============================================================================

# objects in library -----------------------------------------------------------
SET(ABS_SRC
        ABSMainHeader.cpp
    # system -------------------------------------
        system/GlobalConfig.cpp
        system/User.cpp
        system/Shell.cpp
        system/ShellProcessor.cpp
        system/Completion.cpp
        system/CategoryUUID.cpp
        system/TicketChecker.cpp
        system/JobType.cpp

    # software -----------------------------------
        soft/AmsUUID.cpp
        soft/Site.cpp
        soft/Utils.cpp
        soft/Cache.cpp

    # jobs ---------------------------------------
        jobs/search/nodes/NodeMask.tab.c
        jobs/search/nodes/NodeMask.yy.c
        jobs/search/jobs/JobMask.tab.c
        jobs/search/jobs/JobMask.yy.c
        jobs/search/CommonParser.cpp

        jobs/TorqueConfig.cpp
        jobs/ResourceValue.cpp
        jobs/ResourceList.cpp
        jobs/Alias.cpp
        jobs/AliasList.cpp
        jobs/Job.cpp
        jobs/JobList.cpp
        jobs/Node.cpp
        jobs/NodeList.cpp
        jobs/Queue.cpp
        jobs/QueueList.cpp
        jobs/TorqueAttr.cpp
        jobs/Torque.cpp
        )

# create shared library --------------------------------------------------------
ADD_DEFINITIONS(-DABS_BUILDING_DLL)
ADD_LIBRARY(abs_shared SHARED ${ABS_SRC})
SET_TARGET_PROPERTIES(abs_shared PROPERTIES
                        OUTPUT_NAME abs
                        CLEAN_DIRECT_OUTPUT 1
                        VERSION ${LibBuildVersion_ABS})
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

