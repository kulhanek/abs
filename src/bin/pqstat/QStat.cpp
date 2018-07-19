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
    ExtraEndLine = true;
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
    vout << low;

    if( ABSConfig.IsABSAvailable(vout) == false ){
        ES_TRACE_ERROR("abs not configured or available");
        ExtraEndLine = false;
        return(false);
    }

    // init all subsystems
    if( ABSConfig.LoadSystemConfig() == false ){
        ES_ERROR("unable to load ABSConfig config");
        return(false);
    }

    // user must be initializaed before ABSConfig.IsUserTicketValid()
    User.InitUser();

    // check if user has valid ticket
        if( ABSConfig.IsUserTicketValid(vout) == false ){
        ES_TRACE_ERROR("user does not have valid ticket");
        ExtraEndLine = false;
        return(false);
    }

    bool term = false;
    if( Options.IsOptJobSet() == false ){
        vout << "#" << endl;
        // the job is related to only one server, thus it is not necessary to enumerate all servers
        BatchServers.PrintServerOverview(vout);
        term = true;
    }
    if( Options.IsOptSearchSet() ){
        if( ! term ) vout << "#" << endl;
        std::string str = string(Options.GetOptSearch());
        boost::replace_all(str,"<","<<");
        vout << "# Search expr.  : " << str << endl;
        term = true;
    }
    if( term ) vout << "#" << endl;

    if( Options.GetOptTechnical() ){
        BatchServers.PrintJobs(vout);
        return(true);
    }

    if( Options.IsOptJobSet() ){
        // only single job info
        BatchServers.PrintJob(vout,Options.GetOptJob());
        return(true);
    }

    vout << high;
    User.PrintUserInfo(vout);
    vout << low;

    if( Options.IsOptQueueSet() ){
        CSmallString full_srv,short_srv,queue_name;
        BatchServers.DecodeQueueName(Options.GetOptQueue(),full_srv,short_srv,queue_name);
        CSmallString full_q;
        full_q << queue_name << "@" << short_srv;
        if( BatchServers.GetQueueJobs(full_q,Options.GetOptKeepHistory() || Options.GetOptFinished() || Options.GetOptMoved()) == false ){
            ES_ERROR("unable to get queue jobs");
            return(false);
        }
    } else {
        if( BatchServers.GetAllJobs(Options.GetOptKeepHistory() || Options.GetOptFinished() || Options.GetOptMoved()) == false ){
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

    if( Options.GetOptFinished() ) {
        // show only finished jobs
        JobList.KeepOnlyFinishedJobs();
        JobList.SortByFinishDateAndTime();
    } else if( Options.GetOptMoved() ) {
        JobList.KeepOnlyMovedJobs();
        JobList.SortByBatchSubmitDateAndTime();
    } else {
        // this is important in the multibatch environment
        JobList.SortByBatchSubmitDateAndTime();
    }

    // print info about jobs
    JobList.PrintBatchInfo(vout,Options.GetOptIncludePath(),Options.GetOptIncludeComment(),Options.GetOptIncludeOrigin());

    // print batch stat
    vout << endl;
    JobList.PrintBatchInfoStat(vout);

    // print diagnosis
    BatchServers.PrintServerExecTimes(vout);

    return(true);
}

//------------------------------------------------------------------------------

void CQStat::Finalize(void)
{
    // were all server accessbible?
    if( ABSConfig.IsUserTicketValid(vout) == true ){
        BatchServers.PrintWarningIfUnavailable(vout);
    }

    CSmallTimeAndDate dt;
    dt.GetActualTimeAndDate();

    vout << high;
    vout << endl;
    vout << "# ==============================================================================" << endl;
    vout << "# pqstat terminated at " << dt.GetSDateAndTime() << endl;
    vout << "# ==============================================================================" << endl;

    vout << low;
    if( ExtraEndLine ) vout << endl;
    if( ErrorSystem.IsError() || Options.GetOptVerbose() ){
        ErrorSystem.PrintErrors(vout);
        vout << endl;
    }
}

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

