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
ADD_DEPENDENCIES(paliases abs_shared)

TARGET_LINK_LIBRARIES(paliases ${ABS_LIBS})

INSTALL(TARGETS
            paliases
        RUNTIME DESTINATION
            bin
        )

