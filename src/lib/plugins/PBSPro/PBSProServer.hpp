#ifndef PBSProServerH
#define PBSProServerH
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
#include <DynamicPackage.hpp>
#include <iostream>
#include <Job.hpp>
#include <BatchServer.hpp>

// -----------------------------------------------------------------------------

class CQueueList;
class CNodeList;
class CJobList;
class CJob;

// -----------------------------------------------------------------------------

typedef int (*PBS_CONNECT)(const char*);
typedef int (*PBS_DISCONNECT)(int);
typedef struct batch_status* (*PBS_STATSERVER)(int,struct attrl*,char*);
typedef struct batch_status* (*PBS_STATQUE)(int,char*,struct attrl*,char*);
typedef struct batch_status* (*PBS_STATNODE)(int,char*,struct attrl*,char*);
typedef struct batch_status* (*PBS_STATJOB)(int,char*,struct attrl*,char*);
// the last argument is due to PBSPro
typedef struct batch_status* (*PBS_SELSTATJOB)(int,struct attropl*,char*,char*);
typedef void (*PBS_STATFREE)(struct batch_status *);
typedef char* (*PBS_SUBMIT)(int,struct attropl*,const char*,const char*,char*);
typedef char* (*PBS_GETERRMSG)(int connect);
typedef int (*PBS_DELJOB)(int,char*,char*);
typedef char* (*PBS_STRERROR)(int);
typedef int* PBS_ERRNO;

// -----------------------------------------------------------------------------

class ABS_PACKAGE CPBSProServer : public CBatchServer {
public:
// constructor -----------------------------------------------------------------
        CPBSProServer(void);
        ~CPBSProServer(void);

// init torque subsystem -------------------------------------------------------
    //! load symbols and connect to server
    bool Init(const CSmallString& server_name,const CSmallString& short_name);

// enumeration -----------------------------------------------------------------
    //! init queue list
    bool GetQueues(CQueueList& queues);

    //! init node list
    bool GetNodes(CNodeList& nodes);

    //! init all job list
    bool GetAllJobs(CJobList& jobs,bool finished);

    //! get queue jobs
    bool GetQueueJobs(CJobList& jobs,const CSmallString& queue_name,bool finished);

    //! init job list of given user
    bool GetUserJobs(CJobList& jobs,const CSmallString& user,bool finished);

    //! init job list by job id
    bool GetJob(CJobList& jobs,const CSmallString& jobid);

    //! get job by job id
    const CJobPtr GetJob(const CSmallString& jobid);

    //! print technical info about queues
    bool PrintQueues(std::ostream& sout);

    //! print technical info about nodes
    bool PrintNodes(std::ostream& sout);

    //! print technical info about single node
    bool PrintNode(std::ostream& sout,const CSmallString& name);

    //! print technical info about jobs
    bool PrintJobs(std::ostream& sout);

    //! print technical info about single job
    bool PrintJob(std::ostream& sout,const CSmallString& name);

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

    //! get job comment from comment, planned_start, planned_nodes
    static void DecodeBatchJobComment(attrl* p_item,CSmallString& comment);

// section of private data -----------------------------------------------------
private:
    CSmallString    TorqueLibName;
    CDynamicPackage TorqueLib;
    int             ServerID;

    // init symbols
    bool InitSymbols(void);

    // connect to server
    bool ConnectToServer(void);

    // disconnect from server
    bool DisconnectFromServer(void);

    // print batch_status
    void PrintBatchStatus(std::ostream& sout,struct batch_status* p_bs);

    // print attributes
    void PrintAttributes(std::ostream& sout,struct attrl* p_as);

    // print attributes
    void PrintAttributes(std::ostream& sout,struct attropl* p_as);

    // torque api symbols
    PBS_CONNECT     pbs_connect;
    PBS_DISCONNECT  pbs_disconnect;
    PBS_STATSERVER  pbs_statserver;
    PBS_STATQUE     pbs_statque;
    PBS_STATNODE    pbs_statnode;
    PBS_STATJOB     pbs_statjob;
    PBS_SELSTATJOB  pbs_selstat;
    PBS_STATFREE    pbs_statfree;
    PBS_SUBMIT      pbs_submit;
    PBS_DELJOB      pbs_deljob;
    PBS_GETERRMSG   pbs_geterrmsg;
    PBS_STRERROR    pbs_strerror;
    PBS_ERRNO       pbs_errno;
};

// -----------------------------------------------------------------------------

extern CPBSProServer Torque;

// -----------------------------------------------------------------------------

#endif
