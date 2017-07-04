## =============================================================================
# ABS CMake File
## -----------------------------------------------------------------------------

# program objects --------------------------------------------------------------
SET(CONFIG_SRC
        GetConfig.cpp
        GetConfigOptions.cpp
        )

# final build ------------------------------------------------------------------
ADD_EXECUTABLE(abs-get-setup-item ${CONFIG_SRC})
ADD_DEPENDENCIES(abs-get-setup-item abs_shared)

TARGET_LINK_LIBRARIES(abs-get-setup-item ${ABS_LIBS})

INSTALL(TARGETS
            abs-get-setup-item
        RUNTIME DESTINATION
            sbin
        )

