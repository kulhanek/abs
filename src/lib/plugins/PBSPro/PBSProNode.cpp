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

#include "PBSProNode.hpp"
#include <ErrorSystem.hpp>
#include <pbs_ifl.h>
#include "PBSProAttr.hpp"
#include <boost/algorithm/string/split.hpp>
#include <boost/algorithm/string/join.hpp>
#include <boost/algorithm/string/trim.hpp>
#include <boost/algorithm/string/classification.hpp>
#include <boost/format.hpp>

//------------------------------------------------------------------------------

using namespace std;
using namespace boost;
using namespace boost::algorithm;
using namespace boost::range;

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

CPBSProNode::CPBSProNode(void)
{
}

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

bool CPBSProNode::Init(const CSmallString& srv_name,const CSmallString& short_srv_name,struct batch_status* p_node)
{
    if( p_node == NULL ){
        ES_ERROR("p_node is NULL");
        return(false);
    }

    Name = p_node->name;
    BatchServerName = srv_name;
    ShortServerName = short_srv_name;

    // all attributes are optional
    get_attribute(p_node->attribs,"Mom",NULL,MOM);

    get_attribute(p_node->attribs,"resources_available","ncpus",NCPUs);
    get_attribute(p_node->attribs,"resources_assigned","ncpus",AssignedCPUs);

    get_attribute(p_node->attribs,"resources_available","ngpus",NGPUs);
    get_attribute(p_node->attribs,"resources_assigned","ngpus",AssignedGPUs);

    get_attribute(p_node->attribs,"resources_available","mem",Memory);
    get_attribute(p_node->attribs,"resources_assigned","mem",AssignedMemory);

    get_attribute(p_node->attribs,"resources_available","scratch_local",ScratchLocal);
    get_attribute(p_node->attribs,"resources_available","scratch_shared",ScratchShared);
    get_attribute(p_node->attribs,"resources_available","scratch_ssd",ScratchSSD);

    std::string tmp;

    // parse jobs
    get_attribute(p_node->attribs,"jobs",NULL,Jobs);
    if( Jobs != NULL ){
        tmp = Jobs;
        std::vector<string> jlist;
        split(jlist,tmp,is_any_of(","));
        std::vector<string>::iterator it = jlist.begin();
        std::vector<string>::iterator ie = jlist.end();

        while( it != ie ){
            string slot = *it;
            std::vector<string> slist;
            split(slist,slot,is_any_of("/"));
            if( slist.size() == 2 ){
                string job = slist[0];
                string sid = slist[1];
                trim(job);
                trim(sid);
                JobList.insert(job);
                JobSlots[job].push_back(sid);
            }
            it++;
        }
    }

    // queue list
    get_attribute(p_node->attribs,"resources_available","queue_list",Queues);
    tmp = string(Queues);
    if( ! tmp.empty() ) split(QueueList,tmp,is_any_of(","));

    // user list
    // queue list
    get_attribute(p_node->attribs,"resources_available","node_owner",NodeOwner);
    tmp = string(NodeOwner);
    if( ! tmp.empty() ) split(OwnerList,tmp,is_any_of(","));

    // detect properties
    // all "resources_available" which is boolean
    struct attrl* p_list = p_node->attribs;

    while( p_list != NULL ){
        if( strcmp(p_list->name,"resources_available") == 0 ){
            if( p_list->resource != NULL ){
                if( (strcmp(p_list->value,"True") == 0) || (strcmp(p_list->value,"true") == 0) ){
                    PropList.push_back(p_list->resource);
                    AllPropList.push_back(p_list->resource);
                }
                if( (strcmp(p_list->value,"False") == 0) || (strcmp(p_list->value,"false") == 0) ){
                    AllPropList.push_back(p_list->resource);
                }
            }
        }
        p_list = p_list->next;
    }

    sort(PropList.begin(),PropList.end());
    sort(AllPropList.begin(),AllPropList.end());
    Properties = join(PropList,",");

    // comment
    get_attribute(p_node->attribs,"comment",NULL,Comment);

    // decode status
    CSmallString state;
    get_attribute(p_node->attribs,"state",NULL,state);

    std::list<std::string>  states;
                            tmp = string(state);
    if( ! tmp.empty() ) split(states,tmp,is_any_of(","));

    // remove free, job-exclusive  (wrong in MetaCentrum)
    // is it necessary for PBSPro?
    states.remove("free");
    states.remove("job-exclusive");

    CSmallString queue;
    get_attribute(p_node->attribs,"queue",NULL,queue);
    if( queue == "maintenance"){
        states.push_back("maintenance");
    }

    // update state
    if( IsDown() == false ){
        if( (AssignedCPUs == 0) && (AssignedGPUs  == 0) ) {
            states.push_back("free");
        } else if( AssignedCPUs == NCPUs ) {
            states.push_back("fully-occupied");
        } else {
            states.push_back("partially-free");
        }
    }

    State = join(states, ",");

    return(true);
}

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

