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
typedef struct batch_status* (*PBS_STATHOST)(int,char*,struct attrl*,char*);
typedef struct batch_status* (*PBS_STATVNODE)(int,char*,struct attrl*,char*);
typedef struct batch_status* (*PBS_STATJOB)(int,char*,struct attrl*,char*);
// the last argument is due to PBSPro
typedef struct batch_status* (*PBS_SELSTATJOB)(int,struct attropl*,char*,char*);
typedef void (*PBS_STATFREE)(struct batch_status *);
typedef char* (*PBS_SUBMIT)(int,struct attropl*,const char*,const char*,char*);
typedef char* (*PBS_GETERRMSG)(int connect);
typedef char* (*PBS_LOCJOB)(int,char*,char*);
typedef int (*PBS_DELJOB)(int,char*,char*);
typedef char* (*PBS_STRERROR)(int);
typedef int (*PBS_ERRNO)(void);

// -----------------------------------------------------------------------------

class ABS_PACKAGE CPBSProServer : public CBatchServer {
public:
// constructor -----------------------------------------------------------------
        CPBSProServer(void);
        ~CPBSProServer(void);

// init batch system subsystem -------------------------------------------------------
    //! load symbols and connect to server
    bool Init(const CSmallString& server_name,const CSmallString& short_name);

// enumeration -----------------------------------------------------------------
    //! init queue list
    bool GetQueues(CQueueList& queues);

    //! init node list
    bool GetNodes(CNodeList& nodes);

    //! get given node
    const CNodePtr GetNode(const CSmallString& name);

    //! init all job list
    bool GetAllJobs(CJobList& jobs,bool include_history);

    //! get queue jobs
    bool GetQueueJobs(CJobList& jobs,const CSmallString& queue_name,bool include_history);

    //! init job list of given user
    bool GetUserJobs(CJobList& jobs,const CSmallString& user,bool include_history);

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
    bool PrintJobs(std::ostream& sout,bool include_history);

    //! print technical info about single job
    bool PrintJob(std::ostream& sout,const CSmallString& jobid,bool include_history);

    //! locate job - return server name for the job
    const CSmallString LocateJob(const CSmallString& jobid);

// execution -------------------------------------------------------------------
    //! init specific resources for batch server
    bool InitBatchResources(CResourceList* p_rl);

    //! submit job
    bool SubmitJob(CJob& job,bool verbose);

    //! get job status
    bool GetJobStatus(CJob& job);

    //! kill job
    bool KillJob(CJob& job);

    //! kill job by ID
    bool KillJobByID(const CSmallString& jobid);

    //! get last error message
    const CSmallString GetLastErrorMsg(void);

    //! decode batch attributes
    void CreateJobAttributes(struct attropl* &p_prev,CResourceList* p_rl);

// section of private data -----------------------------------------------------
private:
    CSmallString    PBSProLibName;
    CDynamicPackage PBSProLib;
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

    // batch system api symbols
    PBS_CONNECT     pbspro_connect;
    PBS_DISCONNECT  pbspro_disconnect;
    PBS_STATSERVER  pbspro_statserver;
    PBS_STATQUE     pbspro_statque;
    PBS_STATHOST    pbspro_stathost;
    PBS_STATVNODE   pbspro_statvnode;
    PBS_STATJOB     pbspro_statjob;
    PBS_SELSTATJOB  pbspro_selstat;
    PBS_STATFREE    pbspro_statfree;
    PBS_SUBMIT      pbspro_submit;
    PBS_DELJOB      pbspro_deljob;
    PBS_GETERRMSG   pbspro_geterrmsg;
    PBS_STRERROR    pbspro_strerror;
    PBS_LOCJOB      pbspro_locjob;
    PBS_ERRNO       pbspro_errno;
};

// -----------------------------------------------------------------------------

extern CPBSProServer PBSPro;

// -----------------------------------------------------------------------------

#endif
