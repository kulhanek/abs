# ==============================================================================
# ABS CMake File
# ==============================================================================

INSTALL(FILES
            abs-cleanipcs
            abs-execution-script-LL
            abs-execution-script-L0
            abs-execution-script-L1
            abs-execution-script-L2
            abs-execution-script-L3
            abs-execution-script-L4
            abs-execution-script-L5
            abs-execution-script-L6
            abs-execution-script-L6-ijobs
            abs-execution-ijob
            abs-go
            abs-go-remote
            abs-kill
            abs-sync
            screenrc
       DESTINATION
            share/scripts
        PERMISSIONS
            OWNER_READ OWNER_WRITE OWNER_EXECUTE GROUP_READ GROUP_EXECUTE WORLD_READ WORLD_EXECUTE
        )
