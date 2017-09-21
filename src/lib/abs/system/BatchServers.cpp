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
#include <JobList.hpp>
#include <NodeList.hpp>
#include <QueueList.hpp>
#include <Shell.hpp>
#include <cctype>

//------------------------------------------------------------------------------

using namespace std;

//------------------------------------------------------------------------------

bool CBatchServers::RetrySeverInit = false;

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

CBatchServers::CBatchServers(void)
{
    vout.Attach(Console);
    vout.Verbosity(CVerboseStr::low);
    vout << low;
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

    CSmallString src = ABSConfig.GetSystemConfigItem("INF_RETRY_COUNT");
    CSmallString srt = ABSConfig.GetSystemConfigItem("INF_RETRY_TIME");
    int rc = 3;
    if( src != NULL ){
        rc = src.ToInt();
    }
    if( rc < 0 ) rc = 0;
    if( RetrySeverInit == false ) rc = 0; // no retry init

    int rt = 600;
    if( srt != NULL ){
        rt = srt.ToInt();
    }

    for(int i = 0; i <= rc; i++){

        bool result = true;

        CXMLElement* p_sele = p_ele->GetFirstChildElement("server");
        while( p_sele != NULL ){

            CSmallString name, short_name;
            p_sele->GetAttribute("name",name);
            p_sele->GetAttribute("short",short_name);

            p_sele = p_sele->GetNextSiblingElement();

            // create plugin object
            CComObject* p_obj = PluginDatabase.CreateObject(CExtUUID(plugin));
            if( p_obj == NULL ){
                ES_ERROR("unable to create batch server object");
                return(false); // this is FATAL error in configuration
            }

            CBatchServerPtr plg_obj(dynamic_cast<CBatchServer*>(p_obj));
            if( plg_obj == NULL ){
                delete p_obj;
                ES_ERROR("object is not of correct type");
                return(false); // this is FATAL error in configuration
            }

            if( plg_obj->Init(name,short_name) == false ){
                CSmallString error;
                error << "unable to init server '" << name << "' (" << short_name << ")";
                ES_TRACE_ERROR(error);
                result = false;
                continue;
            }

            push_back(plg_obj);
        }

        if( result == true ) return(true); // all is setup correctly

        if( rc == 0 ) break; // no retry allowed - exit with error
        if( i == rc ) break; // do not wait - no additional attempt - exit with error

        vout << endl;
        vout << "ERROR: Initialization of batch servers was not sucessfull! I will retry in " << rt << " seconds." << endl;
        vout << endl;
        sleep(rt);
    }

    ES_ERROR("unable to init batch servers");

    return(false);
}

//------------------------------------------------------------------------------

const CSmallString CBatchServers::GetGenericResourceName(void)
{
    CXMLElement* p_ele = ABSConfig.GetServerGroupConfig();
    if( p_ele == NULL ) {
        ES_ERROR("no servers");
        return(false);
    }

    CSmallString genres;
    p_ele->GetAttribute("genres",genres);
    return(genres);
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

    CSmallString src = ABSConfig.GetSystemConfigItem("INF_RETRY_COUNT");
    CSmallString srt = ABSConfig.GetSystemConfigItem("INF_RETRY_TIME");
    int rc = 3;
    if( src != NULL ){
        rc = src.ToInt();
    }
    if( rc < 0 ) rc = 0;
    if( RetrySeverInit == false ) rc = 0; // no retry init

    int rt = 600;
    if( srt != NULL ){
        rt = srt.ToInt();
    }

    for(int i = 0; i <= rc; i++){

        bool result = false;

        CXMLElement* p_sele = p_ele->GetFirstChildElement("server");
        while( p_sele != NULL ){

            CSmallString name, short_name;
            p_sele->GetAttribute("name",name);
            p_sele->GetAttribute("short",short_name);

            if( (name == srv) || (short_name == srv) ){
                // create plugin object
                CComObject* p_obj = PluginDatabase.CreateObject(CExtUUID(plugin));
                if( p_obj == NULL ){
                    ES_ERROR("unable to create batch server object");
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
                    ES_TRACE_ERROR(error);
                    result = false;
                    break;
                }

                push_back(plg_obj);
                result = true;
                break;
            }

            p_sele = p_sele->GetNextSiblingElement();
        }

        if( result == true ) return(true); // all is setup correctly

        if( rc == 0 ) break; // no retry allowed - exit with error
        if( i == rc ) break; // do not wait - no additional attempt - exit with error

        vout << endl;
        vout << "ERROR: Initialization of batch server was not sucessfull! I will retry in " << rt << " seconds." << endl;
        vout << endl;
        sleep(rt);
    }

    ES_ERROR("unable to init batch server");
    return(false);
}

