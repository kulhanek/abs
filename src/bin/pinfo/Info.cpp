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

#include "Info.hpp"
#include <ErrorSystem.hpp>
#include <SmallTimeAndDate.hpp>
#include <ABSConfig.hpp>
#include <BatchServers.hpp>

using namespace std;

//------------------------------------------------------------------------------

MAIN_ENTRY(CInfo)

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

CInfo::CInfo(void)
{
}

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

int CInfo::Init(int argc,char* argv[])
{
// encode program options, all check procedures are done inside of CABFIntOpts
    int result = Options.ParseCmdLine(argc,argv);

// should we exit or was it error?
    if(result != SO_CONTINUE) return(result);

// attach verbose stream to terminal stream and set desired verbosity level
    vout.Attach(Console);
    if( Options.GetOptVerbose() ) {
        vout.Verbosity(CVerboseStr::high);
    } else {
        vout.Verbosity(CVerboseStr::low);
    }

    CSmallTimeAndDate dt;
    dt.GetActualTimeAndDate();

    vout << high;
    vout << endl;
    vout << "# ==============================================================================" << endl;
    vout << "# pinfo (ABS utility) started at " << dt.GetSDateAndTime() << endl;
    vout << "# ==============================================================================" << endl;

    return(SO_CONTINUE);
}

//------------------------------------------------------------------------------

bool CInfo::Run(void)
{
    // init all subsystems
    if( ABSConfig.LoadSystemConfig() == false ){
        ES_ERROR("unable to load ABSConfig config");
        return(false);
    }

    vout << low;

    // check if user has valid ticket
    if( ABSConfig.IsUserTicketValid(vout) == false ){
        vout << endl;
        ES_TRACE_ERROR("user does not have valid ticket");
        return(false);
    }

    // get list of info files
    if( Options.GetNumberOfProgArgs() > 0 ){
        for( int i=0; i < Options.GetNumberOfProgArgs(); i++ ){
            if( Jobs.AddJob(Options.GetProgArg(i)) == false ){
                vout << endl;
                vout << "<b><red> ERROR: Unable to load job info file '" << Options.GetProgArg(i) << "'!</red></b>" << endl;
                vout << endl;
                return(false);
            }
        }

    } else {
        Jobs.InitByInfoFiles(".",Options.GetOptRecursive());
    }

    if( Jobs.GetNumberOfJobs() == 0 ){
        vout << endl;
        vout << "<b><red> ERROR: No job info files were found!</red></b>" << endl;
        vout << endl;
        return(false);
    }

    // sort them
    Jobs.SortByPrepareDateAndTime();

    // update status of live jobs
    Jobs.UpdateJobStatuses();

    if( Options.GetOptTerminalStatus() == true ){
        Jobs.PrintTerminalJobStatus(vout);
    } else {
        // print final information
        if( Options.GetOptCompact() ){
            Jobs.PrintInfosCompact(vout,Options.GetOptIncludePath(),Options.GetOptIncludeComment());
        } else {
            Jobs.PrintInfos(vout);
        }
        if( Options.GetOptNoStatistics() == false ){
            Jobs.PrintStatistics(vout);
        }
        vout << endl;
    }

    return(true);
}

//------------------------------------------------------------------------------

bool CInfo::Finalize(void)
{
    CSmallTimeAndDate dt;
    dt.GetActualTimeAndDate();

    vout << high;
    vout << endl;
    vout << "# ==============================================================================" << endl;
    vout << "# pinfo terminated at " << dt.GetSDateAndTime() << endl;
    vout << "# ==============================================================================" << endl;

    if( ErrorSystem.IsError() || Options.GetOptVerbose() ){
        vout << low;
        ErrorSystem.PrintErrors(vout);
    }

    vout << endl;

    return(true);
}

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

