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

#include <BatchServers.hpp>
#include <Job.hpp>
#include <JobList.hpp>
#include <AMSGlobalConfig.hpp>

//------------------------------------------------------------------------------

using namespace std;

//------------------------------------------------------------------------------

CBatchServers BatchServers;

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

CBatchServers::CBatchServers(void)
{
}

//------------------------------------------------------------------------------

CBatchServers::~CBatchServers()
{
}

//------------------------------------------------------------------------------

bool CBatchServers::Init(void)
{
    return(false);
}

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

bool CBatchServers::IsServerAvailable(const CSmallString& name)
{
    return(false);
}

//------------------------------------------------------------------------------

void CBatchServers::PrintServerOverview(std::ostream& vout)
{
    vout << "# Site name     : " << AMSGlobalConfig.GetActiveSiteName() << endl;
   // vout << "# Torque server : " << GetServerName() << endl;
}

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

bool CBatchServers::GetQueues(void)
{
    return(false);
}

//------------------------------------------------------------------------------

bool CBatchServers::GetNodes(void)
{
    return(false);
}

//------------------------------------------------------------------------------

bool CBatchServers::GetAllJobs(CJobList& jobs)
{
    return(false);
}

//------------------------------------------------------------------------------

bool CBatchServers::GetQueueJobs(CJobList& jobs,const CSmallString& queue_name)
{
    return(false);
}

//------------------------------------------------------------------------------

bool CBatchServers::GetUserJobs(CJobList& jobs,const CSmallString& user)
{
    return(false);
}

//------------------------------------------------------------------------------

bool CBatchServers::GetJob(CJobList& jobs,const CSmallString& jobid)
{
    return(false);
}

//------------------------------------------------------------------------------

const CJobPtr CBatchServers::GetJob(const CSmallString& jobid)
{
    return(CJobPtr());
}

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

bool CBatchServers::SubmitJob(CJob& job)
{
    return(false);
}

//------------------------------------------------------------------------------

bool CBatchServers::GetJobStatus(CJob& job)
{
    return(false);
}

//------------------------------------------------------------------------------

bool CBatchServers::KillJob(CJob& job)
{
    return(false);
}

//------------------------------------------------------------------------------

bool CBatchServers::KillJobByID(const CSmallString& jobid)
{
    return(false);
}

//------------------------------------------------------------------------------

const CSmallString CBatchServers::GetLastErrorMsg(void)
{
    return("");
}

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

bool CBatchServers::PrintQueues(std::ostream& sout)
{
    return(false);
}

//------------------------------------------------------------------------------

bool CBatchServers::PrintNodes(std::ostream& sout)
{
    return(false);
}

//------------------------------------------------------------------------------

bool CBatchServers::PrintNode(std::ostream& sout,const CSmallString& name)
{
    return(false);
}

//------------------------------------------------------------------------------

bool CBatchServers::PrintJobs(std::ostream& sout)
{
    return(false);
}

//------------------------------------------------------------------------------

bool CBatchServers::PrintJob(std::ostream& sout,const CSmallString& name)
{
    return(false);
}

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

