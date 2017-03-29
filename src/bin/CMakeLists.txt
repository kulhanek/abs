# ==============================================================================
# ABS CMake File
# ==============================================================================

# general commands -------------------------------
ADD_SUBDIRECTORY(infinity-env)
#ADD_SUBDIRECTORY(abs-config)

# inline commands --------------------------------
#ADD_SUBDIRECTORY(_abs-cgen)
#ADD_SUBDIRECTORY(_abs-go)
#ADD_SUBDIRECTORY(_abs-collection)
#ADD_SUBDIRECTORY(_abs-kill)
#ADD_SUBDIRECTORY(_abs-sync)

# jobs commands ----------------------------------
ADD_SUBDIRECTORY(paliases)
ADD_SUBDIRECTORY(pnodes)
ADD_SUBDIRECTORY(pqueues)
ADD_SUBDIRECTORY(pjobs)
ADD_SUBDIRECTORY(pqstat)
ADD_SUBDIRECTORY(pinfo)
ADD_SUBDIRECTORY(psubmit)
ADD_SUBDIRECTORY(pstatus)
ADD_SUBDIRECTORY(pkillall)
ADD_SUBDIRECTORY(psanitize)

