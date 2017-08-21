#ifndef InfoGoOptionsH
#define InfoGoOptionsH
// =============================================================================
// ABS - Advanced Batch System
// -----------------------------------------------------------------------------
//    Copyright (C) 2011      Petr Kulhanek, kulhanek@chemi.muni.cz
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

class CInfoGoOptions : public CSimpleOptions {
    public:
        // constructor - tune option setup
        CInfoGoOptions(void);

 // program name and description -----------------------------------------------
    CSO_PROG_NAME_BEGIN
        "pgo"
    CSO_PROG_NAME_END

    CSO_PROG_DESC_BEGIN
        "Change current directory into the job input or working directory."
    CSO_PROG_DESC_END

    CSO_PROG_ARGS_SHORT_DESC_BEGIN
        "[info/jobid]"
    CSO_PROG_ARGS_SHORT_DESC_END

    CSO_PROG_ARGS_LONG_DESC_BEGIN
        "<b>Arguments:</b>\n"
        "   <u>info</u>  name of job info file\n"
        "   <u>jobid</u> batch system job ID"
    CSO_PROG_ARGS_LONG_DESC_END

    CSO_PROG_VERS_BEGIN
        LibBuildVersion_ABS
    CSO_PROG_VERS_END

 // list of all options and arguments ------------------------------------------
    CSO_LIST_BEGIN
        // options ------------------------------
        CSO_OPT(bool,Force)
        CSO_OPT(bool,NoWait)
        CSO_OPT(bool,NoTerminal)
        CSO_OPT(bool,Proxy)
        CSO_OPT(bool,Help)
        CSO_OPT(bool,Version)
        CSO_OPT(bool,Verbose)
    CSO_LIST_END

    CSO_MAP_BEGIN
 // description of options -----------------------------------------------------
        CSO_MAP_OPT(bool,                           /* option type */
                    Force,                        /* option name */
                    false,                          /* default value */
                    false,                          /* is option mandatory */
                    'f',                           /* short option name */
                    "force",                      /* long option name */
                    NULL,                           /* parametr name */
                    "try to go to the working directory at any circumstances")   /* option description */
    //----------------------------------------------------------------------
        CSO_MAP_OPT(bool,                           /* option type */
                    NoWait,                        /* option name */
                    false,                          /* default value */
                    false,                          /* is option mandatory */
                    'n',                           /* short option name */
                    "nowait",                      /* long option name */
                    NULL,                           /* parametr name */
                    "do not wait until a job is running")   /* option description */
    //----------------------------------------------------------------------
        CSO_MAP_OPT(bool,                           /* option type */
                    NoTerminal,                        /* option name */
                    false,                          /* default value */
                    false,                          /* is option mandatory */
                    0,                           /* short option name */
                    "noterm",                      /* long option name */
                    NULL,                           /* parametr name */
                    "do not login to terminal")   /* option description */
    //----------------------------------------------------------------------
        CSO_MAP_OPT(bool,                           /* option type */
                    Proxy,                        /* option name */
                    false,                          /* default value */
                    false,                          /* is option mandatory */
                    0,                           /* short option name */
                    "proxy",                      /* long option name */
                    NULL,                           /* parametr name */
                    "start VNC proxy server for the GUI job")   /* option description */
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
