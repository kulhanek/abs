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
#include <VerboseStr.hpp>
#include <TerminalStr.hpp>

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

    /// get default server name
    const CSmallString GetDefaultSrvName(void);

    /// get batch server - srv_name can be full or short name
    const CBatchServerPtr FindBatchServer(const CSmallString& srv_name,bool init=false);

    /// return number of batch servers
    size_t GetNumberOfServers(void) const;

    /// setup RetrySeverInit
    static void SetServerInitRetryMode(bool set);

// -----------------------------------------------------------------------------
    /// print batch systems
    void PrintServerOverview(std::ostream& vout);

    /// print warning if any server is inaccessible
    void PrintWarningIfUnavailable(std::ostream& vout);

    /// return name of the generic resource
    const CSmallString GetGenericResourceName(void);

    /// decode queue name
    bool DecodeQueueName(const CSmallString& input,CSmallString& srv_name,CSmallString& srv_short,CSmallString& queue);

// -----------------------------------------------------------------------------
    /// list queues
    bool GetQueues(void);

    /// list nodes
    bool GetNodes(void);

    //! init all job list
    bool GetAllJobs(bool include_history);

    //! get queue jobs
    bool GetQueueJobs(const CSmallString& queue_name,bool include_history);

    //! init job list of given user
    bool GetUserJobs(const CSmallString& user,bool include_history);

    //! init job list by job id
    bool GetJob(CJobList& jobs,const CSmallString& jobid);

    //! get job by job id
    const CJobPtr GetJob(const CSmallString& jobid);

// execution -------------------------------------------------------------------
    //! submit job
    bool SubmitJob(CJob& job,bool verbose);

    //! get job status
    bool GetJobStatus(CJob& job);

    //! kill job
    bool KillJob(CJob& job);

    //! kill job by ID
    bool KillJobByID(const CSmallString& jobid);

// technical print -------------------------------------------------------------
    //! print technical info about queues
    void PrintQueues(std::ostream& sout);

    //! print technical info about nodes
    void PrintNodes(std::ostream& sout);

    //! print technical info about single node
    bool PrintNode(std::ostream& sout,const CSmallString& name);

    //! print technical info about jobs
    void PrintJobs(std::ostream& sout);

    //! print technical info about single job
    bool PrintJob(std::ostream& sout,const CSmallString& name);

// private data ----------------------------------------------------------------
private:
    /// find batch server by queue
    /// name@server           short or long name supported
    /// name                  for default server
    /// the server name is stripped from the queue name
    const CBatchServerPtr FindBatchServerByQueue(CSmallString& name);

    /// find batch server by node
    /// name@server           short or long name supported
    /// name                  for default server
    /// the server name is stripped from the node name
    const CBatchServerPtr FindBatchServerByNode(CSmallString& name);

    /// find batch server by jobid
    /// number.server           1234.wagap-pro.cerit-sc.cz
    /// numberSERVER_SHORT_NAME 1234C
    /// number                  for default server
    /// it resolves moved jobs - see RT#258670
    /// the server name is added to the jobid if not present
    const CBatchServerPtr FindBatchServerByJobID(CSmallString& jobid);

    /// find batch server by jobid
    /// same as above
    const CBatchServerPtr FindBatchServerByJobID(CJob& job);

    // global setup
    CTerminalStr    Console;
    CVerboseStr     vout;
    static bool     RetrySeverInit; // if server initialization fails, retry
};

//------------------------------------------------------------------------------

extern CBatchServers BatchServers;

// -----------------------------------------------------------------------------

#endif
