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

bool CPBSProQueue::Init(const CSmallString& short_srv_name,struct batch_status* p_queue)
{
    if( p_queue == NULL ){
        ES_ERROR("p_queue is NULL");
        return(false);
    }

    Name = p_queue->name;

    // all ttributtes are optional
    get_attribute(p_queue->attribs,"queue_type",NULL,Type);
    get_attribute(p_queue->attribs,"started",NULL,Started);
    get_attribute(p_queue->attribs,"enabled",NULL,Enabled);
    get_attribute(p_queue->attribs,"Priority",NULL,Priority);
    get_attribute(p_queue->attribs,"total_jobs",NULL,TotalJobs);
    get_attribute(p_queue->attribs,"resources_max","walltime",MaxWallTime);
    get_attribute(p_queue->attribs,"route_destinations",NULL,RouteDestinations);

    bool acl_enabled;
    //---------------------
    acl_enabled = false;
    get_attribute(p_queue->attribs,"acl_user_enable",NULL,acl_enabled);
    if( acl_enabled ){
        get_attribute(p_queue->attribs,"acl_users",NULL,ACLUsers);
    }
    //---------------------
    acl_enabled = false;
    get_attribute(p_queue->attribs,"acl_group_enable",NULL,acl_enabled);
    if( acl_enabled ){
        get_attribute(p_queue->attribs,"acl_groups",NULL,ACLGroups);
    }

    //---------------------
    // parse state count
    int n = 0;
    CSmallString count;
    if( get_attribute(p_queue->attribs,"state_count",NULL,count) == true ){
        sscanf(count,"Transit:%d Queued:%d Held:%d Waiting:%d Running:%d Exiting:%d Begun:%d",&n,&QueuedJobs,&n,&n,&RunningJobs,&n,&n);
    }

    return(true);
}

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

