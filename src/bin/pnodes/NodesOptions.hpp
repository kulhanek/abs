#ifndef NodesOptionsH
#define NodesOptionsH
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

class CNodesOptions : public CSimpleOptions {
    public:
        // constructor - tune option setup
        CNodesOptions(void);

 // program name and description -----------------------------------------------
    CSO_PROG_NAME_BEGIN
        "pnodes"
    CSO_PROG_NAME_END

    CSO_PROG_DESC_BEGIN
        "Print info about computational nodes of the batch systems.\n"
        "\n"
        "<b>Search expressions:</b>\n"
        "<i>Search tokens:</i>        name, props, st (state code), bs (short name of batch server),\n"
               "                      ncpus, nfreecpus, ngpus, nfreegpus, mem, freemem\n"
        "<i>Comparison operators:</i> == (equal), != (not equal), <<, <<=, >, >=\n"
        "<i>Logical operators:</i>    ! (logical not), && (logical and), || (logical or)\n"
        "<i>Braces:</i>               ( )\n"
        "The search token must be always on the left side of comparison operator.\n"
        "On the right side, it can be:\n"
        "   = an integer number for <u>ncpus</u>, <u>nfreecpus</u>, <u>ngpus</u>, <u>nfreegpus</u> tokens\n"
        "   = a string for <u>name</u>, <u>props</u>, <u>st</u>, <u>bs</u>\n"
        "   = a size for <u>mem</u>, and <u>freemem</u> tokens\n"
        "     Size can be in kb, mb, gb, or tb (the unit must be specified).\n"
        "String comparison can use a shell wildcard pattern and can employ  == or != comparison operators only."
    CSO_PROG_DESC_END

    CSO_PROG_VERS_BEGIN
        LibBuildVersion_ABS
    CSO_PROG_VERS_END

 // list of all options and arguments ------------------------------------------
    CSO_LIST_BEGIN
        // options ------------------------------
        CSO_OPT(CSmallString,Groups)
        CSO_OPT(CSmallString,Node)
        CSO_OPT(CSmallString,Search)
        CSO_OPT(bool,DoNotGroup)
        CSO_OPT(bool,PrintGroups)
        CSO_OPT(bool,KeepAll)   
        CSO_OPT(bool,Technical)
        CSO_OPT(bool,PrintNames)
        CSO_OPT(bool,PrintHosts)
        CSO_OPT(bool,Help)
        CSO_OPT(bool,Version)
        CSO_OPT(bool,Verbose)
    CSO_LIST_END

    CSO_MAP_BEGIN
 // description of options -----------------------------------------------------
        CSO_MAP_OPT(CSmallString,                           /* option type */
                    Groups,                        /* option name */
                    "all",                          /* default value */
                    false,                          /* is option mandatory */
                    'g',                           /* short option name */
                    "groups",                      /* long option name */
                    "LIST",                           /* parametr name */
                    "comma separated node groups")   /* option description */                    
        //----------------------------------------------------------------------
        CSO_MAP_OPT(CSmallString,                           /* option type */
                    Node,                        /* option name */
                    NULL,                          /* default value */
                    false,                          /* is option mandatory */
                    'n',                           /* short option name */
                    "node",                      /* long option name */
                    "NAME",                           /* parametr name */
                    "name of node")   /* option description */
        //----------------------------------------------------------------------
        CSO_MAP_OPT(CSmallString,                           /* option type */
                    Search,                        /* option name */
                    NULL,                          /* default value */
                    false,                          /* is option mandatory */
                    's',                           /* short option name */
                    "search",                      /* long option name */
                    "EXPRESSION",                           /* parametr name */
                    "search for nodes satisfying logical expression")   /* option description */
        //----------------------------------------------------------------------
        CSO_MAP_OPT(bool,                           /* option type */
                    DoNotGroup,                        /* option name */
                    false,                          /* default value */
                    false,                          /* is option mandatory */
                    0,                           /* short option name */
                    "donotgroup",                      /* long option name */
                    NULL,                           /* parametr name */
                    "do not group nodes")   /* option description */
        //----------------------------------------------------------------------
        CSO_MAP_OPT(bool,                           /* option type */
                    PrintGroups,                        /* option name */
                    false,                          /* default value */
                    false,                          /* is option mandatory */
                    0,                           /* short option name */
                    "printgroups",                      /* long option name */
                    NULL,                           /* parametr name */
                    "print defined node groups")   /* option description */
        //----------------------------------------------------------------------
        CSO_MAP_OPT(bool,                           /* option type */
                    KeepAll,                        /* option name */
                    false,                          /* default value */
                    false,                          /* is option mandatory */
                    'k',                           /* short option name */
                    "keep",                      /* long option name */
                    NULL,                           /* parametr name */
                    "keep all nodes, it also shows nodes in maintenance state or inaccessible to user")   /* option description */
        //----------------------------------------------------------------------
        CSO_MAP_OPT(bool,                           /* option type */
                    Technical,                        /* option name */
                    false,                          /* default value */
                    false,                          /* is option mandatory */
                    't',                           /* short option name */
                    "technical",                      /* long option name */
                    NULL,                           /* parametr name */
                    "print all nodes in technical format")   /* option description */
        //----------------------------------------------------------------------
        CSO_MAP_OPT(bool,                           /* option type */
                    PrintNames,                        /* option name */
                    false,                          /* default value */
                    false,                          /* is option mandatory */
                    0,                           /* short option name */
                    "names",                      /* long option name */
                    NULL,                           /* parametr name */
                    "print only node names")   /* option description */
        //----------------------------------------------------------------------
        CSO_MAP_OPT(bool,                           /* option type */
                    PrintHosts,                        /* option name */
                    false,                          /* default value */
                    false,                          /* is option mandatory */
                    0,                           /* short option name */
                    "hosts",                      /* long option name */
                    NULL,                           /* parametr name */
                    "print only host names")   /* option description */
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
