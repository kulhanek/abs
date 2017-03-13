#ifndef IJobsOptionsH
#define IJobsOptionsH
// =============================================================================
// ABS - Advanced Batch System
// -----------------------------------------------------------------------------
//    Copyright (C) 2013      Petr Kulhanek, kulhanek@chemi.muni.cz
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

class CIJobsOptions : public CSimpleOptions {
public:
        // constructor - tune option setup
        CIJobsOptions(void);

// program name and description -----------------------------------------------
    CSO_PROG_NAME_BEGIN
        "abs-ijobs"
    CSO_PROG_NAME_END

    CSO_PROG_DESC_BEGIN
        "Set/get information related to ijobs."
    CSO_PROG_DESC_END

    CSO_PROG_VERS_BEGIN
        ABS_VERSION
    CSO_PROG_VERS_END

// list of all options and arguments ------------------------------------------
    CSO_LIST_BEGIN
        CSO_ARG(CSmallString,InfoFile)
        CSO_ARG(CSmallString,Property)
        // options ------------------------------
        CSO_OPT(int,ID)
        CSO_OPT(CSmallString,Set)
        CSO_OPT(bool,Help)
        CSO_OPT(bool,Version)
        CSO_OPT(bool,Verbose)
    CSO_LIST_END

// -----------------------------------------------------------------------------
    CSO_MAP_BEGIN
        CSO_MAP_ARG(CSmallString,                   /* argument type */
                    InfoFile,                          /* argument name */
                    NULL,                           /* default value */
                    true,                           /* is argument mandatory */
                    "InfoFile",                        /* parametr name */
                    "info file containing ijobs information")   /* argument description */
        //----------------------------------------------------------------------
        CSO_MAP_ARG(CSmallString,                   /* argument type */
                    Property,                          /* argument name */
                    NULL,                           /* default value */
                    true,                           /* is argument mandatory */
                    "property",                        /* parametr name */
                    "requested property (supported: )")   /* argument description */
    // description of options -----------------------------------------------------
        CSO_MAP_OPT(int,                           /* option type */
                    ID,                        /* option name */
                    1,                          /* default value */
                    false,                          /* is option mandatory */
                    'i',                           /* short option name */
                    "id",                      /* long option name */
                    "NUMBER",                           /* parametr name */
                    "ijob ID")   /* option description */
        //----------------------------------------------------------------------
        CSO_MAP_OPT(CSmallString,                           /* option type */
                    Set,                        /* option name */
                    NULL,                          /* default value */
                    false,                          /* is option mandatory */
                    's',                           /* short option name */
                    "set",                      /* long option name */
                    "VALUE",                           /* parametr name */
                    "set property to provided value")   /* option description */
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
