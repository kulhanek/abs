# ==============================================================================
# ABS CMake File
# ==============================================================================

IF(ABS_CORE_ONLY)
    ADD_SUBDIRECTORY(plusula)
    ADD_SUBDIRECTORY(abs)
ELSE(ABS_CORE_ONLY)
    ADD_SUBDIRECTORY(plusula)
    ADD_SUBDIRECTORY(abs)
    ADD_SUBDIRECTORY(plugins)
ENDIF(ABS_CORE_ONLY)




