# ==============================================================================
# Infinity CMake File
# ==============================================================================

# program objects --------------------------------------------------------------
SET(PROG_SRC
        Cgen.cpp
        CgenOptions.cpp
        )

# final build ------------------------------------------------------------------
ADD_EXECUTABLE(_abs-cgen ${PROG_SRC})

TARGET_LINK_LIBRARIES(_abs-cgen ${ABS_LIBS})

INSTALL(TARGETS
            _abs-cgen
        DESTINATION
            bin
        )