//------------------------------------------------------------------------------

const CSmallString CBatchServers::GetDefaultSrvName(void)
{
    CSmallString srv_name;

    CXMLElement* p_ele = ABSConfig.GetServerGroupConfig();
    if( p_ele == NULL ) {
        ES_ERROR("no servers");
        return(srv_name);
    }

    // is PBS_SERVER set?
    srv_name = CShell::GetSystemVariable("PBS_SERVER");
    if( srv_name ){
        // is it in the list of supported servers?
        CXMLElement* p_sele = p_ele->GetFirstChildElement("server");
        while( p_sele != NULL ){
            CSmallString name, short_name;
            p_sele->GetAttribute("name",name);
            p_sele->GetAttribute("short",short_name);
            if( srv_name == name ) return(srv_name);
            p_sele = p_sele->GetNextSiblingElement();
        }
    }

    // get default or the first server
    CXMLElement* p_sele = p_ele->GetFirstChildElement("server");
    bool set = false;
    while( p_sele != NULL ){
        CSmallString name, short_name;
        p_sele->GetAttribute("name",name);
        p_sele->GetAttribute("short",short_name);
        if( set == false ){
            srv_name = name;
            set = true;
        }
        bool def = false;
        p_sele->GetAttribute("default",def);
        if( def == true ){
            srv_name = name;
        }
        if( srv_name == name ) return(srv_name);
        p_sele = p_sele->GetNextSiblingElement();
    }

    return(srv_name);
}

//------------------------------------------------------------------------------

const CBatchServerPtr CBatchServers::FindBatchServer(const CSmallString& srv_name,bool create)
{
    // init servers if not done already
    // DO NOT INIT HERE - if( size() == 0 ) InitAll();
    // only when requested by create

    std::list<CBatchServerPtr>::iterator    it = begin();
    std::list<CBatchServerPtr>::iterator    ie = end();

    while( it != ie ){
        CBatchServerPtr srv_ptr = *it;
        if( (srv_ptr->GetServerName() == srv_name) || (srv_ptr->GetShortName() == srv_name) ){
            return(srv_ptr);
        }
        it++;
    }

    if( create == true ){
        if( Init(srv_name) == false ) return(CBatchServerPtr());
        return(back());
    }

    return(CBatchServerPtr());
}

//------------------------------------------------------------------------------

size_t CBatchServers::GetNumberOfServers(void) const
{
    return(size());
}

//------------------------------------------------------------------------------

void CBatchServers::SetServerInitRetryMode(bool set)
{
    RetrySeverInit = set;
}

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

void CBatchServers::PrintServerOverview(std::ostream& vout)
{
    // init servers if not done already
    if( size() == 0 ) InitAll();

    vout << "# Site name     : " << AMSGlobalConfig.GetActiveSiteName() << endl;
    CXMLElement* p_ele = ABSConfig.GetServerGroupConfig();
    if( p_ele ){
        p_ele = p_ele->GetFirstChildElement("server");
    }
    CSmallString default_srv = GetDefaultSrvName();

    vout << "# Batch servers ..." << endl;
    while( p_ele != NULL ){
        CSmallString name, short_name;
        p_ele->GetAttribute("name",name);
        p_ele->GetAttribute("short",short_name);
        if( FindBatchServer(name,false) == NULL ){
            // indicate that the server is not available
            vout << "<red>";
        }
        if( default_srv == name ){
            vout << "# -> * " << setw(1) << short_name << " " << name;
        } else {
            vout << "# ->   " << setw(1) << short_name << " " << name;
        }
        if( FindBatchServer(name,false) == NULL ){
            // indicate that the server is not available
            vout << " - ERROR - unable to connect to the server</red>";
        }
        vout << endl;
        p_ele = p_ele->GetNextSiblingElement();
    }
}

//------------------------------------------------------------------------------

void CBatchServers::PrintWarningIfUnavailable(std::ostream& vout)
{
// determine number of server configurations
    size_t config_srv = 0;
    CXMLElement* p_ele = ABSConfig.GetServerGroupConfig();
    if( p_ele ){
        p_ele = p_ele->GetFirstChildElement("server");
    }

    while( p_ele != NULL ){
        config_srv++;
        p_ele = p_ele->GetNextSiblingElement();
    }
// any discrepancy
    if( size() != config_srv ) {
        vout << endl;
        vout << "<b><blue> >>> WARNING: Incomplete results because some batch servers are not accessible!</blue></b>" << endl;
    }
}

//------------------------------------------------------------------------------

