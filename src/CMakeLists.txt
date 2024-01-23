# ==============================================================================
# ABS CMake File
# ==============================================================================

INCLUDE_DIRECTORIES(lib/plusula SYSTEM)
INCLUDE_DIRECTORIES(lib/abs SYSTEM)
INCLUDE_DIRECTORIES(lib/abs/system SYSTEM)
INCLUDE_DIRECTORIES(lib/abs/resources SYSTEM)
INCLUDE_DIRECTORIES(lib/abs/resources/default SYSTEM)
INCLUDE_DIRECTORIES(lib/abs/jobs SYSTEM)
INCLUDE_DIRECTORIES(lib/abs/jobs/search SYSTEM)
INCLUDE_DIRECTORIES(lib/abs/jobs/search/nodes SYSTEM)
INCLUDE_DIRECTORIES(lib/abs/jobs/search/jobs SYSTEM)

# include subdirectories -------------------------------------------------------
IF(ABS_CORE_ONLY)
    ADD_SUBDIRECTORY(lib)
ELSE(ABS_CORE_ONLY)
    ADD_SUBDIRECTORY(lib)
    ADD_SUBDIRECTORY(bin)
    ADD_SUBDIRECTORY(sbin)
ENDIF(ABS_CORE_ONLY)



