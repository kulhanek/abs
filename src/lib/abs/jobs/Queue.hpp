#ifndef QueueH
#define QueueH
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
#include <vector>
#include <iostream>
#include <SmallString.hpp>
#include <SmallTime.hpp>
#include <User.hpp>
#include <boost/shared_ptr.hpp>

// -----------------------------------------------------------------------------

class CQueue;
typedef boost::shared_ptr<CQueue>  CQueuePtr;

// -----------------------------------------------------------------------------

/// toruqe queue

class ABS_PACKAGE CQueue {
public:
// constructor -----------------------------------------------------------------
        CQueue(void);

// information methods ---------------------------------------------------------
    //! is started?
    bool IsStarted(void) const;

    //! is enabled?
    bool IsEnabled(void) const;

    //! is route
    bool IsRoute(void) const;

    //! is route destination
    bool IsRouteDestination(void) const;

    //! print line info about the queue
    void PrintLineInfo(std::ostream& sout);

    //! is queue allowed for the user
    bool IsAllowed(CUser& user) const;

    //! get name
    const CSmallString& GetName(void) const;

    //! get type
    const CSmallString& GetType(void) const;

    //! get walltime
    const CSmallTime& GetMaxWallTime(void) const;

// section of private data -----------------------------------------------------
protected:
    CSmallString                ShortServerName;

    CSmallString                Name;
    CSmallString                Type;
    bool                        Enabled;
    bool                        Started;
    bool                        OnlyRoutable;
    int                         Priority;

    int                         TotalJobs;
    int                         QueuedJobs;
    int                         RunningJobs;

    CSmallString                Comment;

    CSmallTime                  MaxWallTime;
    CSmallString                RouteDestinations;
    CSmallString                RouteQueue;

    std::vector<CSmallString>   ACLUsers;
    std::vector<CSmallString>   ACLGroups;

    friend class CQueueList;
};

// -----------------------------------------------------------------------------

#endif
