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

#include "SubmitOptions.hpp"
#include <ErrorSystem.hpp>

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

CSubmitOptions::CSubmitOptions(void)
{
    SetShowMiniUsage(true);
    SetAllowProgArgs(true);
}

//------------------------------------------------------------------------------

int CSubmitOptions::CheckOptions(void)
{
    if( IsOptNumOfCopiesSet() ){
        if( (GetProgArg(1) == "screen") || (GetProgArg(1) == "gui") ){
            if( IsError == false ) fprintf(stderr,"\n");
            fprintf(stderr,"%s: interactive job (screen, gui) is not compatible with the --numofcopies option\n",
                    (const char*)GetProgramName());
            IsError = true;
            return(SO_OPTS_ERROR);
        }
        if( (GetOptNumOfCopies() < 2) || (GetOptNumOfCopies() > 999) ){
            if( IsError == false ) fprintf(stderr,"\n");
            fprintf(stderr,"%s: number of copies must by in the range of 2-999, but %d is requested\n",
                    (const char*)GetProgramName(),GetOptNumOfCopies());
            IsError = true;
            return(SO_OPTS_ERROR);
        }
    }

    return(SO_CONTINUE);
}

//------------------------------------------------------------------------------

int CSubmitOptions::FinalizeOptions(void)
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

int CSubmitOptions::CheckArguments(void)
{
    return(SO_CONTINUE);
}

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================
