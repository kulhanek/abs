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

#include <NodeList.hpp>
#include <SimpleIterator.hpp>
#include <iomanip>
#include <XMLElement.hpp>
#include <fnmatch.h>
#include <cctype>
#include <string.h>
#include <boost/algorithm/string/split.hpp>
#include <boost/algorithm/string/classification.hpp>
#include <CommonParser.hpp>
#include <ErrorSystem.hpp>
#include <fnmatch.h>
#include <ABSConfig.hpp>
#include <ResourceValue.hpp>
#include <set>

//------------------------------------------------------------------------------

using namespace std;
using namespace boost;
using namespace boost::algorithm;

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

CNodeList::CNodeList(void)
{

}

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

void CNodeList::PrintInfos(std::ostream& sout,bool include_jobs,bool include_queues,const std::string& jobid)
{
    CTerminal   term;
    int ncolumns = term.GetNumberOfColumns();

    list<CNodeGroupPtr>::iterator git = NodeGroups.begin();
    list<CNodeGroupPtr>::iterator get = NodeGroups.end();

    while( git != get ){
        CNodeGroupPtr p_group = *git;

        sout << endl;
        sout << "# Node Group   : " << p_group->GroupName << endl;
        sout << "# --------------------------------------------------------------------------------------------------" << endl;
        set<string>::iterator pit = p_group->CommonProps.begin();
        set<string>::iterator pet = p_group->CommonProps.end();

        bool first = true;
        bool leading = true;
        int  len = 0;
        while( pit != pet ){
            if( len >= 80 ){
                len = 0;
                first = true;
                sout << endl;
            }
            if( first && leading ){
                sout << "# Common properties:";
                len += 20;
                first = false;
                leading = false;
            }
            if( first ){
                sout << "#";
                len += 1;
                first = false;
            }
            sout << " " << *pit;
            len += (*pit).size() + 1;
            pit++;
        }
        if( first == false ) sout << endl;
        if( p_group->CommonProps.size() > 0 ){
        sout << "# --------------------------------------------------------------------------------------------------" << endl;
        }
        sout << "#  Node name                S   CPUs  CPU Memory  GPUs  GPUMem Local Share  SSD  Extra properties   " << endl;
        sout << "# ------------------------- - ---/--- -----/----- --/-- ------ ----- ----- ----- -------------------" << endl;

        list<CNodePtr>::iterator it = p_group->SortedNodes.begin();
        list<CNodePtr>::iterator et = p_group->SortedNodes.end();

        int tot_cpus = 0;
        int free_cpus = 0;
        int tot_gpus = 0;
        int free_gpus = 0;
        int nnodes = 0;

        while( it != et ){
            CNodePtr p_node = *it;
            p_node->PrintLineInfo(sout,p_group->CommonProps,ncolumns);
            if( include_jobs ){
                p_node->PrintJobsInfo(sout,jobid);
            }
            if( include_queues ){
                p_node->PrintQueuesInfo(sout);
            }

            tot_cpus += p_node->GetNumOfCPUs();
            free_cpus += p_node->GetNumOfFreeCPUs();
            tot_gpus += p_node->GetNumOfGPUs();
            free_gpus += p_node->GetNumOfFreeGPUs();
            nnodes++;
            it++;
        }

        sout << "# --------------------------------------------------------------------------------------------------" << endl;
        sout << "# Total number of nodes: " << right << setw(5) << nnodes << endl;
        sout << "# Total number of CPUs : " << right << setw(5) << tot_cpus;
        sout << "    Total number of GPUs : " << right << setw(5) << tot_gpus << endl;
        sout << "# Free CPUs            : " << right << setw(5) << free_cpus;
        sout << "    Free GPUs            : " << right << setw(5) << free_gpus << endl;

        git++;
    }
}

//------------------------------------------------------------------------------

