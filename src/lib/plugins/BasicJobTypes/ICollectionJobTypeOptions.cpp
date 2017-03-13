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

#include "ICollectionJobTypeOptions.hpp"
#include <ErrorSystem.hpp>

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

CIJobOptions::CIJobOptions(void)
{
    SetShowMiniUsage(true);
    SetAllowProgArgs(true);
}

//------------------------------------------------------------------------------

int CIJobOptions::CheckOptions(void)
{
    if( (IsOptFromSet() == true) && (IsOptFromSet() == false) ){
        if( IsError == false ) fprintf(stderr,"\n");
        fprintf(stderr,"%s: both options --from and --to must be specified together\n",
                (const char*)GetProgramName());
        IsError = true;
        return(SO_OPTS_ERROR);
    }

    if( (IsOptFromSet() == false) && (IsOptFromSet() == true) ){
        if( IsError == false ) fprintf(stderr,"\n");
        fprintf(stderr,"%s: both options --from and --to must be specified together\n",
                (const char*)GetProgramName());
        IsError = true;
        return(SO_OPTS_ERROR);
    }

    return(SO_CONTINUE);
}

//------------------------------------------------------------------------------

int CIJobOptions::FinalizeOptions(void)
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

int CIJobOptions::CheckArguments(void)
{
    if( GetArgNCPUs().IsInt() == false ){
        if( IsError == false ) fprintf(stderr,"\n");
        fprintf(stderr,"%s: number of CPUs must be specified by an integer number\n",
                (const char*)GetProgramName());
        IsError = true;
        return(SO_OPTS_ERROR);
    }

    return(SO_CONTINUE);
}

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================
