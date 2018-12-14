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

#include <BatchServer.hpp>
#include <Job.hpp>
#include <JobList.hpp>
#include <boost/algorithm/string/split.hpp>
#include <boost/algorithm/string/classification.hpp>

//------------------------------------------------------------------------------

using namespace std;
using namespace boost;

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

CBatchServer::CBatchServer(CPluginObject* p_objectinfo)
    : CComObject(p_objectinfo)
{
    ExecTime = 0;
    TimerStart = 0;
    AllowJobTransfer = true;
}

//------------------------------------------------------------------------------

CBatchServer::~CBatchServer(void)
{

}

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

bool CBatchServer::Init(const CSmallString& server_name,const CSmallString& short_name,
                        const CSmallString& alt_names,bool job_transfer)
{
    return(false);
}

//------------------------------------------------------------------------------

const CSmallString CBatchServer::GetServerName(void)
{
    return(ServerName);
}

//------------------------------------------------------------------------------

const CSmallString CBatchServer::GetShortName(void)
{
    return(ShortName);
}

//------------------------------------------------------------------------------

bool CBatchServer::DoesItMatchName(const CSmallString& srvname)
{
    if( srvname == ServerName ) return(true);
    if( srvname == ShortName ) return(true);

    std::vector<std::string> alt_names;
    std::string salts(AltNames);
    split(alt_names, salts, is_any_of(":"));

    if( std::find(alt_names.begin(), alt_names.end(), string(srvname)) != alt_names.end() ){
        return(true);
    }

    return(false);
}

//------------------------------------------------------------------------------

const CSmallString CBatchServer::GetFullJobID(const CSmallString& jobid)
{
    if( jobid.FindSubString(".") != -1 ) return(jobid);
    CSmallString full_job_id;
    full_job_id << jobid << "." << ServerName;
    return(full_job_id);
}

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

bool CBatchServer::GetQueues(CQueueList& queues)
{
    return(false);
}

//------------------------------------------------------------------------------

bool CBatchServer::GetNodes(CNodeList& nodes)
{
    return(false);
}

//------------------------------------------------------------------------------

const CNodePtr CBatchServer::GetNode(const CSmallString& name)
{
    return(CNodePtr());
}

//------------------------------------------------------------------------------

bool CBatchServer::GetAllJobs(CJobList& jobs,bool include_history)
{
    return(false);
}

//------------------------------------------------------------------------------

bool CBatchServer::GetQueueJobs(CJobList& jobs,const CSmallString& queue_name,bool include_history)
{
    return(false);
}

//------------------------------------------------------------------------------

bool CBatchServer::GetUserJobs(CJobList& jobs,const CSmallString& user,bool include_history)
{
    return(false);
}

//------------------------------------------------------------------------------

const CJobPtr CBatchServer::GetJob(const CSmallString& jobid)
{
    return(CJobPtr());
}

//------------------------------------------------------------------------------

bool CBatchServer::PrintQueues(std::ostream& sout)
{
    return(true);
}

//------------------------------------------------------------------------------

bool CBatchServer::PrintNodes(std::ostream& sout)
{
    return(true);
}

//------------------------------------------------------------------------------

bool CBatchServer::PrintNode(std::ostream& sout,const CSmallString& name)
{
    return(true);
}

//------------------------------------------------------------------------------

bool CBatchServer::PrintJobs(std::ostream& sout,bool include_history)
{
    return(true);
}

//------------------------------------------------------------------------------

bool CBatchServer::PrintJob(std::ostream& sout,const CSmallString& name,bool include_history)
{
    return(true);
}

//------------------------------------------------------------------------------

const CSmallString CBatchServer::LocateJob(const CSmallString& jobid)
{
    return("");
}

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

bool CBatchServer::InitBatchResources(CResourceList* p_rl)
{
    return(true);
}

//------------------------------------------------------------------------------

bool CBatchServer::SubmitJob(CJob& job,bool verbose)
{
    return(false);
}

//------------------------------------------------------------------------------

bool CBatchServer::GetJobStatus(CJob& job)
{
    return(false);
}

//------------------------------------------------------------------------------

bool CBatchServer::KillJob(CJob& job)
{
    return(false);
}

//------------------------------------------------------------------------------

bool CBatchServer::KillJobByID(const CSmallString& jobid)
{
    return(false);
}

//------------------------------------------------------------------------------

void CBatchServer::StartTimer(void)
{
    TimerStart =  clock();
}

//------------------------------------------------------------------------------

void CBatchServer::EndTimer(void)
{
    ExecTime += clock() - TimerStart;
}

//------------------------------------------------------------------------------

double CBatchServer::GetExecTime(void)
{
    return( (double)ExecTime / (double)CLOCKS_PER_SEC );
}

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

