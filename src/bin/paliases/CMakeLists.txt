# ==============================================================================
# Infinity CMake File
# ==============================================================================

# program objects --------------------------------------------------------------
SET(ALIASES_SRC
        Aliases.cpp
        AliasesOptions.cpp
        )

# final build ------------------------------------------------------------------
ADD_EXECUTABLE(paliases ${ALIASES_SRC})

TARGET_LINK_LIBRARIES(paliases ${ABS_LIBS})

INSTALL(TARGETS
            paliases
        RUNTIME DESTINATION
            bin
        )