void CNodeList::PrintStatistics(std::ostream& sout)
{
    if( (NodeGroups.size() == 1) && (AllProps.size() == 0) ){
        return;
    }

    int tot_cpus = 0;
    int free_cpus = 0;
    int tot_gpus = 0;
    int free_gpus = 0;
    int nnodes = 0;

    set<CNodePtr>::iterator it = ReasonableNodes.begin();
    set<CNodePtr>::iterator et = ReasonableNodes.end();

    while( it != et ){
        CNodePtr p_node = *it;
        tot_cpus += p_node->GetNumOfCPUs();
        free_cpus += p_node->GetNumOfFreeCPUs();
        tot_gpus += p_node->GetNumOfGPUs();
        free_gpus += p_node->GetNumOfFreeGPUs();
        nnodes++;
        it++;
    }

    // print list of unique properties
    if( AllProps.size() > 0 ){
        sout << endl;
        sout << "# All properties" << endl;
        sout << "# ---------------------------------------------------------------------------------------------" << endl;

        set<string>::iterator pit = AllProps.begin();
        set<string>::iterator pet = AllProps.end();

        bool first = true;
        int  len = 0;
        while( pit != pet ){
            if( len >= 80 ){
                len = 0;
                first = true;
                sout << endl;
            }
            if( first ){
                sout << "#";
                len += 1;
                first = false;
            }
            sout << " " << *pit;
            len += (*pit).size() + 1;
            pit++;
        }
        if( first == false ) sout << endl;
    }

    sout << "# ---------------------------------------------------------------------------------------------" << endl;
    sout << "# Total number of nodes: " << right << setw(5) << nnodes << endl;
    sout << "# Total number of CPUs : " << right << setw(5) << tot_cpus;
    sout << "    Total number of GPUs : " << right << setw(5) << tot_gpus << endl;
    sout << "# Free CPUs            : " << right << setw(5) << free_cpus;
    sout << "    Free GPUs            : " << right << setw(5) << free_gpus << endl;
}

//------------------------------------------------------------------------------

void CNodeList::PrintTechStatistics(std::ostream& sout)
{
    int tot_cpus = 0;
    int free_cpus = 0;
    int tot_gpus = 0;
    int free_gpus = 0;
    int nnodes = 0;

    set<CNodePtr>::iterator it = ReasonableNodes.begin();
    set<CNodePtr>::iterator et = ReasonableNodes.end();

    while( it != et ){
        CNodePtr p_node = *it;
        tot_cpus += p_node->GetNumOfCPUs();
        free_cpus += p_node->GetNumOfFreeCPUs();
        tot_gpus += p_node->GetNumOfGPUs();
        free_gpus += p_node->GetNumOfFreeGPUs();
        nnodes++;
        it++;
    }

    sout << "nnodes    " <<  nnodes << endl;
    sout << "ntotcpus  " <<  tot_cpus << endl;
    sout << "ntotgpus  " <<  tot_gpus << endl;
    sout << "nfreecpus " <<  free_cpus << endl;
    sout << "nfreegpus " <<  free_gpus << endl;
}

//------------------------------------------------------------------------------

void CNodeList::PrintNames(std::ostream& sout)
{
    list<CNodeGroupPtr>::iterator git = NodeGroups.begin();
    list<CNodeGroupPtr>::iterator get = NodeGroups.end();

    while( git != get ){
        CNodeGroupPtr p_group = *git;

        list<CNodePtr>::iterator it = p_group->SortedNodes.begin();
        list<CNodePtr>::iterator et = p_group->SortedNodes.end();

        while( it != et ){
            CNodePtr p_node = *it;
            sout << p_node->GetName() << endl;
            it++;
        }

        git++;
    }
}

//------------------------------------------------------------------------------

void CNodeList::PrintHosts(std::ostream& sout)
{
    set<string>         hosts;

    list<CNodeGroupPtr>::iterator git = NodeGroups.begin();
    list<CNodeGroupPtr>::iterator get = NodeGroups.end();

    while( git != get ){
        CNodeGroupPtr p_group = *git;

        list<CNodePtr>::iterator it = p_group->SortedNodes.begin();
        list<CNodePtr>::iterator et = p_group->SortedNodes.end();

        while( it != et ){
            CNodePtr p_node = *it;
            hosts.insert(string(p_node->GetHost()));
            it++;
        }

        git++;
    }

    set<string>::iterator hit = hosts.begin();
    set<string>::iterator het = hosts.end();

    while( hit != het ){
        sout << *hit << endl;
        hit++;
    }
}

