# ==============================================================================
# Infinity CMake File
# ==============================================================================

# objects in library -----------------------------------------------------------
SET(PLUSULA_SRC
        PluSuLaMainHeader.cpp
        ComObject.cpp
        ExtUUID.cpp
        PluginDatabase.cpp
        PluginModule.cpp
        PluginObject.cpp
        )

# for plugin enabled program, dynamic linking is obligatory!

# create shared library --------------------------------------------------------
ADD_DEFINITIONS(-DPLUSULA_BUILDING_DLL)

ADD_LIBRARY(plusula_shared SHARED ${PLUSULA_SRC})

TARGET_LINK_LIBRARIES(plusula_shared
                ${HIPOLY_LIB_NAME}
                ${SYSTEM_LIBS}
                )

SET_TARGET_PROPERTIES(plusula_shared PROPERTIES
                OUTPUT_NAME plusula
                CLEAN_DIRECT_OUTPUT 1
                VERSION ${LibBuildVersion_ABS}
                )

INSTALL(TARGETS
            plusula_shared
        DESTINATION
            lib
        )
