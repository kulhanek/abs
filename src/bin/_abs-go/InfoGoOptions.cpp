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

#include "InfoGoOptions.hpp"
#include <ErrorSystem.hpp>

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

CInfoGoOptions::CInfoGoOptions(void)
{
    SetShowMiniUsage(true);
    SetAllowProgArgs(true);
}

//------------------------------------------------------------------------------

int CInfoGoOptions::CheckOptions(void)
{
    if( IsOptNoTerminalSet() ){
        if( IsOptProxySet() ){
            if( IsError == false ) fprintf(stderr,"\n");
            fprintf(stderr,"%s: option --noterm is not compatible with --proxy option\n",
                    (const char*)GetProgramName());
            IsError = true;
            return(SO_OPTS_ERROR);
        }
    }
    if( IsOptForceSet() ){
        if( IsOptProxySet() ){
            if( IsError == false ) fprintf(stderr,"\n");
            fprintf(stderr,"%s: option --force is not compatible with --proxy option\n",
                    (const char*)GetProgramName());
            IsError = true;
            return(SO_OPTS_ERROR);
        }
    }
    return(SO_CONTINUE);
}

//------------------------------------------------------------------------------

int CInfoGoOptions::FinalizeOptions(void)
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

int CInfoGoOptions::CheckArguments(void)
{
    if( GetNumberOfProgArgs() > 1 ){
        if( IsError == false ) fprintf(stderr,"\n");
        fprintf(stderr,"%s: only one argument can be specified, but %d are provided\n",
                (const char*)GetProgramName(),GetNumberOfProgArgs());
        IsError = true;
        return(SO_OPTS_ERROR);
    }

    return(SO_CONTINUE);
}

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================
