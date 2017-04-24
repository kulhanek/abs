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

    ShortServerName = short_srv_name;
    Name = p_queue->name;

    // all ttributtes are optional
    get_attribute(p_queue->attribs,ATTR_qtype,NULL,Type);
    get_attribute(p_queue->attribs,ATTR_start,NULL,Started);
    get_attribute(p_queue->attribs,ATTR_enable,NULL,Enabled);
    get_attribute(p_queue->attribs,ATTR_p,NULL,Priority);
    get_attribute(p_queue->attribs,ATTR_total,NULL,TotalJobs);
    get_attribute(p_queue->attribs,ATTR_rescmax,"walltime",MaxWallTime);

    get_attribute(p_queue->attribs,ATTR_rescdflt,"walltime",DefaultWallTime);
    if( DefaultWallTime.GetSecondsFromBeginning() == 0 ){
        DefaultWallTime = MaxWallTime;
    }

    get_attribute(p_queue->attribs,ATTR_routedest,NULL,RouteDestinations);
    get_attribute(p_queue->attribs,ATTR_comment,NULL,Comment);
    get_attribute(p_queue->attribs,ATTR_fromroute,NULL,OnlyRoutable);

    bool acl_enabled;
    //---------------------
    acl_enabled = false;
    get_attribute(p_queue->attribs,ATTR_acluren,NULL,acl_enabled);
    if( acl_enabled ){
        get_attribute(p_queue->attribs,ATTR_acluser,NULL,ACLUsers);
    }
    //---------------------
    acl_enabled = false;
    get_attribute(p_queue->attribs,ATTR_aclgren,NULL,acl_enabled);
    if( acl_enabled ){
        get_attribute(p_queue->attribs,ATTR_aclgroup,NULL,ACLGroups);
    }

    get_attribute(p_queue->attribs,ATTR_DefaultChunk,"queue_list",ChunkQueue);

    //---------------------
    // parse state count
    int n = 0;
    CSmallString count;
    if( get_attribute(p_queue->attribs,ATTR_count,NULL,count) == true ){
        sscanf(count,"Transit:%d Queued:%d Held:%d Waiting:%d Running:%d Exiting:%d Begun:%d",&n,&QueuedJobs,&n,&n,&RunningJobs,&n,&n);
    }

// BUG - FIXME - TotalJobs reported by PBSPro is incorrect
    TotalJobs =  QueuedJobs + RunningJobs;

    return(true);
}

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

