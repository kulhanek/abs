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

#include "UpdateInfoFileOptions.hpp"
#include <ErrorSystem.hpp>

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

CUpdateInfoFileOptions::CUpdateInfoFileOptions(void)
{
    SetShowMiniUsage(true);
}

//------------------------------------------------------------------------------

int CUpdateInfoFileOptions::CheckOptions(void)
{
    return(SO_CONTINUE);
}

//------------------------------------------------------------------------------

int CUpdateInfoFileOptions::FinalizeOptions(void)
{
    bool ret_opt = false;

    if( GetOptHelp() == true ){
        PrintUsage();
        ret_opt = true;
    }

    if( GetOptVersion() == true ){
        PrintVersion();
        ret_opt = true;
    }

    if( ret_opt == true ){
        printf("\n");
        return(SO_EXIT);
    }

    return(SO_CONTINUE);
}

//------------------------------------------------------------------------------

int CUpdateInfoFileOptions::CheckArguments(void)
{
    if( GetArgAction() == "start" ) return(SO_CONTINUE);
    if( GetArgAction() == "stop" ) return(SO_CONTINUE);
    if( GetArgAction() == "cliready" ) return(SO_CONTINUE);
    if( GetArgAction() == "guiready" ) return(SO_CONTINUE);
    if( GetArgAction() == "incstage" ) return(SO_CONTINUE);
    if( GetArgAction() == "archive" ) return(SO_CONTINUE);
    if( IsVerbose() ){
        if( IsError == false ) fprintf(stderr,"\n");
        fprintf(stderr,"%s: action '%s' is not supported\n",
                        (const char*)GetProgramName(),(const char*)GetArgAction());
        IsError = true;
    }
    return(SO_OPTS_ERROR);
}

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================
