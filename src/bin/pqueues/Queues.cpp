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

#include "Queues.hpp"
#include <ErrorSystem.hpp>
#include <SmallTimeAndDate.hpp>
#include <ABSConfig.hpp>
#include <User.hpp>
#include <QueueList.hpp>
#include <BatchServers.hpp>

using namespace std;

//------------------------------------------------------------------------------

MAIN_ENTRY(CQueues)

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

CQueues::CQueues(void)
{
    ExtraEndLine = true;
}

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

int CQueues::Init(int argc,char* argv[])
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
    vout << "# pqueues (ABS utility) started at " << dt.GetSDateAndTime() << endl;
    vout << "# ==============================================================================" << endl;

    return(SO_CONTINUE);
}

//------------------------------------------------------------------------------

bool CQueues::Run(void)
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

    vout << "#" << endl;
    BatchServers.PrintServerOverview(vout);
    vout << "#" << endl;

    // user must be initializaed before ABSConfig.IsUserTicketValid()
    User.InitUser();

    // check if user has valid ticket
    if( ABSConfig.IsUserTicketValid(vout) == false ){
        ES_TRACE_ERROR("user does not have valid ticket");
        ExtraEndLine = false;
        return(false);
    }

    if( Options.GetOptTechnical() ){
        vout << endl;
        BatchServers.PrintQueues(vout);
        return(true);
    }

    vout << high;
    User.PrintUserInfo(vout);
    vout << endl;
    vout << low;

    if( BatchServers.GetQueues() == false ){
        ES_ERROR("unable to get queues");
        return(false);
    }

    if( Options.GetOptAll() == false ){
        QueueList.RemoveDisabledQueues();
        QueueList.RemoveStoppedQueues();
        QueueList.RemoveInaccesibleQueues(User);
        QueueList.RemoveNonexecutiveQueues();
    }

    QueueList.SortByName();
    QueueList.PrintInfos(vout);

    return(true);
}

//------------------------------------------------------------------------------

bool CQueues::Finalize(void)
{
    // were all server accessbible?
    BatchServers.PrintWarningIfUnavailable(vout);

    CSmallTimeAndDate dt;
    dt.GetActualTimeAndDate();

    vout << high;
    vout << "# ==============================================================================" << endl;
    vout << "# pqueues terminated at " << dt.GetSDateAndTime() << endl;
    vout << "# ==============================================================================" << endl;

    if( ErrorSystem.IsError() || Options.GetOptVerbose() ){
        vout << low;
        ErrorSystem.PrintErrors(vout);
    }

    vout << low;
    if( ExtraEndLine ) vout << endl;

    return(true);
}

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

