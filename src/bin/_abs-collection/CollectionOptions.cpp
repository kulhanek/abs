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

#include "CollectionOptions.hpp"
#include <ErrorSystem.hpp>

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

CCollectionOptions::CCollectionOptions(void)
{
    SetShowMiniUsage(true);
    SetAllowProgArgs(true);
}

//------------------------------------------------------------------------------

int CCollectionOptions::CheckOptions(void)
{
    return(SO_CONTINUE);
}

//------------------------------------------------------------------------------

int CCollectionOptions::FinalizeOptions(void)
{
    bool ret_opt = false;

    if( GetOptHelp() == true ){
        PrintUsage(stderr);
        ret_opt = true;
    }

    if( GetOptVersion() == true ){
        PrintVersion(stderr);
        ret_opt = true;
    }

    if( ret_opt == true ){
        fprintf(stderr,"\n");
        return(SO_EXIT);
    }

    return(SO_CONTINUE);
}

//------------------------------------------------------------------------------

int CCollectionOptions::CheckArguments(void)
{
    if( GetNumberOfArgs() < 1 ){
        if( IsVerbose() ) {
            if( IsError == false ) fprintf(stderr,"\n");
            fprintf(stderr,"%s: wrong number of arguments, at least one is required\n",
                    (const char*)GetProgramName());
            IsError = true;
        }
        return(SO_OPTS_ERROR);
    }

    if( GetNumberOfArgs() == 1 ) return(SO_CONTINUE); // info action

    if( (GetNumberOfArgs() == 2) &&
        (GetProgArg(1) != "info") &&
        (GetProgArg(1) != "open") &&
        (GetProgArg(1) != "create") &&
        (GetProgArg(1) != "close") &&
        (GetProgArg(1) != "submit") &&
        (GetProgArg(1) != "kill")){
        if( IsVerbose() ) {
            if( IsError == false ) fprintf(stderr,"\n");
            fprintf(stderr,"%s: wrong number of arguments, only two arguments for action %s\n",
                    (const char*)GetProgramName(), (const char*)GetProgArg(1));
            IsError = true;
        }
        return(SO_OPTS_ERROR);
    }

    if( (GetNumberOfArgs() == 3) &&
        (GetProgArg(1) != "rmjob") ){
        if( IsVerbose() ) {
            if( IsError == false ) fprintf(stderr,"\n");
            fprintf(stderr,"%s: wrong number of arguments, only tree arguments for action %s\n",
                    (const char*)GetProgramName(), (const char*)GetProgArg(1));
            IsError = true;
        }
        return(SO_OPTS_ERROR);
    }

    if( (GetNumberOfArgs() < 4) &&
        (GetProgArg(1) != "prepare") ){
        if( IsVerbose() ) {
            if( IsError == false ) fprintf(stderr,"\n");
            fprintf(stderr,"%s: wrong number of arguments, at least four arguments for action %s\n",
                    (const char*)GetProgramName(), (const char*)GetProgArg(1));
            IsError = true;
        }
        return(SO_OPTS_ERROR);
    }

    return(SO_CONTINUE);
}

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================
