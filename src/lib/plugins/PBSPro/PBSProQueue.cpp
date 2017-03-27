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

#include "PBSProQueue.hpp"
#include <ErrorSystem.hpp>
#include <pbs_ifl.h>
#include <iomanip>
#include "PBSProAttr.hpp"

using namespace std;

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

CPBSProQueue::CPBSProQueue(void)
{

}

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

bool CPBSProQueue::Init(struct batch_status* p_queue)
{
    if( p_queue == NULL ){
        ES_ERROR("p_queue is NULL");
        return(false);
    }

    Name = p_queue->name;

    // get attributes
    bool result = true;
    result &= get_attribute(p_queue->attribs,ATTR_QUEUE_TYPE,NULL,Type);
    result &= get_attribute(p_queue->attribs,ATTR_QUEUE_STARTED,NULL,Started);
    result &= get_attribute(p_queue->attribs,ATTR_QUEUE_ENABLED,NULL,Enabled);

    // optional
    get_attribute(p_queue->attribs,ATTR_QUEUE_PRIORITY,NULL,Priority);

    result &= get_attribute(p_queue->attribs,ATTR_QUEUE_TOTAL_JOBS,NULL,TotalJobs);

//    result &= get_attribute(p_queue->attribs,ATTR_QUEUE_MAX_RUNNING,NULL,MaxRunning);
//    result &= get_attribute(p_queue->attribs,ATTR_QUEUE_MAX_USER_RUNNING,NULL,MaxUserRunning);

    result &= get_attribute(p_queue->attribs,ATTR_QUEUE_RESOURCES_MAX,RESOURCES_PROCS,MaxCPUs);
    result &= get_attribute(p_queue->attribs,ATTR_QUEUE_RESOURCES_MAX,RESOURCES_WALLTIME,MaxWallTime);

    bool acl_enabled;
    //---------------------
    acl_enabled = false;
    get_attribute(p_queue->attribs,ATTR_QUEUE_ACL_USER_ENABLED,NULL,acl_enabled);
    if( acl_enabled ){
        result &= get_attribute(p_queue->attribs,ATTR_QUEUE_ACL_USERS,NULL,ACLUsers);
    }
    //---------------------
    acl_enabled = false;
    get_attribute(p_queue->attribs,ATTR_QUEUE_ACL_GROUP_ENABLED,NULL,acl_enabled);
    if( acl_enabled ){
        result &= get_attribute(p_queue->attribs,ATTR_QUEUE_ACL_GROUPS,NULL,ACLGroups);
    }
    //---------------------
    // optional
    RouteDestinations = "";
    get_attribute(p_queue->attribs,ATTR_QUEUE_ROUTE_DESTINATIONS,NULL,RouteDestinations);
    //---------------------
    // parse state count
    int n = 0;
    CSmallString count;
    if( get_attribute(p_queue->attribs,ATTR_QUEUE_STATE_COUNT,NULL,count) == true ){
        sscanf(count,"Transit:%d Queued:%d Held:%d Waiting:%d Running:%d Exiting:%d Begun:%d",&n,&QueuedJobs,&n,&n,&RunningJobs,&n,&n);
    }

    if( result ){
        CSmallString error;
        error << "unable to get attribute(s) of queue '" << Name << "'";
        ES_TRACE_ERROR(error);
    }

    return(result);
}

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