bool CBatchServers::DecodeQueueName(const CSmallString& input,CSmallString& srv_name,CSmallString& srv_short,CSmallString& queue)
{
    // queue            default server name
    // queue@S          short server name
    // queue@server     explicit server name

    srv_name = NULL;
    queue = input;

    int at = input.FindSubString("@");
    if( at >= 0 ){
        if( at + 1 < (int)input.GetLength() ){
            srv_name = input.GetSubString(at+1,input.GetLength()-(at+1));
        }
        if( at > 0 ){
            queue = input.GetSubString(0,at);
        } else {
            queue = NULL;
        }
    } else {
        // list all queues - do not report any error
        if( size() == 0 ) InitAll();
        GetQueues();
    }

    // not provided - use default server or unique queue from all servers
    if( srv_name == NULL ){
        CQueuePtr q_ptr = QueueList.FindQueueUnique(queue);
        if( q_ptr != NULL ){
            srv_name = q_ptr->GetShortServerName();
        } else {
            srv_name = GetDefaultSrvName();
        }
    }

    // check if server is supported by the site
    CXMLElement* p_ele = ABSConfig.GetServerGroupConfig();
    if( p_ele ){
        p_ele = p_ele->GetFirstChildElement("server");
    }
    while( p_ele != NULL ){
        CSmallString sname,sshort;
        p_ele->GetAttribute("name",sname);
        p_ele->GetAttribute("short",sshort);
        if( (sname == srv_name) || (sshort == srv_name) ){
            srv_name = sname;
            srv_short = sshort;
            return(true);
        }
        p_ele = p_ele->GetNextSiblingElement();
    }

    return((srv_name != NULL)&&(srv_short != NULL)&&(queue != NULL));
}

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

bool CBatchServers::GetQueues(void)
{
    // init servers if not done already
    if( size() == 0 ) InitAll();

    // list queues
    std::list<CBatchServerPtr>::iterator    it = begin();
    std::list<CBatchServerPtr>::iterator    ie = end();

    bool result = true;
    while( it != ie ){
        CBatchServerPtr srv_ptr = *it;
        result &= srv_ptr->GetQueues(QueueList);
        it++;
    }
    return(result);
}

//------------------------------------------------------------------------------

bool CBatchServers::GetNodes(void)
{
    // init servers if not done already
    if( size() == 0 ) InitAll();

    // list nodes
    std::list<CBatchServerPtr>::iterator    it = begin();
    std::list<CBatchServerPtr>::iterator    ie = end();

    bool result = true;
    while( it != ie ){
        CBatchServerPtr srv_ptr = *it;
        result &= srv_ptr->GetNodes(NodeList);
        it++;
    }
    return(result);
}

//------------------------------------------------------------------------------

bool CBatchServers::GetAllJobs(bool finished)
{
    // init servers if not done already
    if( size() == 0 ) InitAll();

    // list jobs
    std::list<CBatchServerPtr>::iterator    it = begin();
    std::list<CBatchServerPtr>::iterator    ie = end();

    bool result = true;
    while( it != ie ){
        CBatchServerPtr srv_ptr = *it;
        result &= srv_ptr->GetAllJobs(JobList,finished);
        it++;
    }
    return(result);
}

//------------------------------------------------------------------------------

bool CBatchServers::GetQueueJobs(const CSmallString& queue_name,bool finished)
{
    // init servers if not done already
    if( size() == 0 ) InitAll();

    CSmallString new_queue_name = queue_name;
    CBatchServerPtr srv_ptr = FindBatchServerByQueue(new_queue_name);
    if( srv_ptr == NULL ){
        CSmallString error;
        error << "batch server was not found for the queue '" << queue_name << "'";
        ES_ERROR(error);
        return(false);
    }

    return( srv_ptr->GetQueueJobs(JobList,new_queue_name,finished) );
}

//------------------------------------------------------------------------------

bool CBatchServers::GetUserJobs(const CSmallString& user,bool finished)
{
    // init servers if not done already
    if( size() == 0 ) InitAll();

    // list jobs
    std::list<CBatchServerPtr>::iterator    it = begin();
    std::list<CBatchServerPtr>::iterator    ie = end();

    bool result = true;
    while( it != ie ){
        CBatchServerPtr srv_ptr = *it;
        result &= srv_ptr->GetUserJobs(JobList,user,finished);
        it++;
    }
    return(result);
}

//------------------------------------------------------------------------------

bool CBatchServers::GetJob(CJobList& jobs,const CSmallString& jobid)
{
    CJobPtr job_ptr = GetJob(jobid);
    if( job_ptr == NULL ) return(false);
    jobs.push_back(job_ptr);
    return(true);
}

