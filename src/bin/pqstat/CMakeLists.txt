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

TARGET_LINK_LIBRARIES(pqstat ${ABS_LIBS})

INSTALL(TARGETS
            pqstat
        RUNTIME DESTINATION
            bin
        )