//------------------------------------------------------------------------------

void CNodeList::PrintNodeGroupNames(std::ostream& sout)
{
    sout << endl;
    sout << "# Node group name" << endl;
    sout << "# --------------------------------------" << endl;
//    CXMLElement* p_ele = ABSConfig.GetNodeGroupConfig();
//    if( p_ele != NULL ){
//        CXMLElement* p_gele = p_ele->GetFirstChildElement("group");
//        while( p_gele != NULL ){
//            CSmallString gname;
//            p_gele->GetAttribute("name",gname);
//            sout << gname << endl;
//            p_gele = p_gele->GetNextSiblingElement("group");
//        }
//    }

    for(CNodeGroupPtr grp : NodeGroups){
        sout << grp->GroupName << endl;
    }
}

//------------------------------------------------------------------------------

void CNodeList::PrepareNodeGroups(void)
{
    CXMLElement* p_ele = ABSConfig.GetNodeGroupConfig();
    if( p_ele == NULL ){
        // all nodes into one big group
        CNodeGroupPtr p_group = CNodeGroupPtr(new CNodeGroup);
        p_group->GroupName = "all nodes";

        list<CNodePtr>::iterator it = begin();
        list<CNodePtr>::iterator ie = end();
        while( it != ie ){
            p_group->insert(*it);
            it++;
        }
        NodeGroups.push_back(p_group);
    } else {
        bool autogroup = false;
        p_ele->GetAttribute("autogroup",autogroup);
        int  minsize = 3;
        p_ele->GetAttribute("minsize",minsize);
        if( autogroup ){
            AutoGroups(minsize);
        }
        PrepareNodeGroups(p_ele);
    }
}

//------------------------------------------------------------------------------

void CNodeList::PrepareNodeGroups(CXMLElement* p_ele)
{
    if( p_ele == NULL ) return;
    CXMLElement* p_gele = p_ele->GetFirstChildElement("group");
    while( p_gele != NULL ){
        CSmallString gname;
        p_gele->GetAttribute("name",gname);

        CNodeGroupPtr p_group = CNodeGroupPtr(new CNodeGroup);
        p_group->GroupName = gname;

        list<CNodePtr>::iterator it = begin();
        list<CNodePtr>::iterator ie = end();
        while( it != ie ){
            CNodePtr p_node = *it;

            CXMLElement* p_filter = p_gele->GetFirstChildElement("filter");
            while( p_filter != NULL ){
                CSmallString filter;
                if( p_filter->GetAttribute("byname",filter) == true ){
                    if( fnmatch(filter,p_node->GetName(),0) == 0 ){
                        p_group->insert(p_node);
                    }
                } else if( p_filter->GetAttribute("byprop",filter) == true ){
                    if( p_node->HasProperty(string(filter)) ){
                        p_group->insert(p_node);
                    }
                }
                p_filter = p_filter->GetNextSiblingElement("filter");
            }

            it++;
        }
        if( p_group->size() > 0 ) {
            NodeGroups.push_back(p_group);
        }
        p_gele = p_gele->GetNextSiblingElement("group");
    }
}

//------------------------------------------------------------------------------

