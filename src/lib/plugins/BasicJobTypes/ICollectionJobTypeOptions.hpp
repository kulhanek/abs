#ifndef ICollectionJobTypeOptionsH
#define ICollectionJobTypeOptionsH
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

class CIJobOptions : public CSimpleOptions {
    public:
        // constructor - tune option setup
        CIJobOptions(void);

 // program name and description -----------------------------------------------
    CSO_PROG_NAME_BEGIN
        "isubmit"
    CSO_PROG_NAME_END

    CSO_PROG_DESC_BEGIN
        "Submit a job as a part of icollection."
    CSO_PROG_DESC_END

    CSO_PROG_VERS_BEGIN
        LibBuildVersion_ABS
    CSO_PROG_VERS_END

 // list of all options and arguments ------------------------------------------
    CSO_LIST_BEGIN
        CSO_ARG(CSmallString,Directory)
        CSO_ARG(CSmallString,JobName)
        CSO_ARG(CSmallString,NCPUs)
        // options ------------------------------
        CSO_OPT(int,From)
        CSO_OPT(int,To)
        CSO_OPT(bool,Help)
        CSO_OPT(bool,Version)
        CSO_OPT(bool,Verbose)
    CSO_LIST_END

    CSO_MAP_BEGIN
 // description of args --------------------------------------------------------
        CSO_MAP_ARG(CSmallString,                   /* argument type */
                    Directory,                          /* argument name */
                    NULL,                           /* default value */
                    true,                           /* is argument mandatory */
                    "DIR",                        /* parametr name */
                    "job directory")   /* argument description */
    //----------------------------------------------------------------------
        CSO_MAP_ARG(CSmallString,                   /* argument type */
                    JobName,                          /* argument name */
                    NULL,                           /* default value */
                    true,                           /* is argument mandatory */
                    "NAME",                        /* parametr name */
                    "job name")   /* argument description */
    //----------------------------------------------------------------------
        CSO_MAP_ARG(CSmallString,                   /* argument type */
                    NCPUs,                          /* argument name */
                    NULL,                           /* default value */
                    true,                           /* is argument mandatory */
                    "NCPUs",                        /* parametr name */
                    "number of CPUs used by a job")   /* argument description */
 // description of options -----------------------------------------------------
        CSO_MAP_OPT(int,                           /* option type */
                From,                        /* option name */
                0,                          /* default value */
                false,                          /* is option mandatory */
                'f',                           /* short option name */
                "from",                      /* long option name */
                "NUM",                           /* parametr name */
                "starting index for parametric isubmit")   /* option description */
    //----------------------------------------------------------------------
        CSO_MAP_OPT(int,                           /* option type */
                To,                        /* option name */
                0,                          /* default value */
                false,                          /* is option mandatory */
                't',                           /* short option name */
                "to",                      /* long option name */
                "NUM",                           /* parametr name */
                "final index for parametric isubmit")   /* option description */
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
