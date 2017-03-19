# ==============================================================================
# ABS CMake File
# ==============================================================================

# objects in library -----------------------------------------------------------
SET(BASICTCTCHK_SRC
        BasicTicketCheckMainHeader.cpp
        BasicTicketCheckModule.cpp
        SSHTicketChecker.cpp
        KerberosTicketChecker.cpp
        NoTicketChecker.cpp
        )

# create shared library --------------------------------------------------------
ADD_LIBRARY(basictctchk_shared SHARED ${BASICTCTCHK_SRC})
ADD_DEPENDENCIES(basictctchk_shared abs_shared)

SET_TARGET_PROPERTIES(basictctchk_shared PROPERTIES
                        OUTPUT_NAME BasicTicketCheck
                        CLEAN_DIRECT_OUTPUT 1
                        VERSION ${ABS_VERSION})

TARGET_LINK_LIBRARIES(basictctchk_shared ${ABS_LIBS})

INSTALL(TARGETS
            basictctchk_shared
        DESTINATION
            lib
        )

