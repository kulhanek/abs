# ==============================================================================
# ABS CMake File
# ==============================================================================

# program objects --------------------------------------------------------------
SET(SANITIZE_SRC
        Sanitize.cpp
        SanitizeOptions.cpp
        )

# final build ------------------------------------------------------------------
ADD_EXECUTABLE(psanitize ${SANITIZE_SRC})
ADD_DEPENDENCIES(psanitize abs_shared)

TARGET_LINK_LIBRARIES(psanitize ${ABS_LIBS})

INSTALL(TARGETS
            psanitize
        RUNTIME DESTINATION
            bin
        )