//------------------------------------------------------------------------------

const CJobPtr CBatchServers::GetJob(const CSmallString& jobid)
{
    CJobPtr job_ptr;

    CSmallString new_jobid = jobid;
    CBatchServerPtr srv_ptr = FindBatchServerByJobID(new_jobid);
    if( srv_ptr == NULL ){
        CSmallString error;
        error << "batch server was not found for the job '" << jobid << "'";
        ES_ERROR(error);
        return(job_ptr);
    }

    job_ptr = srv_ptr->GetJob(new_jobid);
    if( job_ptr == NULL ){
        CSmallString error;
        error << "unable to locate job '" << new_jobid << "'";
        ES_TRACE_ERROR(error);
        return(job_ptr);
    }

    return(job_ptr);
}

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

bool CBatchServers::SubmitJob(CJob& job,bool verbose)
{
    CBatchServerPtr srv_ptr = FindBatchServer(job.GetServerName(),true);
    if( srv_ptr == NULL ){
        ES_ERROR("no batch server was found for the job");
        return(false);
    }

    return(srv_ptr->SubmitJob(job,verbose));
}

//------------------------------------------------------------------------------

bool CBatchServers::GetJobStatus(CJob& job)
{
    CBatchServerPtr srv_ptr = FindBatchServer(job.GetServerName(),true);
    if( srv_ptr == NULL ){
        ES_ERROR("no batch server was found for the job");
        return(false);
    }

    return(srv_ptr->GetJobStatus(job));
}

//------------------------------------------------------------------------------

bool CBatchServers::KillJob(CJob& job)
{
    CBatchServerPtr srv_ptr = FindBatchServer(job.GetServerName(),true);
    if( srv_ptr == NULL ){
        ES_ERROR("no batch server was found for the job");
        return(false);
    }

    return(srv_ptr->KillJob(job));
}

//------------------------------------------------------------------------------

bool CBatchServers::KillJobByID(const CSmallString& jobid)
{
    CSmallString    new_jobid = jobid;
    CBatchServerPtr srv_ptr = FindBatchServerByJobID(new_jobid);
    if( srv_ptr == NULL ){
        CSmallString error;
        error << "batch server was not found for the job '" << jobid << "'";
        ES_ERROR(error);
        return(false);
    }

    return(srv_ptr->KillJobByID(new_jobid));
}

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

void CBatchServers::PrintQueues(std::ostream& sout)
{
    // init servers if not done already
    if( size() == 0 ) InitAll();

    // list queues
    std::list<CBatchServerPtr>::iterator    it = begin();
    std::list<CBatchServerPtr>::iterator    ie = end();

    while( it != ie ){
        CBatchServerPtr srv_ptr = *it;
    sout << endl;
    sout << "# Batch server  : " << srv_ptr->GetServerName() << " (" << srv_ptr->GetShortName() << ")" << endl;
    sout << "# ------------------------------------------------------------------------------" << endl;
        srv_ptr->PrintQueues(sout);
        it++;
    }
}

//------------------------------------------------------------------------------

void CBatchServers::PrintNodes(std::ostream& sout)
{
    // init servers if not done already
    if( size() == 0 ) InitAll();

    // list nodes
    std::list<CBatchServerPtr>::iterator    it = begin();
    std::list<CBatchServerPtr>::iterator    ie = end();

    while( it != ie ){
        CBatchServerPtr srv_ptr = *it;
    sout << endl;
    sout << "# Batch server  : " << srv_ptr->GetServerName() << " (" << srv_ptr->GetShortName() << ")" << endl;
    sout << "# ------------------------------------------------------------------------------" << endl;
        srv_ptr->PrintNodes(sout);
        it++;
    }
}

//------------------------------------------------------------------------------

bool CBatchServers::PrintNode(std::ostream& sout,const CSmallString& name)
{
    // init servers if not done already
    if( size() == 0 ) InitAll();

    CSmallString batch_name = name;
    CBatchServerPtr srv_ptr = FindBatchServerByNode(batch_name);

    if( srv_ptr == NULL ){
        ES_ERROR("no server found");
        return(false);
    }

    sout << endl;
    sout << "# Batch server  : " << srv_ptr->GetServerName() << " (" << srv_ptr->GetShortName() << ")" << endl;
    sout << "# ------------------------------------------------------------------------------" << endl;
    srv_ptr->PrintNode(sout,batch_name);

    return(false);
}

//------------------------------------------------------------------------------