void CNodeList::AutoGroups(unsigned int minsize)
{
    // make copy of nodes
    list<CNodePtr> lnodes;
    lnodes.insert(lnodes.begin(),begin(),end());

    list<CNodePtr>::iterator it = lnodes.begin();
    list<CNodePtr>::iterator ie = lnodes.end();

    CNodeGroupPtr p_common_group = CNodeGroupPtr(new CNodeGroup);
    p_common_group->GroupName = "various nodes";

    while( it != ie ){
        CNodePtr p_node = *it;
        it++;

        std::string     s1 = string(p_node->GetName());
        CSmallString    shortname = p_node->GetShortServerName();
        std::string     n1 = s1;
        std::size_t found = s1.find_first_of("0123456789");
        if( (found != string::npos) && (found > 0) ){
            n1 = s1.substr(0,found);
        }

        CNodeGroupPtr p_group = CNodeGroupPtr(new CNodeGroup);
        p_group->GroupName = n1;
        p_group->insert(p_node);

        // assembly filter
        CSmallString filter = n1;
        filter += "*";

        list<CNodePtr>::iterator git = it;
        list<CNodePtr>::iterator gie = lnodes.end();

        // remove already added node
        lnodes.remove(p_node);

        while( git != gie ){
            CNodePtr p_gnode = *git;
            git++;
            if( (p_gnode->GetShortServerName() == shortname) && (fnmatch(filter,p_gnode->GetName(),0) == 0) ){
                p_group->insert(p_gnode);
                lnodes.remove(p_gnode);
            }
        }
        if( p_group->size() >= minsize ) {
            NodeGroups.push_back(p_group);
        } else {
            p_common_group->insert(p_group->begin(),p_group->end());
        }

        // start over
        it = lnodes.begin();
    }

    // insert common group
    if( p_common_group->size() > 0 ) {
        NodeGroups.push_front(p_common_group);
    }
}

//------------------------------------------------------------------------------

void CNodeList::PrepareSingleNodeGroup(void)
{
    // all nodes into one big group
    CNodeGroupPtr p_group = CNodeGroupPtr(new CNodeGroup);
    p_group->GroupName = "all nodes";

    list<CNodePtr>::iterator it = begin();
    list<CNodePtr>::iterator ie = end();
    while( it != ie ){
        p_group->insert(*it);
        it++;
    }
    NodeGroups.push_back(p_group);
}

//------------------------------------------------------------------------------

void CNodeList::FinalizeNodeGroups(void)
{
    ReasonableNodes.clear();
    AllProps.clear();

    // post process groups
    list<CNodeGroupPtr>::iterator git = NodeGroups.begin();
    list<CNodeGroupPtr>::iterator get = NodeGroups.end();

    while( git != get ){
        CNodeGroupPtr p_group = *git;
        p_group->MakeSortedNodeList();
        p_group->GenerateCommonProperties();
        ReasonableNodes.insert(p_group->begin(),p_group->end());
        git++;
    }

    set<CNodePtr>::iterator rit = ReasonableNodes.begin();
    set<CNodePtr>::iterator ret = ReasonableNodes.end();

    while( rit != ret ){
        CNodePtr p_node = *rit;
        AllProps.insert(p_node->GetAllProps().begin(),p_node->GetAllProps().end());
        rit++;
    }
}

//------------------------------------------------------------------------------

void CNodeList::KeepNodesByQueuesWithServer(const std::vector<std::string>& qlist)
{
    list<CNodePtr>::iterator it = begin();
    list<CNodePtr>::iterator et = end();

    while( it != et ){
        CNodePtr p_node = *it;
        if( p_node->IsInAnyQueueWithServer(qlist) == false ){
            it = erase(it);
        } else {
            it++;
        }
    }
}

//------------------------------------------------------------------------------

void CNodeList::KeepNodesByOwner(const std::string& user)
{
    list<CNodePtr>::iterator it = begin();
    list<CNodePtr>::iterator et = end();

    while( it != et ){
        CNodePtr p_node = *it;
        if( p_node->IsInOwnerList(user) == false ){
            it = erase(it);
        } else {
            it++;
        }
    }
}

//------------------------------------------------------------------------------

void CNodeList::KeepNodesByMask(struct SExpression* p_mask)
{
    list<CNodePtr>::iterator it = begin();
    list<CNodePtr>::iterator et = end();

    while( it != et ){
        CNodePtr p_node = *it;
        if( IsNodeSelected(p_node,p_mask) == false ){
            it = erase(it);
        } else {
            it++;
        }
    }
}

//------------------------------------------------------------------------------

