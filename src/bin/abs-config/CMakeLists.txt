# ==============================================================================
# ABS CMake File
# ==============================================================================

# program objects --------------------------------------------------------------
SET(PROG_SRC
        config-cli.cpp
        config-cli-opts.cpp
        )

# final build ------------------------------------------------------------------
ADD_EXECUTABLE(abs-config ${PROG_SRC})
ADD_DEPENDENCIES(abs-config abs_shared)

TARGET_LINK_LIBRARIES(abs-config ${ABS_LIBS})

INSTALL(TARGETS
            abs-config
        DESTINATION
            bin
        )

