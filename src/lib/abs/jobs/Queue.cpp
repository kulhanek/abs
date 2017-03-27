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
#include <iomanip>

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
    OnlyRoutable = false;
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

bool CQueue::IsRoute(void) const
{
    return( RouteDestinations != NULL );
}

//------------------------------------------------------------------------------

bool CQueue::IsRouteDestination(void) const
{
    return( RouteQueue != NULL );
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
    if( OnlyRoutable ){
        sout << "<gray>";
    }

    sout << right;
    sout << " " << setw(3) << ShortServerName;

    sout << left;
    sout << setw(17) << Name;

    sout << right;
    sout << " " << setw(5) << Priority;
    sout << " " << setw(5) << TotalJobs;
    sout << " " << setw(5) << QueuedJobs;
    sout << " " << setw(5) << RunningJobs;
    sout << " " << setw(5) << TotalJobs - (RunningJobs+QueuedJobs);
    sout << " " << setw(13) << MaxWallTime.GetSTimeAndDay();
    sout << " " << Comment << endl;

    if( OnlyRoutable ){
        sout << "</gray>";
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