bool CNodeList::IsNodeSelected(CNodePtr p_node,struct SExpression* p_expr)
{
    if(p_expr->Selection != NULL) {
        return( IsNodeSelected(p_node,p_expr->Selection) );
    } else {
        bool left;
        bool right;
        switch(p_expr->Operator) {
            case O_AND:
                left = IsNodeSelected(p_node,p_expr->LeftExpression);
                right = IsNodeSelected(p_node,p_expr->RightExpression);
                return( left && right );
            case O_OR:
                left = IsNodeSelected(p_node,p_expr->LeftExpression);
                right = IsNodeSelected(p_node,p_expr->RightExpression);
                return( left || right );
            case O_NOT:
                right = IsNodeSelected(p_node,p_expr->RightExpression);
                return( ! right );

            default:
                ES_ERROR("<- unknown operator");
                return(false);
        };
    }
}

//------------------------------------------------------------------------------

bool CNodeList::IsNodeSelected(CNodePtr p_node,struct SSelection* p_sel)
{
    switch(p_sel->Type){
        case T_NCPUS:
            switch(p_sel->Operator){
                case O_LT:
                    return( p_node->GetNumOfCPUs() < p_sel->IValue );
                case O_LE:
                    return( p_node->GetNumOfCPUs() <= p_sel->IValue );
                case O_GT:
                    return( p_node->GetNumOfCPUs() > p_sel->IValue );
                case O_GE:
                    return( p_node->GetNumOfCPUs() >= p_sel->IValue );
                case O_EQ:
                    return( p_node->GetNumOfCPUs() == p_sel->IValue );
                case O_NE:
                    return( p_node->GetNumOfCPUs() != p_sel->IValue );
                default:
                    ES_ERROR("<- unknown selection operator");
                    return(false);
            }
        case T_NFREECPUS:
            switch(p_sel->Operator){
                case O_LT:
                    return( p_node->GetNumOfFreeCPUs() < p_sel->IValue );
                case O_LE:
                    return( p_node->GetNumOfFreeCPUs() <= p_sel->IValue );
                case O_GT:
                    return( p_node->GetNumOfFreeCPUs() > p_sel->IValue );
                case O_GE:
                    return( p_node->GetNumOfFreeCPUs() >= p_sel->IValue );
                case O_EQ:
                    return( p_node->GetNumOfFreeCPUs() == p_sel->IValue );
                case O_NE:
                    return( p_node->GetNumOfFreeCPUs() != p_sel->IValue );
                default:
                    ES_ERROR("<- unknown selection operator");
                    return(false);
            }
        case T_NGPUS:
            switch(p_sel->Operator){
                case O_LT:
                    return( p_node->GetNumOfGPUs() < p_sel->IValue );
                case O_LE:
                    return( p_node->GetNumOfGPUs() <= p_sel->IValue );
                case O_GT:
                    return( p_node->GetNumOfGPUs() > p_sel->IValue );
                case O_GE:
                    return( p_node->GetNumOfGPUs() >= p_sel->IValue );
                case O_EQ:
                    return( p_node->GetNumOfGPUs() == p_sel->IValue );
                case O_NE:
                    return( p_node->GetNumOfGPUs() != p_sel->IValue );
                default:
                    ES_ERROR("<- unknown selection operator");
                    return(false);
            }
        case T_NFREEGPUS:
            switch(p_sel->Operator){
                case O_LT:
                    return( p_node->GetNumOfFreeGPUs() < p_sel->IValue );
                case O_LE:
                    return( p_node->GetNumOfFreeGPUs() <= p_sel->IValue );
                case O_GT:
                    return( p_node->GetNumOfFreeGPUs() > p_sel->IValue );
                case O_GE:
                    return( p_node->GetNumOfFreeGPUs() >= p_sel->IValue );
                case O_EQ:
                    return( p_node->GetNumOfFreeGPUs() == p_sel->IValue );
                case O_NE:
                    return( p_node->GetNumOfFreeGPUs() != p_sel->IValue );
                default:
                    ES_ERROR("<- unknown selection operator");
                    return(false);
            }
        case T_MEM:
            switch(p_sel->Operator){
                case O_LT:
                    return( p_node->GetCPUMemory() < CResourceValue::GetSize(p_sel->SValue) );
                case O_LE:
                    return( p_node->GetCPUMemory() <= CResourceValue::GetSize(p_sel->SValue) );
                case O_GT:
                    return( p_node->GetCPUMemory() > CResourceValue::GetSize(p_sel->SValue) );
                case O_GE:
                    return( p_node->GetCPUMemory() >= CResourceValue::GetSize(p_sel->SValue) );
                case O_EQ:
                    return( p_node->GetCPUMemory() == CResourceValue::GetSize(p_sel->SValue) );
                case O_NE:
                    return( p_node->GetCPUMemory() != CResourceValue::GetSize(p_sel->SValue) );
                default:
                    ES_ERROR("<- unknown selection operator");
                    return(false);
            }
        case T_FREEMEM:
            switch(p_sel->Operator){
                case O_LT:
                    return( p_node->GetFreeCPUMemory() < CResourceValue::GetSize(p_sel->SValue) );
                case O_LE:
                    return( p_node->GetFreeCPUMemory() <= CResourceValue::GetSize(p_sel->SValue) );
                case O_GT:
                    return( p_node->GetFreeCPUMemory() > CResourceValue::GetSize(p_sel->SValue) );
                case O_GE:
                    return( p_node->GetFreeCPUMemory() >= CResourceValue::GetSize(p_sel->SValue) );
                case O_EQ:
                    return( p_node->GetFreeCPUMemory() == CResourceValue::GetSize(p_sel->SValue) );
                case O_NE:
                    return( p_node->GetFreeCPUMemory() != CResourceValue::GetSize(p_sel->SValue) );
                default:
                    ES_ERROR("<- unknown selection operator");
                    return(false);
            }
    case T_GPUMEM:
        switch(p_sel->Operator){
            case O_LT:
                return( p_node->GetGPUMemory() < CResourceValue::GetSize(p_sel->SValue) );
            case O_LE:
                return( p_node->GetGPUMemory() <= CResourceValue::GetSize(p_sel->SValue) );
            case O_GT:
                return( p_node->GetGPUMemory() > CResourceValue::GetSize(p_sel->SValue) );
            case O_GE:
                return( p_node->GetGPUMemory() >= CResourceValue::GetSize(p_sel->SValue) );
            case O_EQ:
                return( p_node->GetGPUMemory() == CResourceValue::GetSize(p_sel->SValue) );
            case O_NE:
                return( p_node->GetGPUMemory() != CResourceValue::GetSize(p_sel->SValue) );
            default:
                ES_ERROR("<- unknown selection operator");
                return(false);
        }
        case T_NAME:
            switch(p_sel->Operator){
                case O_EQ:
                    return( fnmatch(p_sel->SValue,p_node->GetName(),0) == 0 );
                case O_NE:
                    return( fnmatch(p_sel->SValue,p_node->GetName(),0) != 0 );
                default:
                    ES_ERROR("<- unknown selection operator");
                    return(false);
            }
        case T_BS:
            switch(p_sel->Operator){
                case O_EQ:
                    return( fnmatch(p_sel->SValue,p_node->GetShortServerName(),0) == 0 );
                case O_NE:
                    return( fnmatch(p_sel->SValue,p_node->GetShortServerName(),0) != 0 );
                default:
                    ES_ERROR("<- unknown selection operator");
                    return(false);
            }
        case T_ST:
            switch(p_sel->Operator){
                case O_EQ:
                    return( fnmatch(p_sel->SValue,CSmallString(p_node->GetStateCode()),0) == 0 );
                case O_NE:
                    return( fnmatch(p_sel->SValue,CSmallString(p_node->GetStateCode()),0) != 0 );
                default:
                    ES_ERROR("<- unknown selection operator");
                    return(false);
            }
        case T_PROPS:{
                std::vector<std::string>::const_iterator it = p_node->GetPropertyList().begin();
                std::vector<std::string>::const_iterator ie = p_node->GetPropertyList().end();
                bool ok = false;
                while( it != ie ){
                    if( fnmatch(p_sel->SValue,(*it).c_str(),0) == 0 ){
                        ok = true;
                        break;
                    }
                    it++;
                }
                switch(p_sel->Operator){
                    case O_EQ:
                        return( ok );
                    case O_NE:
                        return( ! ok );
                    default:
                        ES_ERROR("<- unknown selection operator");
                        return(false);
                }
            }
        default:
            ES_ERROR("<- unsupported selector");
            return(false);
    }

}

