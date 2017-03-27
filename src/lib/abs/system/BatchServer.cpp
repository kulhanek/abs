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

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

CBatchServer::CBatchServer(CPluginObject* p_objectinfo)
    : CComObject(p_objectinfo)
{
}

//------------------------------------------------------------------------------

CBatchServer::~CBatchServer(void)
{

}

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

bool CBatchServer::Init(const CSmallString& server_name,const CSmallString& short_name)
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

bool CBatchServer::GetAllJobs(CJobList& jobs,bool finished)
{
    return(false);
}

//------------------------------------------------------------------------------

bool CBatchServer::GetQueueJobs(CJobList& jobs,const CSmallString& queue_name)
{
    return(false);
}

//------------------------------------------------------------------------------

bool CBatchServer::GetUserJobs(CJobList& jobs,const CSmallString& user)
{
    return(false);
}

//------------------------------------------------------------------------------

bool CBatchServer::GetJob(CJobList& jobs,const CSmallString& jobid)
{
    CJobPtr job = GetJob(jobid);
    if( job != NULL ){
        jobs.push_back(job);
        return(true);
    }
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

bool CBatchServer::PrintJobs(std::ostream& sout)
{
    return(true);
}

//------------------------------------------------------------------------------

bool CBatchServer::PrintJob(std::ostream& sout,const CSmallString& name)
{
    return(true);
}

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

bool CBatchServer::SubmitJob(CJob& job)
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

const CSmallString CBatchServer::GetLastErrorMsg(void)
{
    return("");
}

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

