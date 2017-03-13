# ==============================================================================
# ABS CMake File
# ==============================================================================

INSTALL(FILES
            presubmit
            infinity-ijobs-prepare
            infinity-ijobs-launch
            infinity-ijobs-finalize
            infinity-ijobs-copy-into
            parchive
       DESTINATION
            bin
        PERMISSIONS
            OWNER_READ OWNER_WRITE OWNER_EXECUTE GROUP_READ GROUP_EXECUTE WORLD_READ WORLD_EXECUTE
        )
