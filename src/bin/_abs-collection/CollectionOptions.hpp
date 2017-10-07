#ifndef CollectionOptionsH
#define CollectionOptionsH
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

class CCollectionOptions : public CSimpleOptions {
    public:
        // constructor - tune option setup
        CCollectionOptions(void);

 // program name and description -----------------------------------------------
    CSO_PROG_NAME_BEGIN
        "pcollection"
    CSO_PROG_NAME_END

    CSO_PROG_DESC_BEGIN
        "Manage job collections."
    CSO_PROG_DESC_END

    CSO_PROG_ARGS_SHORT_DESC_BEGIN
        "collname action [arg1 [arg2] ..]"
    CSO_PROG_ARGS_SHORT_DESC_END

    CSO_PROG_ARGS_LONG_DESC_BEGIN
        "<b>Actions:</b>\n"
        "   <b><cyan>info</cyan></b>        list jobs in collection (default action)\n"
        "   <b><cyan>stat</cyan></b>        print all collection job statistics\n"
        "   <b><cyan>create</cyan></b>      create new collection <u>collname</u>\n"
        "   <b><cyan>open</cyan></b>        open existing collection <u>collname</u> if collection does not exist it is created\n"
        "   <b><cyan>close</cyan></b>       close <u>collname</u> collection\n"
        "   <b><cyan>prepare</cyan></b> destination job [resources]\n"
        "               perapre all <u>job</u> and add them to <u>collname</u> collection\n"
        "   <b><cyan>addjob</cyan></b> path\n"
        "               add job with <u>path</u> to <u>collname</u> collection\n"
        "   <b><cyan>rmjob</cyan></b> jobcid\n"
        "               remove job with <u>jobcid</u> from <u>collname</u>\n"
        "   <b><cyan>submit</cyan></b>      submit all jobs in the collection <u>collname</u>\n"
        "   <b><cyan>kill</cyan></b>        kill all jobs in the collection <u>collname</u>\n"
    CSO_PROG_ARGS_LONG_DESC_END

    CSO_PROG_VERS_BEGIN
        LibBuildVersion_ABS
    CSO_PROG_VERS_END

 // list of all options and arguments ------------------------------------------
    CSO_LIST_BEGIN
        // options ------------------------------
        CSO_OPT(bool,AssumeYes)
        CSO_OPT(bool,Compact)
        CSO_OPT(bool,IncludePath)
        CSO_OPT(bool,IncludeComment)
        CSO_OPT(bool,Help)
        CSO_OPT(bool,Version)
        CSO_OPT(bool,Verbose)
    CSO_LIST_END

    CSO_MAP_BEGIN
 // description of options -----------------------------------------------------
        CSO_MAP_OPT(bool,                           /* option type */
                AssumeYes,                        /* option name */
                false,                          /* default value */
                false,                          /* is option mandatory */
                'y',                           /* short option name */
                "yes",                      /* long option name */
                NULL,                           /* parametr name */
                "assume yes")   /* option description */
    //----------------------------------------------------------------------
        CSO_MAP_OPT(bool,                           /* option type */
                Compact,                        /* option name */
                false,                          /* default value */
                false,                          /* is option mandatory */
                'c',                           /* short option name */
                "compact",                      /* long option name */
                NULL,                           /* parametr name */
                "print information in compact mode")   /* option description */
    //----------------------------------------------------------------------
        CSO_MAP_OPT(bool,                           /* option type */
                IncludePath,                        /* option name */
                false,                          /* default value */
                false,                          /* is option mandatory */
                'p',                           /* short option name */
                "path",                      /* long option name */
                NULL,                           /* parametr name */
                "print job path in compact mode")   /* option description */
    //----------------------------------------------------------------------
        CSO_MAP_OPT(bool,                           /* option type */
                IncludeComment,                        /* option name */
                false,                          /* default value */
                false,                          /* is option mandatory */
                'l',                           /* short option name */
                "comment",                      /* long option name */
                NULL,                           /* parametr name */
                "print job comment in compact mode")   /* option description */
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