//------------------------------------------------------------------------------

void CNodeList::RemoveDownNodes(void)
{
    list<CNodePtr>::iterator it = begin();
    list<CNodePtr>::iterator et = end();

    while( it != et ){
        CNodePtr p_node = *it;
        if( p_node->IsDown() == true ){
            it = erase(it);
        } else {
            it++;
        }
    }
}

//------------------------------------------------------------------------------

void CNodeList::KeepNodeGroups(const CSmallString& groups)
{
    string sgroups = string(groups);
    set<string> lgroups;
    split(lgroups,sgroups,is_any_of(","));

    list<CNodeGroupPtr>::iterator it = NodeGroups.begin();
    list<CNodeGroupPtr>::iterator et = NodeGroups.end();

    while( it != et ){
        CNodeGroupPtr p_group = *it;
        if( lgroups.find(p_group->GroupName) == lgroups.end() ){
            it = NodeGroups.erase(it);
        } else {
            it++;
        }
    }

}

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

bool CNodeGroup::CompareNamesB(const CNodePtr& p_left,const CNodePtr& p_right)
{
    if( p_left->GetShortServerName() == p_right->GetShortServerName() ) {
        string s1 = string(p_left->GetName());
        string s2 = string(p_right->GetName());

        string n1 = s1;
        string n2 = s2;
        int    i1 = 0;
        int    i2 = 0;

        std::size_t found = s1.find_first_of("0123456789");
        if( (found != string::npos) && (found > 0) ){
            n1 = s1.substr(0,found);
            stringstream str(s1.substr(found));
            str >> i1;
        }

        found = s2.find_first_of("0123456789");
        if( (found != string::npos) && (found > 0) ){
            n2 = s2.substr(0,found);
            stringstream str(s2.substr(found));
            str >> i2;
        }

        if( n1 == n2 ){
            return( i1 <= i2 );
        }

        return( n1 <= n2 );
    } else {
        return( strcmp(p_left->GetShortServerName(),p_right->GetShortServerName()) < 0 );
    }
}

