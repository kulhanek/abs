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

#include "Aliases.hpp"
#include <ErrorSystem.hpp>
#include <SmallTimeAndDate.hpp>
#include <ABSConfig.hpp>
#include <QueueList.hpp>
#include <AliasList.hpp>
#include <NodeList.hpp>
#include <BatchServers.hpp>
#include <SiteController.hpp>

using namespace std;

//------------------------------------------------------------------------------

MAIN_ENTRY(CAliases)

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

CAliases::CAliases(void)
{
}

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

int CAliases::Init(int argc,char* argv[])
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
    vout << "# paliases (ABS utility) started at " << dt.GetSDateAndTime() << endl;
    vout << "# ==============================================================================" << endl;

    return(SO_CONTINUE);
}

//------------------------------------------------------------------------------

bool CAliases::Run(void)
{
    SiteController.InitSiteControllerConfig();

    vout << low;
    if( ABSConfig.IsABSAvailable(vout) == false ){
        ES_TRACE_ERROR("abs not configured or available");
        return(false);
    }

    if( Options.GetArgAction() == "list" ){
        return( ListAliases() );
    }
    if( Options.GetArgAction() == "add" ){
        return( AddAlias() );
    }
    if( Options.GetArgAction() == "remove" ){
        return( RemoveAlias() );
    }

    CSmallString error;
    error << "unsupported action " << Options.GetArgAction();
    ES_ERROR(error);

    return(false);
}

//------------------------------------------------------------------------------

bool CAliases::ListAliases(void)
{
    if( ABSConfig.LoadSystemConfig() == false ){
        ES_ERROR("unable to load ABSConfig config");
        return(false);
    }

    vout << low;
    vout << "#" << endl;
    BatchServers.PrintServerOverview(vout);
    vout << "#" << endl;
    vout << endl;

    if( AliasList.LoadConfig() == false ){
        ES_ERROR("unable to load aliases");
        return(false);
    }

    vout << low;
    AliasList.PrintInfos(vout);
    vout << endl;

    return(true);
}

//------------------------------------------------------------------------------

bool CAliases::AddAlias(void)
{
    if( ABSConfig.LoadSystemConfig() == false ){
        ES_ERROR("unable to load ABSConfig config");
        return(false);
    }

    // user must be initializaed before ABSConfig.IsUserTicketValid()
    User.InitUser();

    vout << low;
    // check if user has valid ticket
    if( ABSConfig.IsUserTicketValid(vout) == false ){
        ES_TRACE_ERROR("user does not have valid ticket");
        return(false);
    }

    if( AliasList.LoadConfig() == false ){
        ES_ERROR("unable to load aliases");
        return(false);
    }

    if( AliasList.FindAlias(Options.GetProgArg(1)) != NULL ){
        if( Options.GetOptForce() == false ){
            vout << low;
            vout << endl;
            vout << "<b><red> ERROR: The <u>" << Options.GetProgArg(1) << "</u> alias already exists!</red></b>" << endl;
            vout <<         "        Use --force option to overwrite it!</red></b>" << endl;
            vout << endl;
            return(false);
        }
    }

    if( BatchServers.GetQueues() == false ){
        ES_ERROR("unable to get list of queues");
        return(false);
    }

    QueueList.RemoveDisabledQueues();
    QueueList.RemoveStoppedQueues();
    QueueList.RemoveInaccesibleQueues(User);
    QueueList.RemoveNonexecutiveQueues();
    QueueList.SortByName();

    if( BatchServers.GetNodes() == false ){
        ES_ERROR("unable to load nodes");
        return(false);
    }

    vout << low;
    if( AliasList.AddAlias(vout,Options.GetProgArg(1),Options.GetProgArg(2),
                         Options.GetProgArg(3),Options.GetOptExpertMode()) == false ){
        vout << endl;
        return(false);
    }

    if( AliasList.SaveUserConfig() == false ){
        ES_ERROR("unable to save updated aliases");
        return(false);
    }

    return(true);
}

//------------------------------------------------------------------------------

bool CAliases::RemoveAlias(void)
{
    if( AliasList.LoadConfig() == false ){
        ES_ERROR("unable to load aliases");
        return(false);
    }

    if( AliasList.FindAlias(Options.GetProgArg(1)) == NULL ){
        vout << low;
        vout << endl;
        vout << "<b><red> ERROR: The <u>" << Options.GetProgArg(1) << "</u> alias does not exists!</red></b>" << endl;
        vout << endl;
        return(false);
    }

    AliasList.RemoveAlias(Options.GetProgArg(1));

    if( AliasList.SaveUserConfig() == false ){
        ES_ERROR("unable to save updated aliases");
        return(false);
    }

    return(true);
}

//------------------------------------------------------------------------------

void CAliases::Finalize(void)
{
    CSmallTimeAndDate dt;
    dt.GetActualTimeAndDate();

    vout << high;
    vout << "# ==============================================================================" << endl;
    vout << "# paliases terminated at " << dt.GetSDateAndTime() << endl;
    vout << "# ==============================================================================" << endl;

    if( ErrorSystem.IsError() || Options.GetOptVerbose() ){
        vout << low;
        ErrorSystem.PrintErrors(vout);
    }
    vout << endl;
}

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

