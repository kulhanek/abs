#ifndef BatchClientH
#define BatchClientH
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
#include <iostream>
#include <string>

// -----------------------------------------------------------------------------

class CQueueList;
class CNodeList;
class CJobList;
class CJob;

// -----------------------------------------------------------------------------

class ABS_PACKAGE CBatchClient: public CComObject {
public:
// constructor -----------------------------------------------------------------
        CBatchClient(CPluginObject* p_objectinfo);
        ~CBatchClient(void);

// init torque subsystem -------------------------------------------------------
    //! load symbols and connect to server
    virtual bool Init(const CSmallString& server_name);

// enumeration -----------------------------------------------------------------
    //! init queue list
    virtual bool GetQueues(CQueueList& queues);

    //! init node list
    virtual bool GetNodes(CNodeList& nodes);

    //! init all job list
    virtual bool GetAllJobs(CJobList& jobs);

    //! get queue jobs
    virtual bool GetQueueJobs(CJobList& jobs,const CSmallString& queue_name);

    //! init job list of given user
    virtual bool GetUserJobs(CJobList& jobs,const CSmallString& user);

    //! init job list by job id
    virtual bool GetJob(CJobList& jobs,const CSmallString& jobid);

    //! get job by job id
    virtual const CJobPtr GetJob(const CSmallString& jobid);

    //! print technical info about queues
    virtual bool PrintQueues(std::ostream& sout);

    //! print technical info about nodes
    virtual bool PrintNodes(std::ostream& sout);

    //! print technical info about single node
    virtual bool PrintNode(std::ostream& sout,const CSmallString& name);

    //! print technical info about jobs
    virtual bool PrintJobs(std::ostream& sout);

    //! print technical info about single job
    virtual bool PrintJob(std::ostream& sout,const CSmallString& name);

// execution -------------------------------------------------------------------
    //! submit job
    virtual bool SubmitJob(CJob& job);

    //! get job status
    virtual bool GetJobStatus(CJob& job);

    //! kill job
    virtual bool KillJob(CJob& job);

    //! kill job by ID
    virtual bool KillJobByID(const CSmallString& jobid);

    //! get last error message
    virtual const std::string GetLastErrorMsg(void);
};

// -----------------------------------------------------------------------------

#endif
