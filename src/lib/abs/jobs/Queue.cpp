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

#include <Queue.hpp>
#include <ErrorSystem.hpp>
#include <pbs_ifl.h>
#include <iomanip>
#include <TorqueAttr.hpp>

using namespace std;

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

CQueue::CQueue(void)
{
    Enabled = true;
    Started = true;
    Priority = 0;
    TotalJobs = 0;
    QueuedJobs = 0;
    RunningJobs = 0;
    MaxRunning = 0;
    MaxUserRunning = 0;
    MaxCPUs = 0;
}

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

bool CQueue::Init(struct batch_status* p_queue)
{
    if( p_queue == NULL ){
        ES_ERROR("p_queue is NULL");
        return(false);
    }

    Name = p_queue->name;
    CSmallString count;

    // get attributes
    bool result = true;
    result &= get_attribute(p_queue->attribs,ATTR_QUEUE_TYPE,NULL,Type);
    result &= get_attribute(p_queue->attribs,ATTR_QUEUE_STARTED,NULL,Started);
    result &= get_attribute(p_queue->attribs,ATTR_QUEUE_ENABLED,NULL,Enabled);
    result &= get_attribute(p_queue->attribs,ATTR_QUEUE_PRIORITY,NULL,Priority);
    result &= get_attribute(p_queue->attribs,ATTR_QUEUE_TOTAL_JOBS,NULL,TotalJobs);
    result &= get_attribute(p_queue->attribs,ATTR_QUEUE_STATE_COUNT,NULL,count);
    result &= get_attribute(p_queue->attribs,ATTR_QUEUE_REQUIRED_PROPERTY,NULL,RequiredProperty);
    result &= get_attribute(p_queue->attribs,ATTR_QUEUE_MAX_RUNNING,NULL,MaxRunning);
    result &= get_attribute(p_queue->attribs,ATTR_QUEUE_MAX_USER_RUNNING,NULL,MaxUserRunning);
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
    sscanf(count,"Transit:%d Queued:%d Held:%d Waiting:%d Running:%d Exiting:%d",&n,&QueuedJobs,&n,&n,&RunningJobs,&n);

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

bool CQueue::IsStarted(void) const
{
    return(Started);
}

//------------------------------------------------------------------------------

bool CQueue::IsEnabled(void) const
{
    return(Enabled);
}

//------------------------------------------------------------------------------

const CSmallString& CQueue::GetName(void) const
{
    return(Name);
}

//------------------------------------------------------------------------------

const CSmallString& CQueue::GetType(void) const
{
    return(Type);
}

//------------------------------------------------------------------------------

const CSmallString& CQueue::GetRequiredProperty(void) const
{
    return(RequiredProperty);
}

//------------------------------------------------------------------------------

bool CQueue::IsAllowed(CUser& user) const
{
    // any ACL?
    if( ACLUsers.empty() && ACLGroups.empty() ) return(true);

    // check users
    for(unsigned int i = 0; i < ACLUsers.size(); i++){
        if( user.GetName() == ACLUsers[i] ) return(true);
    }

    // check groups
    for(unsigned int i = 0; i < ACLGroups.size(); i++){
        if( user.IsInGroup(ACLGroups[i]) ) return(true);
    }

    return(false);
}

//------------------------------------------------------------------------------

void CQueue::PrintLineInfo(std::ostream& sout)
{
    sout << left;
    sout << setw(17) << Name;
    sout << right;

    if( Type == "Execution" ) {

    sout << " " << setw(5) << Priority;
    sout << " " << setw(5) << TotalJobs;
    sout << " " << setw(5) << QueuedJobs;
    sout << " " << setw(5) << RunningJobs;
    sout << " " << setw(5) << TotalJobs - (RunningJobs+QueuedJobs);
    sout << " " << setw(5) << MaxRunning;
    sout << " " << setw(4) << MaxUserRunning;
    sout << " " << setw(4) << MaxCPUs;
    sout << " " << setw(13) << MaxWallTime.GetSTimeAndDay();
    sout << left;
    sout << "  ";

    if( Started ){
        sout << "S";
    } else {
        sout << "-";
    }
    if( Enabled ){
        sout << "E";
    } else {
        sout << "-";
    }
    sout << " " << RequiredProperty << endl;

    } else {
    sout << " --> " << RouteDestinations << " (routing queue)" << endl;
    }
}

//------------------------------------------------------------------------------

const CSmallTime& CQueue::GetMaxWallTime(void) const
{
    return(MaxWallTime);
}

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

