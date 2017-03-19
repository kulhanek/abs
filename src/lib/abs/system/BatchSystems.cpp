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

#include <BatchSystems.hpp>
#include <Job.hpp>
#include <JobList.hpp>

//------------------------------------------------------------------------------

CBatchSystems BatchSystems;

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

CBatchSystems::CBatchSystems(void)
{
}

//------------------------------------------------------------------------------

CBatchSystems::~CBatchSystems()
{
}

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

bool CBatchSystems::Init(void)
{
    return(false);
}

//------------------------------------------------------------------------------

bool CBatchSystems::GetQueues(void)
{
    return(false);
}

//------------------------------------------------------------------------------

bool CBatchSystems::GetNodes(void)
{
    return(false);
}

//------------------------------------------------------------------------------

bool CBatchSystems::GetAllJobs(CJobList& jobs)
{
    return(false);
}

//------------------------------------------------------------------------------

bool CBatchSystems::GetQueueJobs(CJobList& jobs,const CSmallString& queue_name)
{
    return(false);
}

//------------------------------------------------------------------------------

bool CBatchSystems::GetUserJobs(CJobList& jobs,const CSmallString& user)
{
    return(false);
}

//------------------------------------------------------------------------------

bool CBatchSystems::GetJob(CJobList& jobs,const CSmallString& jobid)
{
    return(false);
}

//------------------------------------------------------------------------------

const CJobPtr CBatchSystems::GetJob(const CSmallString& jobid)
{
    return(CJobPtr());
}

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

bool CBatchSystems::SubmitJob(CJob& job)
{
    return(false);
}

//------------------------------------------------------------------------------

bool CBatchSystems::GetJobStatus(CJob& job)
{
    return(false);
}

//------------------------------------------------------------------------------

bool CBatchSystems::KillJob(CJob& job)
{
    return(false);
}

//------------------------------------------------------------------------------

bool CBatchSystems::KillJobByID(const CSmallString& jobid)
{
    return(false);
}

//------------------------------------------------------------------------------

const CSmallString CBatchSystems::GetLastErrorMsg(void)
{
    return("");
}

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

bool CBatchSystems::PrintQueues(std::ostream& sout)
{
    return(false);
}

//------------------------------------------------------------------------------

bool CBatchSystems::PrintNodes(std::ostream& sout)
{
    return(false);
}

//------------------------------------------------------------------------------

bool CBatchSystems::PrintNode(std::ostream& sout,const CSmallString& name)
{
    return(false);
}

//------------------------------------------------------------------------------

bool CBatchSystems::PrintJobs(std::ostream& sout)
{
    return(false);
}

//------------------------------------------------------------------------------

bool CBatchSystems::PrintJob(std::ostream& sout,const CSmallString& name)
{
    return(false);
}

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

