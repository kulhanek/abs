#ifndef UpdateInfoFileOptionsH
#define UpdateInfoFileOptionsH
// =============================================================================
// ABS - Advanced Batch System
// -----------------------------------------------------------------------------
//    Copyright (C) 2012      Petr Kulhanek, kulhanek@chemi.muni.cz
//    Copyright (C) 2001-2008 Petr Kulhanek, kulhanek@chemi.muni.cz
//
//     This program is free software; you can redistribute it and/or modify
//     it under the terms of the GNU General Public License as published by
//     the Free Software Foundation; either version 2 of the License, or
//     (at your option) any later version.
//
//     This program is distributed in the hope that it will be useful,
//     but WITHOUT ANY WARRANTY; without even the implied warranty of
//     MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//     GNU General Public License for more details.
//
//     You should have received a copy of the GNU General Public License along
//     with this program; if not, write to the Free Software Foundation, Inc.,
//     51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
// =============================================================================

#include <SimpleOptions.hpp>
#include <ABSMainHeader.hpp>

//------------------------------------------------------------------------------

class CUpdateInfoFileOptions : public CSimpleOptions {
public:
        // constructor - tune option setup
        CUpdateInfoFileOptions(void);

// program name and description -----------------------------------------------
    CSO_PROG_NAME_BEGIN
        "abs-update-infofile"
    CSO_PROG_NAME_END

    CSO_PROG_DESC_BEGIN
        "Update information about a job in the job info file. Supported actions are as follows:\n"
        "      start        - job was started\n"
        "      stop         - job was terminated\n"
        "      cliready     - CLI terminal is available\n"
        "      guiready     - GUI terminal is available\n"
        "      incstage     - increment INF_RECYCLE_CURRENT\n"
        "      archive      - archive runtime files of finished jobs\n"
        "      clean        - clean runtime files of finished jobs\n"
    CSO_PROG_DESC_END

    CSO_PROG_VERS_BEGIN
        LibBuildVersion_ABS
    CSO_PROG_VERS_END

// list of all options and arguments ------------------------------------------
    CSO_LIST_BEGIN
        CSO_ARG(CSmallString,Action)
        // options ------------------------------
        CSO_OPT(CSmallString,Agent)
        CSO_OPT(CSmallString,VNCID)
        CSO_OPT(CSmallString,Format)
        CSO_OPT(bool,Help)
        CSO_OPT(bool,Version)
        CSO_OPT(bool,Verbose)
    CSO_LIST_END

    CSO_MAP_BEGIN
        CSO_MAP_ARG(CSmallString,                   /* argument type */
                    Action,                          /* argument name */
                    NULL,                           /* default value */
                    true,                           /* is argument mandatory */
                    "action",                        /* parametr name */
                    "what should be updated")   /* argument description */
    // description of options -----------------------------------------------------
        CSO_MAP_OPT(CSmallString,                           /* option type */
                    Agent,                        /* option name */
                    NULL,                          /* default value */
                    false,                          /* is option mandatory */
                    '\0',                           /* short option name */
                    "agent",                      /* long option name */
                    "MODULE",                           /* parametr name */
                    "terminal agent module name")   /* option description */
    // description of options -----------------------------------------------------
        CSO_MAP_OPT(CSmallString,                           /* option type */
                    VNCID,                        /* option name */
                    NULL,                          /* default value */
                    false,                          /* is option mandatory */
                    '\0',                           /* short option name */
                    "vncid",                      /* long option name */
                    "ID",                           /* parametr name */
                    "vnc terminal id in the form of user@machine:display")   /* option description */
        //----------------------------------------------------------------------
        CSO_MAP_OPT(CSmallString,                           /* option type */
                Format,                        /* option name */
                NULL,                          /* default value */
                false,                          /* is option mandatory */
                '\0',                           /* short option name */
                "format",                      /* long option name */
                "STRING",                           /* parametr name */
                "format for runtime file backup")   /* option description */
        //----------------------------------------------------------------------
        CSO_MAP_OPT(bool,                           /* option type */
                    Verbose,                        /* option name */
                    false,                          /* default value */
                    false,                          /* is option mandatory */
                    'v',                           /* short option name */
                    "verbose",                      /* long option name */
                    NULL,                           /* parametr name */
                    "increase output verbosity")   /* option description */
        //----------------------------------------------------------------------
        CSO_MAP_OPT(bool,                           /* option type */
                    Version,                        /* option name */
                    false,                          /* default value */
                    false,                          /* is option mandatory */
                    '\0',                           /* short option name */
                    "version",                      /* long option name */
                    NULL,                           /* parametr name */
                    "output version information and exit")   /* option description */
        //----------------------------------------------------------------------
        CSO_MAP_OPT(bool,                           /* option type */
                    Help,                        /* option name */
                    false,                          /* default value */
                    false,                          /* is option mandatory */
                    'h',                           /* short option name */
                    "help",                      /* long option name */
                    NULL,                           /* parametr name */
                    "display this help and exit")   /* option description */
    CSO_MAP_END

// final operation with options ------------------------------------------------
    private:
    virtual int CheckOptions(void);
    virtual int FinalizeOptions(void);
    virtual int CheckArguments(void);
};

//------------------------------------------------------------------------------

#endif
