# ==============================================================================
# Infinity CMake File
# ==============================================================================

# program objects --------------------------------------------------------------
SET(QSTAT_SRC
        QStat.cpp
        QStatOptions.cpp
        )

# final build ------------------------------------------------------------------
ADD_EXECUTABLE(pqstat ${QSTAT_SRC})
ADD_DEPENDENCIES(pqstat abs_shared)

TARGET_LINK_LIBRARIES(pqstat ${ABS_LIBS})

INSTALL(TARGETS
            pqstat
        RUNTIME DESTINATION
            bin
        )

