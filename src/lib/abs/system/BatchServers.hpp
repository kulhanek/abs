#ifndef BatchServersH
#define BatchServersH
// =============================================================================
// ABS - Advanced Batch System
// -----------------------------------------------------------------------------
//    Copyright (C) 2017      Petr Kulhanek, kulhanek@chemi.muni.cz
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
#include <SmallString.hpp>
#include <Job.hpp>
#include <BatchServer.hpp>
#include <iostream>

// -----------------------------------------------------------------------------

class CJob;
class CJobList;

// -----------------------------------------------------------------------------

class ABS_PACKAGE CBatchServers : std::list<CBatchServerPtr> {
public:
// constructor -----------------------------------------------------------------
    CBatchServers(void);
    ~CBatchServers(void);

// -----------------------------------------------------------------------------
    /// init all servers
    bool InitAll(void);

    /// init given server - srv can be either the full or short server name
    bool Init(const CSmallString& srv);

// -----------------------------------------------------------------------------
    /// is server available
    bool IsServerAvailable(const CSmallString& name);

    /// print batch systems
    void PrintServerOverview(std::ostream& vout);

// -----------------------------------------------------------------------------
    /// list queues
    bool GetQueues(void);

    /// list nodes
    bool GetNodes(void);

    //! init all job list
    bool GetAllJobs(CJobList& jobs);

    //! get queue jobs
    bool GetQueueJobs(CJobList& jobs,const CSmallString& queue_name);

    //! init job list of given user
    bool GetUserJobs(CJobList& jobs,const CSmallString& user);

    //! init job list by job id
    bool GetJob(CJobList& jobs,const CSmallString& jobid);

    //! get job by job id
    const CJobPtr GetJob(const CSmallString& jobid);

// execution -------------------------------------------------------------------
    //! submit job
    bool SubmitJob(CJob& job);

    //! get job status
    bool GetJobStatus(CJob& job);

    //! kill job
    bool KillJob(CJob& job);

    //! kill job by ID
    bool KillJobByID(const CSmallString& jobid);

    //! get last error message
    const CSmallString GetLastErrorMsg(void);

// technical print -------------------------------------------------------------
    //! print technical info about queues
    void PrintQueues(std::ostream& sout);

    //! print technical info about nodes
    bool PrintNodes(std::ostream& sout);

    //! print technical info about single node
    bool PrintNode(std::ostream& sout,const CSmallString& name);

    //! print technical info about jobs
    bool PrintJobs(std::ostream& sout);

    //! print technical info about single job
    bool PrintJob(std::ostream& sout,const CSmallString& name);

// private data ----------------------------------------------------------------
private:

};

//------------------------------------------------------------------------------

extern CBatchServers BatchServers;

// -----------------------------------------------------------------------------

#endif
