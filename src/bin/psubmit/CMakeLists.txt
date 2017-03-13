# ==============================================================================
# ABS CMake File
# ==============================================================================

# program objects --------------------------------------------------------------
SET(SUBMIT_SRC
        Submit.cpp
        SubmitOptions.cpp
        )

# final build ------------------------------------------------------------------
ADD_EXECUTABLE(psubmit ${SUBMIT_SRC})
ADD_DEPENDENCIES(psubmit abs_shared)

TARGET_LINK_LIBRARIES(psubmit ${ABS_LIBS})

INSTALL(TARGETS
            psubmit
        DESTINATION
            bin
        )