//------------------------------------------------------------------------------

void CNodeGroup::MakeSortedNodeList(void)
{
    // copy items
    SortedNodes.insert(SortedNodes.end(),begin(),end());
    // and sort them
    SortedNodes.sort(CompareNamesB);
}

//------------------------------------------------------------------------------

void CNodeGroup::GenerateCommonProperties(void)
{
    // generate common properties
    list<CNodePtr>::iterator it = SortedNodes.begin();
    list<CNodePtr>::iterator et = SortedNodes.end();

    while( it != et ){
        CNodePtr p_node = *it;
        CommonProps.insert(p_node->GetPropertyList().begin(),p_node->GetPropertyList().end());
        it++;
    }

    // what should be removed
    set<string>::iterator pit = CommonProps.begin();
    set<string>::iterator pet = CommonProps.end();
    set<string>           remove;

    while( pit != pet ){
        list<CNodePtr>::iterator it = SortedNodes.begin();
        list<CNodePtr>::iterator et = SortedNodes.end();

        while( it != et ){
            CNodePtr p_node = *it;
            if( ! p_node->HasProperty(*pit) ){
                remove.insert(*pit);
            }
            it++;
        }
        pit++;
    }

    // remove properties
    set<string>::iterator rit = remove.begin();
    set<string>::iterator rie = remove.end();

    while( rit != rie ){
        CommonProps.erase(*rit);
        rit++;
    }
}

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

