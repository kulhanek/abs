# ==============================================================================
# ABS CMake File
# ==============================================================================

SET(EXECUTABLE_OUTPUT_PATH ${CMAKE_BINARY_DIR}/sbin)

# system wide utilities --------------------------
ADD_SUBDIRECTORY(abs-get-setup-item)
ADD_SUBDIRECTORY(abs-restore-env)
ADD_SUBDIRECTORY(abs-update-infofile)
ADD_SUBDIRECTORY(abs-get-work-dir)
ADD_SUBDIRECTORY(abs-ijobs)
ADD_SUBDIRECTORY(abs-sort-nodes)

