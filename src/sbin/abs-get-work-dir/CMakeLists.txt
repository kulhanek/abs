## =============================================================================
# ABS CMake File
## -----------------------------------------------------------------------------

# program objects --------------------------------------------------------------
SET(CONFIG_SRC
        GetWorkDir.cpp
        GetWorkDirOptions.cpp
        )

# final build ------------------------------------------------------------------
ADD_EXECUTABLE(abs-get-work-dir ${CONFIG_SRC})

TARGET_LINK_LIBRARIES(abs-get-work-dir ${ABS_LIBS})

INSTALL(TARGETS
            abs-get-work-dir
        RUNTIME DESTINATION
            sbin
        )