void CBatchServers::PrintJobs(std::ostream& sout)
{
    // init servers if not done already
    if( size() == 0 ) InitAll();

    // list jobs
    std::list<CBatchServerPtr>::iterator    it = begin();
    std::list<CBatchServerPtr>::iterator    ie = end();

    while( it != ie ){
        CBatchServerPtr srv_ptr = *it;
    sout << endl;
    sout << "# Batch server  : " << srv_ptr->GetServerName() << " (" << srv_ptr->GetShortName() << ")" << endl;
    sout << "# ------------------------------------------------------------------------------" << endl;
        srv_ptr->PrintJobs(sout);
        it++;
    }
}

//------------------------------------------------------------------------------

bool CBatchServers::PrintJob(std::ostream& sout,const CSmallString& jobid)
{
    // init servers if not done already
    if( size() == 0 ) InitAll();

    CSmallString batch_jobid = jobid;
    CBatchServerPtr srv_ptr = FindBatchServerByJobID(batch_jobid);

    if( srv_ptr == NULL ){
        ES_ERROR("no server found");
        return(false);
    }

    sout << endl;
    sout << "# Batch server  : " << srv_ptr->GetServerName() << " (" << srv_ptr->GetShortName() << ")" << endl;
    sout << "# ------------------------------------------------------------------------------" << endl;
    srv_ptr->PrintJob(sout,batch_jobid);

    return(false);
}

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

const CBatchServerPtr CBatchServers::FindBatchServerByQueue(CSmallString& name)
{
    // queue            default server name
    // queue@S          short server name
    // queue@server     explicit server name

    CBatchServerPtr srv_ptr;
    if( name == NULL ) return(srv_ptr);

    CSmallString srv_name;
    CSmallString new_name = name;

    int at = name.FindSubString("@");
    if( at >= 0 ){
        if( at + 1 < (int)name.GetLength() ){
            srv_name = name.GetSubString(at+1,name.GetLength()-(at+1));
        }
        if( at > 0 ){
            new_name = name.GetSubString(0,at);
        } else {
            new_name = NULL;
        }
    }

    // update queue name
    name = new_name;

    // not provided - use default server
    if( srv_name == NULL ){
        srv_name = GetDefaultSrvName();
    }

    srv_ptr = FindBatchServer(srv_name,true);
    return(srv_ptr);
}

//------------------------------------------------------------------------------

const CBatchServerPtr CBatchServers::FindBatchServerByJobID(CSmallString& jobid)
{
    // jobid            default server name
    // jobidS           short server name
    // jobid.server     explicit server name

    CBatchServerPtr srv_ptr;
    if( jobid == NULL ) return(srv_ptr);

    CSmallString srv_name;
    CSmallString new_jobid = jobid;

    int at = jobid.FindSubString(".");
    if( at >= 0 ){
        if( at + 1 < (int)jobid.GetLength() ){
            srv_name = jobid.GetSubString(at+1,jobid.GetLength()-(at+1));
        }
        if( at > 0 ){
            new_jobid = jobid.GetSubString(0,at);
        } else {
            new_jobid = NULL;
        }
    }

    // update jobid name
    jobid = new_jobid;

    // is it in short form?
    if( jobid != NULL ){
        if( isalpha(jobid[jobid.GetLength()-1]) != 0 ){
            srv_name = jobid[jobid.GetLength()-1];
            if( jobid.GetLength() > 1 ){
                new_jobid = jobid.GetSubString(0,jobid.GetLength()-1);
            } else {
                new_jobid = NULL;
            }
        }
    }

    // update joid name
    jobid = new_jobid;

    // not provided - use default server
    if( srv_name == NULL ){
        srv_name = GetDefaultSrvName();
    }

    srv_ptr = FindBatchServer(srv_name,true);
    return(srv_ptr);
}

//------------------------------------------------------------------------------

const CBatchServerPtr CBatchServers::FindBatchServerByNode(CSmallString& name)
{
    // node             default server name
    // node@S           short server name
    // node@server      explicit server name

    CBatchServerPtr srv_ptr;
    if( name == NULL ) return(srv_ptr);

    CSmallString srv_name;
    CSmallString new_name = name;

    int at = name.FindSubString("@");
    if( at >= 0 ){
        if( at + 1 < (int)name.GetLength() ){
            srv_name = name.GetSubString(at+1,name.GetLength()-(at+1));
        }
        if( at > 0 ){
            new_name = name.GetSubString(0,at);
        } else {
            new_name = NULL;
        }
    }

    // update node name
    name = new_name;

    // not provided - use default server
    if( srv_name == NULL ){
        srv_name = GetDefaultSrvName();
    }

    srv_ptr = FindBatchServer(srv_name,true);
    return(srv_ptr);
}

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

