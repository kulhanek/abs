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

#include "IJobs.hpp"
#include <ErrorSystem.hpp>
#include <TerminalStr.hpp>

//------------------------------------------------------------------------------

MAIN_ENTRY(CIJobs)

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

CIJobs::CIJobs(void)
{
}

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

int CIJobs::Init(int argc,char* argv[])
{
    // encode program options
    int result = Options.ParseCmdLine(argc,argv);
    return(result);
}

//------------------------------------------------------------------------------

bool CIJobs::Run(void)
{
    if( Job.LoadInfoFile(Options.GetArgInfoFile()) == false ){
        ES_ERROR("unable to load info file");
        return(false);
    }

 // actions --------------------------------------
    if( Options.GetArgProperty() == "number" ){
        int num = Job.GetNumberOfIJobs();
        std::cout << num;
    }
    else {
        if( Options.IsOptSetSet() == false ){
            CSmallString res;
            if( Job.GetIJobProperty(Options.GetOptID(),Options.GetArgProperty(),res) == false ) return(false);
            std::cout << res;
        } else {
            Job.SetIJobProperty(Options.GetOptID(),Options.GetArgProperty(),Options.GetOptSet());
        }
    }

    // save info file -------------------------------
    if( Options.IsOptSetSet() ){
        if( Job.SaveInfoFile(Options.GetArgInfoFile()) == false ){
            ES_ERROR("unable to save info file");
            return(false);
        }
    }

    return(true);
}

//------------------------------------------------------------------------------

void CIJobs::Finalize(void)
{
    if( Options.GetOptVerbose() || ErrorSystem.IsError() ) {
        ErrorSystem.PrintErrors(stderr);
    }
}

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

