# ==============================================================================
# ABS CMake File
# ==============================================================================

# program objects --------------------------------------------------------------
SET(STATUS_SRC
        Status.cpp
        StatusOptions.cpp
        )

# final build ------------------------------------------------------------------
ADD_EXECUTABLE(pstatus ${STATUS_SRC})
ADD_DEPENDENCIES(pstatus abs_shared)

TARGET_LINK_LIBRARIES(pstatus ${ABS_LIBS})

INSTALL(TARGETS
            pstatus
        RUNTIME DESTINATION
            bin
        )

