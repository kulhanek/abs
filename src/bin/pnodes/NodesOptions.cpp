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

#include "NodesOptions.hpp"
#include <ErrorSystem.hpp>

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

CNodesOptions::CNodesOptions(void)
{
    SetShowMiniUsage(true);
    SetAllowProgArgs(true);
}

//------------------------------------------------------------------------------

int CNodesOptions::CheckOptions(void)
{
    if( IsOptNodeSet() ){
        if( IsOptGroupsSet() ){
            if( IsError == false ) fprintf(stderr,"\n");
            fprintf(stderr,"%s: option --node is not compatible with --groups option\n",
                    (const char*)GetProgramName());
            IsError = true;
            return(SO_OPTS_ERROR);
        }
        if( IsOptPrintGroupsSet() ){
            if( IsError == false ) fprintf(stderr,"\n");
            fprintf(stderr,"%s: option --node is not compatible with --printgroups option\n",
                    (const char*)GetProgramName());
            IsError = true;
            return(SO_OPTS_ERROR);
        }
        if( IsOptTechnicalSet() ){
            if( IsError == false ) fprintf(stderr,"\n");
            fprintf(stderr,"%s: option --node is not compatible with --technical option\n",
                    (const char*)GetProgramName());
            IsError = true;
            return(SO_OPTS_ERROR);
        }
        if( IsOptDoNotGroupSet() ){
            if( IsError == false ) fprintf(stderr,"\n");
            fprintf(stderr,"%s: option --node is not compatible with --donotgroup option\n",
                    (const char*)GetProgramName());
            IsError = true;
            return(SO_OPTS_ERROR);
        }
        if( IsOptSearchSet() ){
            if( IsError == false ) fprintf(stderr,"\n");
            fprintf(stderr,"%s: option --node is not compatible with --search option\n",
                    (const char*)GetProgramName());
            IsError = true;
            return(SO_OPTS_ERROR);
        }
        if( IsOptKeepAllSet() ){
            if( IsError == false ) fprintf(stderr,"\n");
            fprintf(stderr,"%s: option --node is not compatible with --keep option\n",
                    (const char*)GetProgramName());
            IsError = true;
            return(SO_OPTS_ERROR);
        }
    }

    if( IsOptTechnicalSet() ){
        if( IsOptGroupsSet() ){
            if( IsError == false ) fprintf(stderr,"\n");
            fprintf(stderr,"%s: option --technical is not compatible with --groups option\n",
                    (const char*)GetProgramName());
            IsError = true;
            return(SO_OPTS_ERROR);
        }
        if( IsOptPrintGroupsSet() ){
            if( IsError == false ) fprintf(stderr,"\n");
            fprintf(stderr,"%s: option --technical is not compatible with --printgroups option\n",
                    (const char*)GetProgramName());
            IsError = true;
            return(SO_OPTS_ERROR);
        }
        if( IsOptDoNotGroupSet() ){
            if( IsError == false ) fprintf(stderr,"\n");
            fprintf(stderr,"%s: option --technical is not compatible with --donotgroup option\n",
                    (const char*)GetProgramName());
            IsError = true;
            return(SO_OPTS_ERROR);
        }
        if( IsOptSearchSet() ){
            if( IsError == false ) fprintf(stderr,"\n");
            fprintf(stderr,"%s: option --technical is not compatible with --search option\n",
                    (const char*)GetProgramName());
            IsError = true;
            return(SO_OPTS_ERROR);
        }
        if( IsOptKeepAllSet() ){
            if( IsError == false ) fprintf(stderr,"\n");
            fprintf(stderr,"%s: option --technical is not compatible with --keep option\n",
                    (const char*)GetProgramName());
            IsError = true;
            return(SO_OPTS_ERROR);
        }
    }

    if( IsOptPrintGroupsSet() ){
        if( IsOptGroupsSet() ){
            if( IsError == false ) fprintf(stderr,"\n");
            fprintf(stderr,"%s: option --technical is not compatible with --groups option\n",
                    (const char*)GetProgramName());
            IsError = true;
            return(SO_OPTS_ERROR);
        }
        if( IsOptDoNotGroupSet() ){
            if( IsError == false ) fprintf(stderr,"\n");
            fprintf(stderr,"%s: option --technical is not compatible with --donotgroup option\n",
                    (const char*)GetProgramName());
            IsError = true;
            return(SO_OPTS_ERROR);
        }
        if( IsOptSearchSet() ){
            if( IsError == false ) fprintf(stderr,"\n");
            fprintf(stderr,"%s: option --technical is not compatible with --search option\n",
                    (const char*)GetProgramName());
            IsError = true;
            return(SO_OPTS_ERROR);
        }
        if( IsOptKeepAllSet() ){
            if( IsError == false ) fprintf(stderr,"\n");
            fprintf(stderr,"%s: option --technical is not compatible with --keep option\n",
                    (const char*)GetProgramName());
            IsError = true;
            return(SO_OPTS_ERROR);
        }
    }

    return(SO_CONTINUE);
}

//------------------------------------------------------------------------------

int CNodesOptions::FinalizeOptions(void)
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

int CNodesOptions::CheckArguments(void)
{
    return(SO_CONTINUE);
}

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================
