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

#include <BatchClient.hpp>
#include <Job.hpp>
#include <JobList.hpp>

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

CBatchClient::CBatchClient(CPluginObject* p_objectinfo)
    : CComObject(p_objectinfo)
{
}

//------------------------------------------------------------------------------

CBatchClient::~CBatchClient(void)
{

}

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

bool CBatchClient::Init(const CSmallString& server_name)
{
    return(false);
}

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

bool CBatchClient::GetQueues(CQueueList& queues)
{
    return(false);
}

//------------------------------------------------------------------------------

bool CBatchClient::GetNodes(CNodeList& nodes)
{
    return(false);
}

//------------------------------------------------------------------------------

bool CBatchClient::GetAllJobs(CJobList& jobs)
{
    return(false);
}

//------------------------------------------------------------------------------

bool CBatchClient::GetQueueJobs(CJobList& jobs,const CSmallString& queue_name)
{
    return(false);
}

//------------------------------------------------------------------------------

bool CBatchClient::GetUserJobs(CJobList& jobs,const CSmallString& user)
{
    return(false);
}

//------------------------------------------------------------------------------

bool CBatchClient::GetJob(CJobList& jobs,const CSmallString& jobid)
{
    CJobPtr job = GetJob(jobid);
    if( job != NULL ){
        jobs.push_back(job);
        return(true);
    }
    return(false);
}

//------------------------------------------------------------------------------

const CJobPtr CBatchClient::GetJob(const CSmallString& jobid)
{
    return(CJobPtr());
}

//------------------------------------------------------------------------------

bool CBatchClient::PrintQueues(std::ostream& sout)
{
    return(true);
}

//------------------------------------------------------------------------------

bool CBatchClient::PrintNodes(std::ostream& sout)
{
    return(true);
}

//------------------------------------------------------------------------------

bool CBatchClient::PrintNode(std::ostream& sout,const CSmallString& name)
{
    return(true);
}

//------------------------------------------------------------------------------

bool CBatchClient::PrintJobs(std::ostream& sout)
{
    return(true);
}

//------------------------------------------------------------------------------

bool CBatchClient::PrintJob(std::ostream& sout,const CSmallString& name)
{
    return(true);
}

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

bool CBatchClient::SubmitJob(CJob& job)
{
    return(false);
}

//------------------------------------------------------------------------------

bool CBatchClient::GetJobStatus(CJob& job)
{
    return(false);
}

//------------------------------------------------------------------------------

bool CBatchClient::KillJob(CJob& job)
{
    return(false);
}

//------------------------------------------------------------------------------

bool CBatchClient::KillJobByID(const CSmallString& jobid)
{
    return(false);
}

//------------------------------------------------------------------------------

const std::string CBatchClient::GetLastErrorMsg(void)
{
    return("");
}

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

