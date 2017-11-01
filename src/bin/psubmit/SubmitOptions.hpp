#ifndef SubmitOptionsH
#define SubmitOptionsH
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

class CSubmitOptions : public CSimpleOptions {
    public:
        // constructor - tune option setup
        CSubmitOptions(void);

 // program name and description -----------------------------------------------
    CSO_PROG_NAME_BEGIN
        "psubmit"
    CSO_PROG_NAME_END

    CSO_PROG_DESC_BEGIN
        "Submit a job into the batch system."
    CSO_PROG_DESC_END

    CSO_PROG_ARGS_SHORT_DESC_BEGIN
        "destination job [resource[,resource, ...] [resource] ...]"
    CSO_PROG_ARGS_SHORT_DESC_END

    CSO_PROG_ARGS_LONG_DESC_BEGIN
        "<b>Arguments:</b>\n"
        "   <b><cyan>destination</cyan></b>\n"
        "          name of batch queue or Infinity alias\n\n"
        "   <b><cyan>job</cyan></b>\n"
        "          job script or job input file\n\n"
        "   <b><cyan>resources</cyan></b>\n"
        "          number of CPUs or explicit resources, comma or space separated items\n"
    CSO_PROG_ARGS_LONG_DESC_END

    CSO_PROG_VERS_BEGIN
        LibBuildVersion_ABS
    CSO_PROG_VERS_END

 // list of all options and arguments ------------------------------------------
    CSO_LIST_BEGIN
        // options ------------------------------
        CSO_OPT(int,NumOfCopies)
        CSO_OPT(bool,IgnoreRuntimeFiles)
        CSO_OPT(bool,AssumeYes)
        CSO_OPT(bool,Silent)
        CSO_OPT(bool,ExpertMode)
        CSO_OPT(bool,ResubmitMode)
        CSO_OPT(bool,AllowAllPaths)
        CSO_OPT(bool,Help)
        CSO_OPT(bool,Version)
        CSO_OPT(bool,Verbose)
    CSO_LIST_END

    CSO_MAP_BEGIN
 // description of options -----------------------------------------------------
        CSO_MAP_OPT(int,                           /* option type */
                NumOfCopies,                        /* option name */
                0,                          /* default value */
                false,                          /* is option mandatory */
                'n',                           /* short option name */
                "numofcopies",                      /* long option name */
                "NUM",                           /* parametr name */
                "it lunches the job by NUM times")   /* option description */
        //----------------------------------------------------------------------
        CSO_MAP_OPT(bool,                           /* option type */
                IgnoreRuntimeFiles,                        /* option name */
                false,                          /* default value */
                false,                          /* is option mandatory */
                'i',                           /* short option name */
                "ignore",                      /* long option name */
                NULL,                           /* parametr name */
                "ignore run-time files")   /* option description */
        //----------------------------------------------------------------------
        CSO_MAP_OPT(bool,                           /* option type */
                AssumeYes,                        /* option name */
                false,                          /* default value */
                false,                          /* is option mandatory */
                'y',                           /* short option name */
                "yes",                      /* long option name */
                NULL,                           /* parametr name */
                "assume yes in the confirmation of the job submission")   /* option description */
        //----------------------------------------------------------------------
        CSO_MAP_OPT(bool,                           /* option type */
                Silent,                        /* option name */
                false,                          /* default value */
                false,                          /* is option mandatory */
                's',                           /* short option name */
                "silent",                      /* long option name */
                NULL,                           /* parametr name */
                "decrease verbosity to minimum")   /* option description */
        //----------------------------------------------------------------------
        CSO_MAP_OPT(bool,                           /* option type */
                ExpertMode,                        /* option name */
                false,                          /* default value */
                false,                          /* is option mandatory */
                'e',                           /* short option name */
                "expert",                      /* long option name */
                NULL,                           /* parametr name */
                "it disables some tests of input parameters allowing injection of unsupported resources directly to the batch server")   /* option description */
        //----------------------------------------------------------------------
        CSO_MAP_OPT(bool,                           /* option type */
                ResubmitMode,                        /* option name */
                false,                          /* default value */
                false,                          /* is option mandatory */
                'r',                           /* short option name */
                "repeat",                      /* long option name */
                NULL,                           /* parametr name */
                "repeat job submission in the case of the batch server error")   /* option description */
        //----------------------------------------------------------------------
        CSO_MAP_OPT(bool,                           /* option type */
                AllowAllPaths,                        /* option name */
                false,                          /* default value */
                false,                          /* is option mandatory */
                'a',                           /* short option name */
                "allowallpaths",                      /* long option name */
                NULL,                           /* parametr name */
                "allow all input job paths")   /* option description */
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
