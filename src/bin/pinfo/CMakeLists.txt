# ==============================================================================
# ABS CMake File
# ==============================================================================

# program objects --------------------------------------------------------------
SET(INFO_SRC
        Info.cpp
        InfoOptions.cpp
        )

# final build ------------------------------------------------------------------
ADD_EXECUTABLE(pinfo ${INFO_SRC})

TARGET_LINK_LIBRARIES(pinfo ${ABS_LIBS})

INSTALL(TARGETS
            pinfo
        RUNTIME DESTINATION
            bin
        )

