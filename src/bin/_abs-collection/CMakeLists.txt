# ==============================================================================
# ABS CMake File
# ==============================================================================

# program objects --------------------------------------------------------------
SET(COLLECTION_SRC
        Collection.cpp
        CollectionOptions.cpp
        )

# final build ------------------------------------------------------------------
ADD_EXECUTABLE(_abs-collection ${COLLECTION_SRC})

TARGET_LINK_LIBRARIES(_abs-collection ${ABS_LIBS})

INSTALL(TARGETS
            _abs-collection
        RUNTIME DESTINATION
            bin
        )

