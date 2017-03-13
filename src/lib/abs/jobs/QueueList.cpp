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

#include <QueueList.hpp>
#include <SimpleIterator.hpp>
#include <string.h>

using namespace std;

//------------------------------------------------------------------------------

CQueueList QueueList;

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

CQueueList::CQueueList(void)
{

}

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

bool CQueueList::RemoveIfDisabled(const CQueuePtr& p_queue)
{
    return( ! p_queue->IsEnabled() );
}

//------------------------------------------------------------------------------

void CQueueList::RemoveDisabledQueues(void)
{
    remove_if(RemoveIfDisabled);
}

//------------------------------------------------------------------------------

bool CQueueList::RemoveIfStopped(const CQueuePtr& p_queue)
{
    return( ! p_queue->IsStarted() );
}

//------------------------------------------------------------------------------

void CQueueList::RemoveStoppedQueues(void)
{
    remove_if(RemoveIfStopped);
}

//------------------------------------------------------------------------------

class not_allowed_for{
public:
    not_allowed_for(CUser& user)
        : User(user){};

    bool operator()(const CQueuePtr& p_queue)
    { return( !p_queue->IsAllowed(User)); };

    CUser& User;
};

//------------------------------------------------------------------------------

void CQueueList::RemoveInaccesibleQueues(CUser& user)
{
    remove_if(not_allowed_for(user));
}

//------------------------------------------------------------------------------

bool CQueueList::RemoveIfNonexecutive(const CQueuePtr& p_queue)
{
    return( ((p_queue->GetType() != "Execution") && (p_queue->GetType() != "Route") ) && (p_queue->GetType() != NULL) );
}

//------------------------------------------------------------------------------

void CQueueList::RemoveNonexecutiveQueues(void)
{
    remove_if(RemoveIfNonexecutive);
}

//------------------------------------------------------------------------------

bool CQueueList::SortCompName(const CQueuePtr& p_left,const CQueuePtr& p_right)
{
    return( strcmp(p_left->GetName(),p_right->GetName()) < 0 );
}

//------------------------------------------------------------------------------

void CQueueList::SortByName(void)
{
    sort(SortCompName);
}

//------------------------------------------------------------------------------

void CQueueList::GetRequiredProperties(std::vector<std::string>& qprops)
{
    qprops.clear();

    std::list<CQueuePtr>::iterator it = begin();
    std::list<CQueuePtr>::iterator et = end();

    while( it != et ){
        CQueuePtr p_queue = *it;
        string rprop = string(p_queue->GetRequiredProperty());
        if( ! rprop.empty() ) {
            qprops.push_back(rprop);
        }
        it++;
    }
}

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

void CQueueList::PrintInfos(std::ostream& sout)
{
    sout << endl;
    sout << "#       Name       Pri    T     Q     R     O    Max  UMax CMax    MaxWall    Mod Required property" << endl;
    sout << "# --------------- ----- ----- ----- ----- ----- ----- ---- ---- ------------- --- -----------------" << endl;

    std::list<CQueuePtr>::iterator it = begin();
    std::list<CQueuePtr>::iterator et = end();

    while( it != et ){
        (*it)->PrintLineInfo(sout);
        it++;
    }

    sout << endl;
    sout << "# Legend:" << endl;
    sout << "# Pri - Priority,  T - Total, Q - Queued, R - Running jobs" << endl;
    sout << "# O - Other (completed, exiting, hold) jobs" << endl;
    sout << "# Max - Max running jobs, UMax - Max user running jobs" << endl;
    sout << "# CMax - Max CPUs per job, MaxWall - Max wall time per job" << endl;
    sout << "# Mod - Started/(-)Stopped : Enabled/(-)Disabled" << endl;
    sout << low;
}

//------------------------------------------------------------------------------

const CQueuePtr CQueueList::FindQueue(const CSmallString& name)
{
    std::list<CQueuePtr>::iterator it = begin();
    std::list<CQueuePtr>::iterator et = end();

    while( it != et ){
        if( (*it)->GetName() == name ) return(*it);
        it++;
    }

    return(CQueuePtr());
}

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

