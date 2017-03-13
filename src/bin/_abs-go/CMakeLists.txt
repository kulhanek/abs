# ==============================================================================
# ABS CMake File
# ==============================================================================

# program objects --------------------------------------------------------------
SET(INFO_GO_SRC
        InfoGo.cpp
        InfoGoOptions.cpp
        )

# final build ------------------------------------------------------------------
ADD_EXECUTABLE(_abs-go ${INFO_GO_SRC})

TARGET_LINK_LIBRARIES(_abs-go ${ABS_LIBS})

INSTALL(TARGETS
            _abs-go
        RUNTIME DESTINATION
            bin
        )

