# ==============================================================================
# ABS CMake File
# ==============================================================================

INSTALL(FILES
            init.abs
       DESTINATION
            etc/boot
        PERMISSIONS
            OWNER_READ OWNER_WRITE OWNER_EXECUTE GROUP_READ GROUP_EXECUTE WORLD_READ WORLD_EXECUTE
        )
