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
    // init all subsystems
    if( ABSConfig.LoadSystemConfig() == false ){
        ES_ERROR("unable to load ABSConfig config");
        return(false);
    }

    vout << low;
    if( Options.GetOptPrintNames() == false ){
        vout << "#" << endl;
        BatchServers.PrintServerOverview(vout);
        if( Options.IsOptSearchSet() ){
            std::string str = string(Options.GetOptSearch());
            boost::replace_all(str,"<","<<");
        vout << "# Search expr.  : " << str << endl;
        }
        vout << "#" << endl;
    }

    // check if user has valid ticket
    if( ABSConfig.IsUserTicketValid(vout) == false ){
        ES_TRACE_ERROR("user does not have valid ticket");
        return(false);
    }

    if( BatchServers.Init() == false ){
        ES_ERROR("unable to init torque");
        return(false);
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

    User.InitUser();

    vout << high;
    User.PrintUserInfo(vout);
    vout << endl;
    vout << low;

    if( Options.GetOptPrintGroups() == true ){
        NodeList.PrintNodeGroupNames(vout);
        return(true);
    }

    if( BatchServers.GetNodes() == false ){
        ES_ERROR("unable to load nodes");
        return(false);
    }

    if( Options.GetOptKeepAll() == false ) {
        if( BatchServers.GetQueues() == false ){
            ES_ERROR("unable to get queues");
            return(false);
        }

        // get list of required properties by any queue
        vector<string>  qprops;

        // keep only queue accessible to the user
        QueueList.RemoveDisabledQueues();
        QueueList.RemoveStoppedQueues();
        QueueList.RemoveInaccesibleQueues(User);
        QueueList.RemoveNonexecutiveQueues();

        // FIXME
                // again but for queues accessible to user only
                QueueList.GetRequiredProperties(qprops);

                // remove unwanted nodes
                NodeList.KeepNodesThatHaveProperty(qprops);
                NodeList.RemoveNodesWithoutProps();

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

    if( Options.GetOptPrintNames() == false ){
        // list individual nodes
        NodeList.PrintInfos(vout);

        // print final stats
        NodeList.PrintStatistics(vout);
    } else {
        NodeList.PrintNames(vout);
    }

    return(true);
}

//------------------------------------------------------------------------------

void CNodes::Finalize(void)
{
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
    if( Options.GetOptPrintNames() == false ){
        vout << endl;
    }
}

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

