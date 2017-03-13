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

TARGET_LINK_LIBRARIES(psanitize ${ABS_LIBS})

INSTALL(TARGETS
            psanitize
        RUNTIME DESTINATION
            bin
        )

