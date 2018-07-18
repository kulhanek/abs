#ifndef JobsOptionsH
#define JobsOptionsH
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

class CJobsOptions : public CSimpleOptions {
    public:
        // constructor - tune option setup
        CJobsOptions(void);

 // program name and description -----------------------------------------------
    CSO_PROG_NAME_BEGIN
        "pjobs"
    CSO_PROG_NAME_END

    CSO_PROG_DESC_BEGIN
        "Print info about jobs in the batch systems that are owned by the user.\n"
        "\n"
        "<b>Search expressions:</b>\n"
        "<i>Search tokens:</i>        bs (short name of batch server), st (state), user, id (only numeric part), queue (only name), title, path,\n"
               "                      ncpus, ngpus, nnodes\n"
        "<i>Comparison operators:</i> == (equal), != (not equal), <<, <<=, >, >=\n"
        "<i>Logical operators:</i>    ! (logical not), && (logical and), || (logical or)\n"
        "<i>Braces:</i>               ( )\n"
        "The search token must be always on the left side of comparison operator.\n"
        "On the right side, it can be:\n"
        "   an integer number for <u>id</u>, <u>ncpus</u>, <u>ngpus</u>, and <u>nnodes</u> tokens\n"
        "   a string for <u>bs</u>, <u>st</u>, <u>user</u>, , <u>title</u>, and <u>path</u> tokens\n"
        "String comparisons can employ shell wildcard patterns and == or != comparison operators only."
    CSO_PROG_DESC_END

    CSO_PROG_VERS_BEGIN
        LibBuildVersion_ABS
    CSO_PROG_VERS_END

 // list of all options and arguments ------------------------------------------
    CSO_LIST_BEGIN
        // options ------------------------------
        CSO_OPT(CSmallString,User)
        CSO_OPT(CSmallString,Search)
        CSO_OPT(bool,IncludePath)
        CSO_OPT(bool,IncludeComment)
        CSO_OPT(bool,IncludeOrigin)
        CSO_OPT(bool,KeepHistory)
        CSO_OPT(bool,Moved)
        CSO_OPT(bool,Finished)
        CSO_OPT(bool,Help)
        CSO_OPT(bool,Version)
        CSO_OPT(bool,Verbose)
    CSO_LIST_END

    CSO_MAP_BEGIN
 // description of options -----------------------------------------------------
        CSO_MAP_OPT(CSmallString,                           /* option type */
                    User,                        /* option name */
                    "",                          /* default value */
                    false,                          /* is option mandatory */
                    'u',                           /* short option name */
                    "user",                      /* long option name */
                    "NAME",                           /* parametr name */
                    "alternative user name")   /* option description */
    //----------------------------------------------------------------------
        CSO_MAP_OPT(CSmallString,                           /* option type */
                    Search,                        /* option name */
                    NULL,                          /* default value */
                    false,                          /* is option mandatory */
                    's',                           /* short option name */
                    "search",                      /* long option name */
                    "EXPRESSION",                           /* parametr name */
                    "search for jobs satisfying logical expression")   /* option description */
    //----------------------------------------------------------------------
        CSO_MAP_OPT(bool,                           /* option type */
                    IncludePath,                        /* option name */
                    false,                          /* default value */
                    false,                          /* is option mandatory */
                    'p',                           /* short option name */
                    "path",                      /* long option name */
                    NULL,                           /* parametr name */
                    "print job path if possible")   /* option description */
    //----------------------------------------------------------------------
        CSO_MAP_OPT(bool,                           /* option type */
                    IncludeComment,                        /* option name */
                    false,                          /* default value */
                    false,                          /* is option mandatory */
                    'l',                           /* short option name */
                    "comment",                      /* long option name */
                    NULL,                           /* parametr name */
                    "print job comment if possible")   /* option description */
    //----------------------------------------------------------------------
        CSO_MAP_OPT(bool,                           /* option type */
                    IncludeOrigin,                        /* option name */
                    false,                          /* default value */
                    false,                          /* is option mandatory */
                    'o',                           /* short option name */
                    "origin",                      /* long option name */
                    NULL,                           /* parametr name */
                    "print job origin if the job was moved between servers")   /* option description */
    //----------------------------------------------------------------------
        CSO_MAP_OPT(bool,                           /* option type */
                    KeepHistory,                        /* option name */
                    false,                          /* default value */
                    false,                          /* is option mandatory */
                    'c',                           /* short option name */
                    "completed",                      /* long option name */
                    NULL,                           /* parametr name */
                    "keep all jobs")   /* option description */
    //----------------------------------------------------------------------
        CSO_MAP_OPT(bool,                           /* option type */
                    Finished,                        /* option name */
                    false,                          /* default value */
                    false,                          /* is option mandatory */
                    'f',                           /* short option name */
                    "finished",                      /* long option name */
                    NULL,                           /* parametr name */
                    "show only finished jobs sorted by their termination time")   /* option description */
    //----------------------------------------------------------------------
        CSO_MAP_OPT(bool,                           /* option type */
                    Moved,                        /* option name */
                    false,                          /* default value */
                    false,                          /* is option mandatory */
                    'm',                           /* short option name */
                    "moved",                      /* long option name */
                    NULL,                           /* parametr name */
                    "show only moved jobs")   /* option description */
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
