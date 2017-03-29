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

#include "QStat.hpp"
#include <ErrorSystem.hpp>
#include <SmallTimeAndDate.hpp>
#include <ABSConfig.hpp>
#include <BatchServers.hpp>
#include <CommonParser.hpp>
#include <boost/algorithm/string/replace.hpp>

using namespace std;

//------------------------------------------------------------------------------

MAIN_ENTRY(CQStat)

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

CQStat::CQStat(void)
{
}

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

int CQStat::Init(int argc,char* argv[])
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
    vout << "# pqstat (ABS utility) started at " << dt.GetSDateAndTime() << endl;
    vout << "# ==============================================================================" << endl;

    return(SO_CONTINUE);
}

//------------------------------------------------------------------------------

bool CQStat::Run(void)
{
    // init all subsystems
    if( ABSConfig.LoadSystemConfig() == false ){
        ES_ERROR("unable to load ABSConfig config");
        return(false);
    }

    vout << low;
    vout << "#" << endl;
    BatchServers.PrintServerOverview(vout);
    if( Options.IsOptSearchSet() ){
        std::string str = string(Options.GetOptSearch());
        boost::replace_all(str,"<","<<");
    vout << "# Search expr.  : " << str << endl;
    }
    vout << "#" << endl;

    // check if user has valid ticket
    if( ABSConfig.IsUserTicketValid(vout) == false ){
        ES_TRACE_ERROR("user does not have valid ticket");
        return(false);
    }

    if( Options.GetOptTechnical() ){
        BatchServers.PrintJobs(vout);
        return(true);
    }

    if( Options.IsOptJobSet() ){
        // only single node info
        BatchServers.PrintJob(vout,Options.GetOptJob());
        return(true);
    }

    User.InitUser();

    vout << high;
    User.PrintUserInfo(vout);
    vout << low;

    if( Options.IsOptQueueSet() ){
        if( BatchServers.GetQueueJobs(Options.GetOptQueue(),Options.GetOptKeepCompleted()) == false ){
            ES_ERROR("unable to get queue jobs");
            return(false);
        }
    } else {
        if( BatchServers.GetAllJobs(Options.GetOptKeepCompleted()) == false ){
            ES_ERROR("unable to get all jobs");
            return(false);
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
        JobList.KeepJobsByMask(get_expression_tree());
        //print_expression_tree(get_expression_tree());
        free_mask_tree();
    }

    // print info about jobs
    JobList.PrintBatchInfo(vout,Options.GetOptIncludePath(),Options.GetOptIncludeComment());

    // print batch stat
    vout << endl;
    JobList.PrintBatchInfoStat(vout);

    return(true);
}

//------------------------------------------------------------------------------

void CQStat::Finalize(void)
{
    CSmallTimeAndDate dt;
    dt.GetActualTimeAndDate();

    vout << high;
    vout << endl;
    vout << "# ==============================================================================" << endl;
    vout << "# pqstat terminated at " << dt.GetSDateAndTime() << endl;
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

