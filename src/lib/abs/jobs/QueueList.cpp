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
#include <boost/algorithm/string/split.hpp>
#include <boost/algorithm/string/classification.hpp>
#include <string>
#include <vector>

//------------------------------------------------------------------------------

using namespace std;
using namespace boost;
using namespace boost::algorithm;

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

bool CQueueList::RemoveIfOnlyRoutable(const CQueuePtr& p_queue)
{
    return( p_queue->IsOnlyRoutable() == true );
}

//------------------------------------------------------------------------------

void CQueueList::RemoveOnlyRoutable(void)
{
    remove_if(RemoveIfOnlyRoutable);
}

//------------------------------------------------------------------------------

bool CQueueList::SortCompName(const CQueuePtr& p_left,const CQueuePtr& p_right)
{
    if( p_left->GetShortServerName() == p_right->GetShortServerName()  ){
        return( strcmp(p_left->GetName(),p_right->GetName()) < 0 );
    } else {
       return( strcmp(p_left->GetShortServerName(),p_right->GetShortServerName()) < 0 );
    }
}

//------------------------------------------------------------------------------

bool CQueueList::SortCompWallTime(const CQueuePtr& p_left,const CQueuePtr& p_right)
{
    if( p_left->GetShortServerName() == p_right->GetShortServerName()  ){
        return( p_left->GetMaxWallTime() < p_right->GetMaxWallTime() );
    } else {
       return( strcmp(p_left->GetShortServerName(),p_right->GetShortServerName()) < 0 );
    }
}

//------------------------------------------------------------------------------

void CQueueList::SortByName(void)
{
    sort(SortCompName);
}

//------------------------------------------------------------------------------

void CQueueList::SortByWallTime(void)
{
    sort(SortCompWallTime);
}

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

void CQueueList::PrintInfos(std::ostream& sout)
{
    sout << endl;
    sout << "#       Name            Pri    T     Q     R     O      MaxWall     Comment          " << endl;
    sout << "# -------------------- ----- ----- ----- ----- ----- ------------- ------------------" << endl;

    SortByWallTime();

    std::list<CQueuePtr>::iterator it = begin();
    std::list<CQueuePtr>::iterator et = end();

    while( it != et ){
        CQueuePtr p_queue = *it;
        if( p_queue->IsRouteQueue() ){
            PrintRouteDestinationInfos(p_queue,sout);
        } else {
            p_queue->PrintLineInfo(sout,false);
        }
        it++;
    }

    sout << endl;
    sout << "# Legend:" << endl;
    sout << "# Pri - Priority,  T - Total, Q - Queued, R - Running jobs" << endl;
    sout << "# O - Other (finished, exiting, hold) jobs" << endl;
    sout << "# MaxWall - Max wall time per job" << endl;
    sout << low;
}

//------------------------------------------------------------------------------

void CQueueList::PrintRouteDestinationInfos(CQueuePtr& p_mqueue, std::ostream& sout)
{
    std::list<CQueuePtr>::iterator it = begin();
    std::list<CQueuePtr>::iterator et = end();

    while( it != et ){
        CQueuePtr p_queue = *it;
        it++;
        if( p_queue->GetShortServerName() != p_mqueue->GetShortServerName() ) continue;
        if( p_queue->IsRouteDestination(p_mqueue->GetName())){
            p_queue->PrintLineInfo(sout,true);
        }
    }
}

//------------------------------------------------------------------------------

void CQueueList::GetQueuesWithServer(std::vector<std::string>& qlist)
{
    std::list<CQueuePtr>::iterator it = begin();
    std::list<CQueuePtr>::iterator et = end();

    while( it != et ){
        CQueuePtr    p_queue = *it;

        if( p_queue->GetChunkQueueName() != NULL ){
            CSmallString queue;

            queue << p_queue->GetChunkQueueName() << "." << p_queue->GetShortServerName();
            qlist.push_back(string(queue));
        }

        it++;
    }
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

//------------------------------------------------------------------------------

const CQueuePtr CQueueList::FindQueueUnique(const CSmallString& name)
{
    std::list<CQueuePtr>::iterator it = begin();
    std::list<CQueuePtr>::iterator et = end();

    CQueuePtr queue_ptr;

    while( it != et ){
        if( (*it)->GetName() == name ){
            if( queue_ptr != NULL ) return(CQueuePtr());
            queue_ptr = *it;
        }
        it++;
    }

    return(queue_ptr);
}

//------------------------------------------------------------------------------

const CQueuePtr CQueueList::FindQueue(const CSmallString& server,const CSmallString& name)
{
    std::list<CQueuePtr>::iterator it = begin();
    std::list<CQueuePtr>::iterator et = end();

    while( it != et ){
        if( ((*it)->GetName() == name) && ((*it)->GetShortServerName() == server) ) return(*it);
        it++;
    }

    return(CQueuePtr());
}

//------------------------------------------------------------------------------

const CQueuePtr CQueueList::FindQueueByChunkName(const CSmallString& server,const CSmallString& name)
{
    std::list<CQueuePtr>::iterator it = begin();
    std::list<CQueuePtr>::iterator et = end();

    while( it != et ){
        if( ((*it)->GetChunkQueueName() == name) && ((*it)->GetShortServerName() == server)  ) return(*it);
        it++;
    }

    return(CQueuePtr());
}

//------------------------------------------------------------------------------

void CQueueList::GetQueueFromChunkQueue(const CSmallString& server,const CSmallString& chunk_queue,std::list<CSmallString>& qlist)
{
    CQueuePtr p_queue = FindQueueByChunkName(server,chunk_queue);
    if( p_queue == NULL ) return;

    if( p_queue->IsOnlyRoutable() == false ){
        qlist.push_back(p_queue->GetName());
    }

    std::list<CQueuePtr>::iterator it = begin();
    std::list<CQueuePtr>::iterator et = end();

    while( it != et ){
        CQueuePtr p_rqueue = *it;
        it++;
        if( p_rqueue->GetShortServerName() != server ) continue;
        if( p_rqueue->IsRouteQueue() == false ) continue;
        if( p_rqueue->IsRouteDestination(p_queue->GetName()) ){
            CSmallString qname;
            qname << p_rqueue->GetName() << "(via " << p_queue->GetName() << ")";
            qlist.push_back(qname);
        }
    }
}

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

