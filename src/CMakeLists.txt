# ==============================================================================
# ABS CMake File
# ==============================================================================

INCLUDE_DIRECTORIES(lib/plusula SYSTEM)
INCLUDE_DIRECTORIES(lib/abs SYSTEM)
INCLUDE_DIRECTORIES(lib/abs/system SYSTEM)
INCLUDE_DIRECTORIES(lib/abs/jobs SYSTEM)
INCLUDE_DIRECTORIES(lib/abs/jobs/search SYSTEM)
INCLUDE_DIRECTORIES(lib/abs/jobs/search/nodes SYSTEM)
INCLUDE_DIRECTORIES(lib/abs/jobs/search/jobs SYSTEM)

# include subdirectories -------------------------------------------------------
ADD_SUBDIRECTORY(lib)
ADD_SUBDIRECTORY(bin)
#ADD_SUBDIRECTORY(sbin)
