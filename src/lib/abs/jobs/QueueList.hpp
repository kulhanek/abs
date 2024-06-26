#ifndef QueueListH
#define QueueListH
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

#include <ABSMainHeader.hpp>
#include <Queue.hpp>
#include <list>

// -----------------------------------------------------------------------------

class ABS_PACKAGE CQueueList : public std::list<CQueuePtr> {
public:
// constructor -----------------------------------------------------------------
    CQueueList(void);

// executive methods -----------------------------------------------------------
    //! remove disabled queues
    void RemoveDisabledQueues(void);

    //! remove stopped queues
    void RemoveStoppedQueues(void);

    //! remove user innacesible queues
    void RemoveInaccesibleQueues(CUser& user);

    //! remove nonexecutive queues
    void RemoveNonexecutiveQueues(void);

    //! remove only routable queues
    void RemoveOnlyRoutable(void);

    //! sort queues by name
    void SortByName(void);

    //! sort queues by walltime
    void SortByWallTime(void);

// information methods ---------------------------------------------------------
    //! print info about all queues
    void PrintInfos(std::ostream& sout);

    //! get queues suggestions
    void GetQueuesSuggestions(std::vector<CSmallString>& suggestions);

    //! get list of queue with server (queue.server)
    void GetQueuesWithServer(std::vector<std::string>& qlist);

    //! find queue
    const CQueuePtr FindQueue(const CSmallString& name);

    //! find queue - only if one exist
    const CQueuePtr FindQueueUnique(const CSmallString& name);

    //! find queue
    const CQueuePtr FindQueue(const CSmallString& server,const CSmallString& name);

    //! find queue
    const CQueuePtr FindQueueByChunkName(const CSmallString& server,const CSmallString& name);

    //! get queue list for chunk
    void GetQueueFromChunkQueue(const CSmallString& server,const CSmallString& chunk_queue,std::list<std::string>& qlist);

// section of private data -----------------------------------------------------
private:
    //! compare two queues by name
    static bool SortCompName(const CQueuePtr& p_left,const CQueuePtr& p_right);

    //! compare two queues by walltime
    static bool SortCompWallTime(const CQueuePtr& p_left,const CQueuePtr& p_right);

    //! remove if dissabled
    static bool RemoveIfDisabled(const CQueuePtr& p_queue);

    //! remove if stopped
    static bool RemoveIfStopped(const CQueuePtr& p_queue);

    //! remove if nonexecutive
    static bool RemoveIfNonexecutive(const CQueuePtr& p_queue);

    //! remove if only routable
    static bool RemoveIfOnlyRoutable(const CQueuePtr& p_queue);

    //! print route destinations
    void PrintRouteDestinationInfos(CQueuePtr& p_mqueue, std::ostream& sout);
};

//------------------------------------------------------------------------------

extern CQueueList QueueList;

// -----------------------------------------------------------------------------

#endif
