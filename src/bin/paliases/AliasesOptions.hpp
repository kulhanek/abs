#ifndef AliasesOptionsH
#define AliasesOptionsH
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

class CAliasesOptions : public CSimpleOptions {
    public:
        // constructor - tune option setup
        CAliasesOptions(void);

 // program name and description -----------------------------------------------
    CSO_PROG_NAME_BEGIN
        "paliases"
    CSO_PROG_NAME_END

    CSO_PROG_DESC_BEGIN
        "Manage resource aliases."
    CSO_PROG_DESC_END

    CSO_PROG_ARGS_SHORT_DESC_BEGIN
        "[action] [arguments]"
    CSO_PROG_ARGS_SHORT_DESC_END

    CSO_PROG_ARGS_LONG_DESC_BEGIN
        "<b>Actions:</b>\n"
        "   <b><cyan>list</cyan></b>\n"
        "          list all aliases (default action)\n\n"
        "   <b><cyan>add <u>alias</u> <u>destination</u> <u>resources</u></cyan></b>\n"
        "          add new <u>alias</u>\n\n"
        "   <b><cyan>remove <u>alias</u></cyan></b>\n"
        "          remove the <u>alias</u>\n"
    CSO_PROG_ARGS_LONG_DESC_END

    CSO_PROG_VERS_BEGIN
        LibBuildVersion_ABS
    CSO_PROG_VERS_END

 // list of all options and arguments ------------------------------------------
    CSO_LIST_BEGIN
        // options ------------------------------
        CSO_OPT(bool,Force)
        CSO_OPT(bool,ExpertMode)
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
                "add an alias even if another exists")   /* option description */
        //----------------------------------------------------------------------
        CSO_MAP_OPT(bool,                           /* option type */
                ExpertMode,                        /* option name */
                false,                          /* default value */
                false,                          /* is option mandatory */
                'e',                           /* short option name */
                "expert",                      /* long option name */
                NULL,                           /* parametr name */
                "it enables expert mode, in which you can define any batch resource")   /* option description */
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

    //! return action name
    const CSmallString& GetArgAction(void) const;

// final operation with options ------------------------------------------------
private:
    CSmallString    Action;

    virtual int CheckOptions(void);
    virtual int FinalizeOptions(void);
    virtual int CheckArguments(void);
    };

//------------------------------------------------------------------------------

#endif
