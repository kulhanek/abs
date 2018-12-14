#ifndef BatchServerH
#define BatchServerH
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
#include <ComObject.hpp>
#include <Job.hpp>
#include <Node.hpp>
#include <iostream>
#include <string>

// -----------------------------------------------------------------------------

class CQueueList;
class CNodeList;
class CJobList;
class CJob;
class CResourceList;

// -----------------------------------------------------------------------------

class CBatchServer;
typedef boost::shared_ptr<CBatchServer>  CBatchServerPtr;

// -----------------------------------------------------------------------------

class ABS_PACKAGE CBatchServer: public CComObject {
public:
// constructor -----------------------------------------------------------------
        CBatchServer(CPluginObject* p_objectinfo);
        ~CBatchServer(void);

// init batch system subsystem -------------------------------------------------
    //! load symbols and connect to server
    virtual bool Init(const CSmallString& server_name,const CSmallString& short_name,
                      const CSmallString& alt_names,bool job_transfer);

    /// get server name
    const CSmallString GetServerName(void);

    /// get short server name
    const CSmallString GetShortName(void);

    /// match name?
    bool DoesItMatchName(const CSmallString& srvname);

// helper methods --------------------------------------------------------------

    /// get full job ID - if jobid does not contain server, ServerName is appended, otherwise jobid is returned
    const CSmallString GetFullJobID(const CSmallString& jobid);

// enumeration -----------------------------------------------------------------
    // IMPORTANT NOTE:
    // in this API, jobid, vnode, queue must be without server specification
    // not true anymore as we have support for moved jobs

    //! init queue list
    virtual bool GetQueues(CQueueList& queues);

    //! init node list
    virtual bool GetNodes(CNodeList& nodes);

    //! get given node
    virtual const CNodePtr GetNode(const CSmallString& name);

    //! init all job list
    virtual bool GetAllJobs(CJobList& jobs,bool include_history);

    //! get queue jobs
    virtual bool GetQueueJobs(CJobList& jobs,const CSmallString& queue_name,bool include_history);

    //! init job list of given user
    virtual bool GetUserJobs(CJobList& jobs,const CSmallString& user,bool include_history);

    //! get job by job id
    virtual const CJobPtr GetJob(const CSmallString& jobid);

    //! print technical info about queues
    virtual bool PrintQueues(std::ostream& sout);

    //! print technical info about nodes
    virtual bool PrintNodes(std::ostream& sout);

    //! print technical info about single node
    virtual bool PrintNode(std::ostream& sout,const CSmallString& name);

    //! print technical info about jobs
    virtual bool PrintJobs(std::ostream& sout,bool include_history);

    //! print technical info about a single job
    virtual bool PrintJob(std::ostream& sout,const CSmallString& name,bool include_history);

    //! locate job - return server name for the job
    virtual const CSmallString LocateJob(const CSmallString& jobid);

// execution -------------------------------------------------------------------
    //! init specific resources for batch server
    virtual bool InitBatchResources(CResourceList* p_rl);

    //! submit job
    virtual bool SubmitJob(CJob& job,bool verbose);

    //! get job status
    virtual bool GetJobStatus(CJob& job);

    //! kill job
    virtual bool KillJob(CJob& job);

    //! kill job by ID
    virtual bool KillJobByID(const CSmallString& jobid);

// timing ----------------------------------------------------------------------
    /// start timer
    void StartTimer(void);

    /// end timer
    void EndTimer(void);

    /// get exec time in seconds
    double GetExecTime(void);

// -----------------------------------------------------------------------------
protected:
    CSmallString    ServerName;
    CSmallString    ShortName;
    CSmallString    AltNames;           // alternative names (comma separated list)
    bool            AllowJobTransfer;   // allow job transfer to another server
    size_t          ExecTime;           // execution time in clock ticks
    size_t          TimerStart;         // timer start time in clock ticks
};

// -----------------------------------------------------------------------------

#endif
