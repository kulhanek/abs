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

#include "Nodes.hpp"
#include <ErrorSystem.hpp>
#include <SmallTimeAndDate.hpp>
#include <ABSConfig.hpp>
#include <NodeList.hpp>
#include <QueueList.hpp>
#include <CommonParser.hpp>
#include <BatchServers.hpp>
#include <boost/algorithm/string/replace.hpp>

using namespace std;

//------------------------------------------------------------------------------

MAIN_ENTRY(CNodes)

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

CNodes::CNodes(void)
{
    ExtraEndLine = true;
}

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

int CNodes::Init(int argc,char* argv[])
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
    vout << "# pnodes (ABS utility) started at " << dt.GetSDateAndTime() << endl;
    vout << "# ==============================================================================" << endl;

    return(SO_CONTINUE);
}

//------------------------------------------------------------------------------

bool CNodes::Run(void)
{
    SiteController.InitSiteControllerConfig();

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
    if( (Options.GetOptPrintNames() == false) && (Options.GetOptPrintHosts() == false) && (Options.GetOptPrintStat() == false) ){
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
        }
        if( term ) vout << "#" << endl;
    }

    if( Options.GetOptTechnical() ){
        vout << endl;
        BatchServers.PrintNodes(vout);
        return(true);
    }

    if( Options.IsOptNodeSet() ){
        // only single node info
        BatchServers.PrintNode(vout,Options.GetOptNode());
        return(true);
    }

    vout << high;
    User.PrintUserInfo(vout);
    vout << endl;
    vout << low;

    if( Options.GetOptPrintGroups() == true ){
        NodeList.PrintNodeGroupNames(vout);
        return(true);
    }

    std::string jobid;   // epty if not set
    if( Options.IsOptJobSet() == false ){
        // all nodes
        if( BatchServers.GetNodes() == false ){
            ES_ERROR("unable to load nodes");
            return(false);
        }
    } else {
        // only nodes from the job
        CJobPtr job = BatchServers.GetJob(Options.GetOptJob());
        if( job == NULL ){
            ES_TRACE_ERROR("job was not found");
            return(false);
        }
        if( job->GetJobBatchStatus() != EJS_RUNNING ) {
            ES_TRACE_ERROR("job is not running");
            return(false);
        }
        if( job->GetVNodes() == false ){
            ES_ERROR("unable to load job nodes");
            return(false);
        }
        jobid = job->GetBatchJobID();
    }

    if( Options.GetOptKeepAll() == false ) {
        if( BatchServers.GetQueues() == false ){
            ES_ERROR("unable to get queues");
            return(false);
        }

        // get list of queues
        std::vector<std::string>  qlist;

        // keep only queue accessible to the user
        QueueList.RemoveDisabledQueues();
        QueueList.RemoveStoppedQueues();
        QueueList.RemoveInaccesibleQueues(User);
        QueueList.RemoveNonexecutiveQueues();

        vout << high;
        QueueList.PrintInfos(vout);
        vout << low;

        // get list of queues - queue.server
        QueueList.GetQueuesWithServer(qlist);

        // remove unwanted nodes
        NodeList.KeepNodesByQueuesWithServer(qlist);
        NodeList.KeepNodesByOwner(string(User.GetName()));
        NodeList.RemoveDownNodes();
    }

    if( Options.IsOptSearchSet() ){
        // init search mask
        init_mask();
        if( parse_node_mask(Options.GetOptSearch()) != 0 ){
            free_mask_tree();
            ES_ERROR("unable to set search expression");
            return(false);
        }
        NodeList.KeepNodesByMask(get_expression_tree());
        //print_expression_tree(get_expression_tree());
        free_mask_tree();
    }

    if( Options.GetOptDoNotGroup() == true ){
        NodeList.PrepareSingleNodeGroup();
    } else {
        // generate node groups
        NodeList.PrepareNodeGroups();

        if( Options.IsOptGroupsSet() ){
            NodeList.KeepNodeGroups(Options.GetOptGroups());
        }
    }

    NodeList.FinalizeNodeGroups();

    if( (Options.GetOptPrintNames() == false) && (Options.GetOptPrintHosts() == false) && (Options.GetOptPrintStat() == false) ){
        // list individual nodes
        NodeList.PrintInfos(vout,Options.GetOptPrintJobs(),jobid);

        // print final stats
        NodeList.PrintStatistics(vout);
        return(true);
    }
    if( Options.GetOptPrintNames() == true ){
        NodeList.PrintNames(vout);
        return(true);
    }
    if( Options.GetOptPrintHosts() == true ){
        NodeList.PrintHosts(vout);
        return(true);
    }
    if( Options.GetOptPrintStat() == true ){
        NodeList.PrintTechStatistics(vout);
        return(true);
    }

    return(true);
}

//------------------------------------------------------------------------------

void CNodes::Finalize(void)
{
    // were all server accessbible?
    if( ABSConfig.IsUserTicketValid(vout) == true ){
        if( Options.IsOptJobSet() == false ){
            BatchServers.PrintWarningIfUnavailable(vout);
        }
    }

    CSmallTimeAndDate dt;
    dt.GetActualTimeAndDate();

    vout << high;
    vout << endl;
    vout << "# ==============================================================================" << endl;
    vout << "# pnodes terminated at " << dt.GetSDateAndTime() << endl;
    vout << "# ==============================================================================" << endl;

    vout << low;
    if( ErrorSystem.IsError() || Options.GetOptVerbose() ){
        ErrorSystem.PrintErrors(vout);
    }
    if( (Options.GetOptPrintNames() == false) && (Options.GetOptPrintHosts() == false) && (Options.GetOptPrintStat() == false) ){
        if( ExtraEndLine ) vout << endl;
    }
}

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

