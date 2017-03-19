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

#include "Jobs.hpp"
#include <ErrorSystem.hpp>
#include <SmallTimeAndDate.hpp>
#include <ABSConfig.hpp>
#include <CommonParser.hpp>
#include <BatchSystems.hpp>
#include <boost/algorithm/string/replace.hpp>

using namespace std;

//------------------------------------------------------------------------------

MAIN_ENTRY(CJobs)

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

CJobs::CJobs(void)
{
}

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

int CJobs::Init(int argc,char* argv[])
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
    vout << "# pjobs (ABS utility) started at " << dt.GetSDateAndTime() << endl;
    vout << "# ==============================================================================" << endl;

    return(SO_CONTINUE);
}

//------------------------------------------------------------------------------

bool CJobs::Run(void)
{
    // init all subsystems
    if( ABSConfig.LoadSystemConfig() == false ){
        ES_ERROR("unable to load ABSConfig config");
        return(false);
    }

    vout << low;
    vout << "#" << endl;
    ABSConfig.PrintBatchServerInfo(vout);
    if( Options.IsOptSearchSet() ){
        std::string str = string(Options.GetOptSearch());
        boost::replace_all(str,"<","<<");
    vout << "# Search expr.  : " << str << endl;
    }
    vout << "#" << endl;
    vout << endl;

    // check if user has valid ticket
    if( ABSConfig.IsUserTicketValid(vout) == false ){
        ES_TRACE_ERROR("user does not have valid ticket");
        return(false);
    }

    // we need ticket here
    if( BatchSystems.Init() == false ){
        ES_ERROR("unable to init torque");
        return(false);
    }

    if( Options.IsOptUserSet() ){
        // overwrite user name
        User.SetUserName(Options.GetOptUser());
    }

    User.InitUser();

    vout << high;
    User.PrintUserInfo(vout);
    vout << endl;
    vout << low;

    if( BatchSystems.GetUserJobs(Jobs,User.GetName()) == false ){
        ES_ERROR("unable to get jobs");
        return(false);
    }

    if( Options.GetOptFinished() ) {
        Jobs.KeepOnlyCompletedJobs();
        Jobs.SortByFinishDateAndTime();
    } else {
        // filter jobs
        if( ! Options.GetOptKeepCompleted() ){
            Jobs.RemoveCompletedJobs();
        }
    }

    if( Options.IsOptSearchSet() ){
        // init search mask
        init_mask();
        if( parse_job_mask(Options.GetOptSearch()) != 0 ){
            free_mask_tree();
            ES_ERROR("unable to set search expression");
            return(false);
        }
        Jobs.KeepJobsByMask(get_expression_tree());
        //print_expression_tree(get_expression_tree());
        free_mask_tree();
    }

    // print info about jobs
    Jobs.PrintBatchInfo(vout,Options.GetOptIncludePath(),Options.GetOptIncludeComment());

    // print batch stat
    vout << endl;
    Jobs.PrintBatchInfoStat(vout);

    return(true);
}

//------------------------------------------------------------------------------

void CJobs::Finalize(void)
{
    CSmallTimeAndDate dt;
    dt.GetActualTimeAndDate();

    vout << high;
    vout << endl;
    vout << "# ==============================================================================" << endl;
    vout << "# pjobs terminated at " << dt.GetSDateAndTime() << endl;
    vout << "# ==============================================================================" << endl;

    vout << low;
    vout << endl;
    if( ErrorSystem.IsError() || Options.GetOptVerbose() ){
        ErrorSystem.PrintErrors(vout);
        vout << endl;
    }
}

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

