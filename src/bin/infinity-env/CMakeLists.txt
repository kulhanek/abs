# ==============================================================================
# ABS CMake File
# ==============================================================================

# program objects --------------------------------------------------------------
SET(ENV_SRC
        Env.cpp
        EnvOptions.cpp
        )

# final build ------------------------------------------------------------------
ADD_EXECUTABLE(infinity-env ${ENV_SRC})
ADD_DEPENDENCIES(infinity-env abs_shared)

TARGET_LINK_LIBRARIES(infinity-env ${ABS_LIBS})

INSTALL(TARGETS
            infinity-env
        RUNTIME DESTINATION
            bin
        )

