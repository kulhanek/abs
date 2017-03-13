## =============================================================================
# ABS CMake File
## -----------------------------------------------------------------------------

# program objects --------------------------------------------------------------
SET(CONFIG_SRC
        GetScratchDir.cpp
        GetScratchDirOptions.cpp
        )

# final build ------------------------------------------------------------------
ADD_EXECUTABLE(abs-get-scratch-dir ${CONFIG_SRC})

TARGET_LINK_LIBRARIES(abs-get-scratch-dir ${ABS_LIBS})

INSTALL(TARGETS
            abs-get-scratch-dir
        RUNTIME DESTINATION
            sbin
        )

