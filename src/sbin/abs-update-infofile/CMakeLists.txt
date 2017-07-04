## =============================================================================
# ABS CMake File
## -----------------------------------------------------------------------------

# program objects --------------------------------------------------------------
SET(UPDATE_INFO_FILE_SRC
        UpdateInfoFile.cpp
        UpdateInfoFileOptions.cpp
        )

# final build ------------------------------------------------------------------
ADD_EXECUTABLE(abs-update-infofile ${UPDATE_INFO_FILE_SRC})
ADD_DEPENDENCIES(abs-update-infofile abs_shared)

TARGET_LINK_LIBRARIES(abs-update-infofile ${ABS_LIBS})

INSTALL(TARGETS
            abs-update-infofile
        RUNTIME DESTINATION
            sbin
        )

