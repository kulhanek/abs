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
#include <ABSConfig.hpp>
#include <XMLElement.hpp>
#include <iomanip>
#include <ErrorSystem.hpp>
#include <PluginDatabase.hpp>
#include <BatchServer.hpp>

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

bool CBatchServers::InitAll(void)
{
    CXMLElement* p_ele = ABSConfig.GetServerGroupConfig();
    if( p_ele == NULL ) {
        ES_ERROR("no servers");
        return(false);
    }

    CSmallString plugin;
    p_ele->GetAttribute("plugin",plugin);

    CXMLElement* p_sele = p_ele->GetFirstChildElement("server");
    while( p_sele != NULL ){

        CSmallString name, short_name;
        p_ele->GetAttribute("name",name);
        p_ele->GetAttribute("short",short_name);

        // create plugin object
        CComObject* p_obj = PluginDatabase.CreateObject(CExtUUID(plugin));
        if( p_obj == NULL ){
            ES_ERROR("unable to create checker object");
            return(false);
        }

        CBatchServerPtr plg_obj(dynamic_cast<CBatchServer*>(p_obj));
        if( plg_obj == NULL ){
            delete p_obj;
            ES_ERROR("object is not of correct type");
            return(false);
        }

        if( plg_obj->Init(name,short_name) == false ){
            CSmallString error;
            error << "unable to init server '" << name << "' (" << short_name << ")";
            ES_ERROR(error);
            return(false);
        }

        push_back(plg_obj);
        p_sele = p_sele->GetNextSiblingElement();
    }

    return(true);
}

//------------------------------------------------------------------------------

bool CBatchServers::Init(const CSmallString& srv)
{
    CXMLElement* p_ele = ABSConfig.GetServerGroupConfig();
    if( p_ele == NULL ) {
        ES_ERROR("no servers");
        return(false);
    }

    CSmallString plugin;
    p_ele->GetAttribute("plugin",plugin);

    CXMLElement* p_sele = p_ele->GetFirstChildElement("server");
    while( p_sele != NULL ){

        CSmallString name, short_name;
        p_ele->GetAttribute("name",name);
        p_ele->GetAttribute("short",short_name);

        if( (name == srv) || (short_name == srv) ){
            // create plugin object
            CComObject* p_obj = PluginDatabase.CreateObject(CExtUUID(plugin));
            if( p_obj == NULL ){
                ES_ERROR("unable to create checker object");
                return(false);
            }

            CBatchServerPtr plg_obj(dynamic_cast<CBatchServer*>(p_obj));
            if( plg_obj == NULL ){
                delete p_obj;
                ES_ERROR("object is not of correct type");
                return(false);
            }

            if( plg_obj->Init(name,short_name) == false ){
                CSmallString error;
                error << "unable to init server '" << name << "' (" << short_name << ")";
                ES_ERROR(error);
                return(false);
            }

            push_back(plg_obj);
        }

        p_sele = p_sele->GetNextSiblingElement();
    }

    return(true);
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
    CXMLElement* p_ele = ABSConfig.GetServerGroupConfig();
    if( p_ele ){
        p_ele = p_ele->GetFirstChildElement("server");
    }
    while( p_ele != NULL ){
    vout << "# Batch servers ..." << endl;
        CSmallString name, short_name;
        p_ele->GetAttribute("name",name);
        p_ele->GetAttribute("short",short_name);
        vout << "# " << setw(1) << short_name << " " << name << endl;
        p_ele = p_ele->GetNextSiblingElement();
    }
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

void CBatchServers::PrintQueues(std::ostream& sout)
{
    // init servers if not done already
    if( size() == 0 ) InitAll();

    sout << "# Site name     : " << AMSGlobalConfig.GetActiveSiteName() << endl;
    sout <<  endl;

    // list queues
    std::list<CBatchServerPtr>::iterator    it = begin();
    std::list<CBatchServerPtr>::iterator    ie = end();

    while( it != ie ){
        CBatchServerPtr srv_ptr = *it;
    sout << "# Batch server  : " << srv_ptr->GetServerName() << " (" << srv_ptr->GetShortName() << ")" << endl;
        srv_ptr->PrintQueues(sout);
        it++;
    }
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

