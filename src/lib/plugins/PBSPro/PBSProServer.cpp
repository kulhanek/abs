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

#include "PBSProServer.hpp"
#include <ErrorSystem.hpp>
#include <iostream>
#include <pbs_ifl.h>
#include <stdlib.h>
#include <string.h>
#include "PBSProAttr.hpp"
#include <boost/algorithm/string/split.hpp>
#include <boost/algorithm/string/classification.hpp>
#include <Shell.hpp>
#include <FileSystem.hpp>
#include <PluginObject.hpp>
#include "PBSProModule.hpp"
#include <CategoryUUID.hpp>
#include <ABSConfig.hpp>
#include <AMSGlobalConfig.hpp>
#include <PluginDatabase.hpp>
#include <NodeList.hpp>
#include <JobList.hpp>
#include <ResourceList.hpp>
#include "PBSProQueue.hpp"
#include "PBSProNode.hpp"
#include "PBSProJob.hpp"
#include <XMLElement.hpp>

using namespace std;
using namespace boost;

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

CComObject* PBSProServerCB(void* p_data);

CExtUUID        PBSProServerID(
                    "{PBSPRO_SERVER:e6425fbb-f6aa-48ff-84b4-1d37a9aa01eb}",
                    "PBSPro Batch Server");

CPluginObject   PBSProServerObject(&PBSProPlugin,
                    PBSProServerID,BATCH_SERVER_CAT,
                    PBSProServerCB);

// -----------------------------------------------------------------------------

CComObject* PBSProServerCB(void* p_data)
{
    CComObject* p_object = new CPBSProServer();
    return(p_object);
}

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

CPBSProServer::CPBSProServer(void)
    : CBatchServer(&PBSProServerObject)
{
    ServerID = 0;

    pbspro_connect = NULL;
    pbspro_disconnect = NULL;
    pbspro_statserver = NULL;
    pbspro_statque = NULL;
    pbspro_stathost = NULL;
    pbspro_statvnode = NULL;
    pbspro_statjob = NULL;
    pbspro_selstat = NULL;
    pbspro_statfree = NULL;
    pbspro_submit = NULL;
    pbspro_deljob = NULL;
    pbspro_geterrmsg = NULL;
    pbspro_strerror = NULL;
    pbspro_locjob = NULL;
    pbspro_errno = NULL;
}

//------------------------------------------------------------------------------

CPBSProServer::~CPBSProServer(void)
{
    DisconnectFromServer();
}

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

bool CPBSProServer::Init(const CSmallString& server_name,const CSmallString& short_name)
{
    CSmallString libs_tok;

    if( ABSConfig.GetSystemConfigItem("INF_PBSPRO_LIBS",libs_tok) == false ){
        ES_ERROR("unable to get paths to PBSPro client library");
        return(false);
    }

    StartTimer();

    // find suitable library
    vector<string> libs;
    string         slibs_tok(libs_tok);
    split(libs,slibs_tok,is_any_of(":"),boost::token_compress_on);
    vector<string>::iterator it = libs.begin();
    vector<string>::iterator ie = libs.end();

    while( it != ie ){
        CSmallString lib(*it);
        if( CFileSystem::IsFile(lib) == true ){
            PBSProLibName = lib;
            break;
        }
        it++;
    }

    // record found path to libpbs.so for debugging
    CSmallString w;
    w << "libpbs.so: " << PBSProLibName;
    ES_WARNING(w);

    if( InitSymbols() == false ){
        ES_ERROR("unable to init symbols");
        EndTimer();
        return(false);
    }

    ServerName = server_name;
    ShortName = short_name;

    if( ConnectToServer() == false ){
        ES_TRACE_ERROR("unable to connect to server");
        EndTimer();
        return(false);
    }

    EndTimer();
    return(true);
}

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

bool CPBSProServer::InitSymbols(void)
{
    if( PBSProLib.Open(PBSProLibName) == false ){
        ES_ERROR("unable to load batch system library");
        return(false);
    }

    // load symbols
    bool status = true;
    pbspro_connect     = (PBS_CONNECT)PBSProLib.GetProcAddress("pbs_connect");
    if( pbspro_connect == NULL ){
        ES_ERROR("unable to bind to pbs_connect");
        status = false;
    }
    pbspro_disconnect  = (PBS_DISCONNECT)PBSProLib.GetProcAddress("pbs_disconnect");
    if( pbspro_disconnect == NULL ){
        ES_ERROR("unable to bind to pbs_disconnect");
        status = false;
    }
    pbspro_statserver  = (PBS_STATSERVER)PBSProLib.GetProcAddress("pbs_statserver");
    if( pbspro_statserver == NULL ){
        ES_ERROR("unable to bind to pbs_statserver");
        status = false;
    }
    pbspro_statque  = (PBS_STATQUE)PBSProLib.GetProcAddress("pbs_statque");
    if( pbspro_statque == NULL ){
        ES_ERROR("unable to bind to pbs_statque");
        status = false;
    }
    pbspro_stathost  = (PBS_STATHOST)PBSProLib.GetProcAddress("pbs_stathost");
    if( pbspro_stathost == NULL ){
        ES_ERROR("unable to bind to pbs_stathost");
        status = false;
    }
    pbspro_statvnode  = (PBS_STATVNODE)PBSProLib.GetProcAddress("pbs_statvnode");
    if( pbspro_statvnode == NULL ){
        ES_ERROR("unable to bind to pbs_statvnode");
        status = false;
    }
    pbspro_statjob  = (PBS_STATJOB)PBSProLib.GetProcAddress("pbs_statjob");
    if( pbspro_statjob == NULL ){
        ES_ERROR("unable to bind to pbs_statjob");
        status = false;
    }
    pbspro_selstat  = (PBS_SELSTATJOB)PBSProLib.GetProcAddress("pbs_selstat");
    if( pbspro_selstat == NULL ){
        ES_ERROR("unable to bind to pbs_selstat");
        status = false;
    }
    pbspro_statfree  = (PBS_STATFREE)PBSProLib.GetProcAddress("pbs_statfree");
    if( pbspro_statfree == NULL ){
        ES_ERROR("unable to bind to pbs_statfree");
        status = false;
    }
    pbspro_submit  = (PBS_SUBMIT)PBSProLib.GetProcAddress("pbs_submit");
    if( pbspro_submit == NULL ){
        ES_ERROR("unable to bind to pbs_submit");
        status = false;
    }
    pbspro_deljob  = (PBS_DELJOB)PBSProLib.GetProcAddress("pbs_deljob");
    if( pbspro_deljob == NULL ){
        ES_ERROR("unable to bind to pbs_deljob");
        status = false;
    }
    pbspro_geterrmsg  = (PBS_GETERRMSG)PBSProLib.GetProcAddress("pbs_geterrmsg");
    if( pbspro_geterrmsg == NULL ){
        ES_ERROR("unable to bind to pbs_geterrmsg");
        status = false;
    }
    pbspro_locjob  = (PBS_LOCJOB)PBSProLib.GetProcAddress("pbs_locjob");
    if( pbspro_locjob == NULL ){
        ES_ERROR("unable to bind to pbspro_locjob");
        status = false;
    }
    pbspro_errno  = (PBS_ERRNO)PBSProLib.GetProcAddress("__pbs_errno_location");
    if( pbspro_errno == NULL ){
        ES_ERROR("unable to bind to __pbs_errno_location");
        status = false;
    }
    return(status);
}

//------------------------------------------------------------------------------

bool CPBSProServer::ConnectToServer(void)
{
    ServerID = pbspro_connect(ServerName);
    if( ServerID <= 0 ){    
        CSmallString error;
        error << "unable to connect to server '" << ServerName << "'";
        ES_TRACE_ERROR(error);
        return(false);
    }
    return(true);
}

//------------------------------------------------------------------------------

bool CPBSProServer::DisconnectFromServer(void)
{
    if( ServerID <= 0 ) return(true);

    int error = pbspro_disconnect(ServerID);
    ServerID = 0;
    if( error != 0 ){
        ES_ERROR("unable to disconnect from server");
        return(false);
    }

    return(true);
}

//------------------------------------------------------------------------------

void CPBSProServer::PrintBatchStatus(std::ostream& sout,struct batch_status* p_bs)
{
    int i = 0;
    while( p_bs != NULL ){
        if( i > 1 ) sout << endl;
        sout << p_bs->name << endl;
        PrintAttributes(sout,p_bs->attribs);
        p_bs = p_bs->next;
        i++;
    }
}

//------------------------------------------------------------------------------

void CPBSProServer::PrintAttributes(std::ostream& sout,struct attrl* p_as)
{
    while( p_as != NULL ){
        sout << "    " << p_as->name;
        if( (p_as->resource != NULL) && (strlen(p_as->resource) > 0) ){
        sout << "." << p_as->resource;
        }
        sout << " = " << p_as->value << endl;
        p_as = p_as->next;
    }
}

//------------------------------------------------------------------------------

void CPBSProServer::PrintAttributes(std::ostream& sout,struct attropl* p_as)
{
    while( p_as != NULL ){
        sout << "    " << p_as->name;
        if( (p_as->resource != NULL) && (strlen(p_as->resource) > 0) ){
        sout << "." << p_as->resource;
        }
        sout << " = " << p_as->value << endl;
        p_as = p_as->next;
    }
}

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

const CSmallString CPBSProServer::LocateJob(const CSmallString& jobid)
{
    CSmallString full_job_id;
    full_job_id = GetFullJobID(jobid);

    char* p_srv_name = pbspro_locjob(ServerID,(char*)full_job_id.GetBuffer(),NULL);
    if( p_srv_name == NULL ) return("");
    CSmallString srv_name(p_srv_name);
    free(p_srv_name);
    return(srv_name);
}

//------------------------------------------------------------------------------

bool CPBSProServer::GetQueues(CQueueList& queues)
{
    StartTimer();

    struct batch_status* p_queue_attrs = pbspro_statque(ServerID,NULL,NULL,NULL);

    bool result = true;
    while( p_queue_attrs != NULL ){
        CPBSProQueue* p_queue = new CPBSProQueue();
        if( p_queue->Init(ShortName, p_queue_attrs) == false ){
            ES_ERROR("unable to init queue");
            result = false;
            delete p_queue;
        } else {
            CQueuePtr queue(p_queue);
            queues.push_back(queue);
        }
        p_queue_attrs = p_queue_attrs->next;
    }

    if( p_queue_attrs ) pbspro_statfree(p_queue_attrs);

    EndTimer();

    return(result);
}

//------------------------------------------------------------------------------

bool CPBSProServer::GetNodes(CNodeList& nodes)
{
    StartTimer();

    struct batch_status* p_node_attrs = pbspro_stathost(ServerID,NULL,NULL,NULL);

    bool result = true;
    while( p_node_attrs != NULL ){
        CPBSProNode* p_node = new CPBSProNode;
        if( p_node->Init(ShortName,p_node_attrs) == false ){
            ES_ERROR("unable to init node");
            result = false;
            delete p_node;
        } else {
            CNodePtr node(p_node);
            nodes.push_back(node);
        }
        p_node_attrs = p_node_attrs->next;
    }

    if( p_node_attrs ) pbspro_statfree(p_node_attrs);

    EndTimer();

    return(result);
}

//------------------------------------------------------------------------------

bool CPBSProServer::GetAllJobs(CJobList& jobs,bool include_history)
{
    StartTimer();

    CSmallString extend;
    if( include_history ){
        extend << "x";
    }

    struct batch_status* p_jobs;
    p_jobs = pbspro_statjob(ServerID,NULL,NULL,extend.GetBuffer());

    bool result = true;
    while( p_jobs != NULL ){
        CPBSProJob* p_job = new CPBSProJob;
        if( p_job->Init(ServerName,ShortName,p_jobs) == false ){
            ES_ERROR("unable to init job");
            result = false;
            delete p_job;
        } else {
            CJobPtr job(p_job);
            jobs.push_back(job);
        }
        p_jobs = p_jobs->next;
    }

    if( p_jobs ) pbspro_statfree(p_jobs);

    EndTimer();

    return(result);
}

//------------------------------------------------------------------------------

bool CPBSProServer::GetQueueJobs(CJobList& jobs,const CSmallString& queue_name,bool include_history)
{
    StartTimer();

    CSmallString extend;
    if( include_history ){
        extend << "x";
    }

    struct batch_status* p_jobs;
    p_jobs = pbspro_statjob(ServerID,(char*)queue_name.GetBuffer(),NULL,extend.GetBuffer());

    bool result = true;
    while( p_jobs != NULL ){
        CPBSProJob* p_job = new CPBSProJob;
        if( p_job->Init(ServerName,ShortName,p_jobs) == false ){
            ES_ERROR("unable to init job");
            result = false;
            delete p_job;
        } else {
            CJobPtr job(p_job);
            jobs.push_back(job);
        }
        p_jobs = p_jobs->next;
    }

    if( p_jobs ) pbspro_statfree(p_jobs);

    EndTimer();

    return(result);
}

//------------------------------------------------------------------------------

bool CPBSProServer::GetUserJobs(CJobList& jobs,const CSmallString& user,bool include_history)
{
    StartTimer();

    CSmallString extend;
    if( include_history ){
        extend << "x";
    }

    struct attropl* p_first = NULL;
    set_attribute(p_first,ATTR_u,NULL,user,EQ);

    struct batch_status* p_jobs;
    p_jobs = pbspro_selstat(ServerID,p_first,NULL,extend.GetBuffer());

    bool result = true;
    while( p_jobs != NULL ){
        CPBSProJob* p_job = new CPBSProJob;
        if( p_job->Init(ServerName,ShortName,p_jobs) == false ){
            ES_ERROR("unable to init job");
            result = false;
            delete p_job;
        } else {
            CJobPtr job(p_job);
            jobs.push_back(job);
        }
        p_jobs = p_jobs->next;
    }

    if( p_jobs ) pbspro_statfree(p_jobs);

    EndTimer();

    return(result);
}

//------------------------------------------------------------------------------

bool CPBSProServer::GetJob(CJobList& jobs,const CSmallString& jobid)
{
    CJobPtr job = GetJob(jobid);
    if( job != NULL ){
        jobs.push_back(job);
        return(true);
    }
    return(false);
}

//------------------------------------------------------------------------------

const CJobPtr CPBSProServer::GetJob(const CSmallString& jobid)
{
    StartTimer();

    // always use extend attribute
    CSmallString extend;
    extend << "x";

    CSmallString full_job_id;
    full_job_id = GetFullJobID(jobid);

    struct batch_status* p_jobs;
    p_jobs = pbspro_statjob(ServerID,(char*)full_job_id.GetBuffer(),NULL,extend.GetBuffer());

    if( p_jobs == NULL ){
        char* p_error = pbspro_geterrmsg(ServerID);
        if( p_jobs ) ES_TRACE_ERROR(p_error);
    }

    CJobPtr result;

    if( p_jobs != NULL ){
        CPBSProJob* p_job = new CPBSProJob;
        if( p_job->Init(ServerName,ShortName,p_jobs) == false ){
            delete p_job;
            ES_ERROR("unable to init job");
        } else {
            result = CJobPtr(p_job);
        }
    }

    if( p_jobs ) pbspro_statfree(p_jobs);

    EndTimer();

    return(result);
}

//------------------------------------------------------------------------------

bool CPBSProServer::PrintQueues(std::ostream& sout)
{
    StartTimer();

    struct batch_status* p_queues = pbspro_statque(ServerID,NULL,NULL,NULL);
    if( p_queues != NULL ) {
        PrintBatchStatus(sout,p_queues);
        pbspro_statfree(p_queues);
    }

    EndTimer();

    return(p_queues != NULL);
}

//------------------------------------------------------------------------------

bool CPBSProServer::PrintNodes(std::ostream& sout)
{
    StartTimer();

    struct batch_status* p_nodes = pbspro_stathost(ServerID,NULL,NULL,NULL);
    if( p_nodes != NULL ) {
        PrintBatchStatus(sout,p_nodes);
        pbspro_statfree(p_nodes);
    }

    EndTimer();

    return(p_nodes != NULL);
}

//------------------------------------------------------------------------------

bool CPBSProServer::PrintNode(std::ostream& sout,const CSmallString& name)
{
    StartTimer();

    struct batch_status* p_nodes = pbspro_stathost(ServerID,(char*)name.GetBuffer(),NULL,NULL);
    if( p_nodes != NULL ) {
        PrintBatchStatus(sout,p_nodes);
        pbspro_statfree(p_nodes);
    } else {
        // try vhost
        p_nodes = pbspro_statvnode(ServerID,(char*)name.GetBuffer(),NULL,NULL);
        if( p_nodes != NULL ) {
            PrintBatchStatus(sout,p_nodes);
            pbspro_statfree(p_nodes);
        }
    }

    EndTimer();

    return(p_nodes != NULL);
}

//------------------------------------------------------------------------------

bool CPBSProServer::PrintJobs(std::ostream& sout)
{
    StartTimer();

    struct batch_status* p_jobs = pbspro_statjob(ServerID,NULL,NULL,NULL);
    if( p_jobs != NULL ) {
        PrintBatchStatus(sout,p_jobs);
        pbspro_statfree(p_jobs);
    }

    EndTimer();

    return(p_jobs != NULL);
}

//------------------------------------------------------------------------------

bool CPBSProServer::PrintJob(std::ostream& sout,const CSmallString& jobid)
{
    StartTimer();

    CSmallString full_job_id;
    full_job_id = GetFullJobID(jobid);

    struct batch_status* p_jobs = pbspro_statjob(ServerID,(char*)full_job_id.GetBuffer(),NULL,NULL);
    if( p_jobs != NULL ) {
        PrintBatchStatus(sout,p_jobs);
        pbspro_statfree(p_jobs);
    }

    EndTimer();

    return(p_jobs != NULL);
}

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

bool CPBSProServer::InitBatchResources(CResourceList* p_rl)
{
    CResourceValuePtr rv_ptr;
    rv_ptr = p_rl->FindResource("select");
    if( rv_ptr ){
        // FIXME
        ES_ERROR("explicit select is not supported yet");
        return(false);
    }

    stringstream str;

// ------------
    int nnodes = p_rl->GetNumOfNodes();
    int ncpus = p_rl->GetNumOfCPUs();
    int ngpus = p_rl->GetNumOfGPUs();
    str << nnodes << ":ncpus=" << ncpus / nnodes;
    if( ngpus > 0 ){
        str << ":ngpus=" << ngpus / nnodes;
    }
    str << ":mpiprocs=" << ncpus / nnodes;
// ------------
    rv_ptr = p_rl->FindResource("mem");
    if( rv_ptr ){
        // memory in kb
        long long mem = rv_ptr->GetSize();
        mem = mem / nnodes;
        str << ":mem=" << CResourceValue::GetSizeString(mem);
    }
// ------------
    rv_ptr = p_rl->FindResource("workdir");
    CResourceValuePtr rvs_ptr = p_rl->FindResource("worksize");
    if( rv_ptr && rvs_ptr ){
        CXMLElement* p_ele = ABSConfig.GetWorkDirConfig();
        if( p_ele != NULL ){
            // find workdir type
            CXMLElement* p_wele = p_ele->GetFirstChildElement("workdir");
            while( p_wele != NULL ){
                CSmallString name;
                p_wele->GetAttribute("name",name);
                if( name == rv_ptr->GetValue() ) break;
                p_wele = p_wele->GetNextSiblingElement("workdir");
            }
            if( p_wele != NULL ){
                // generate dependent resources
                CXMLElement* p_rele = p_wele->GetFirstChildElement("select");
                if( p_rele != NULL ){
                    CSmallString name;
                    p_rele->GetAttribute("name",name);
                    long long scratch_size = rvs_ptr->GetSize();
                    scratch_size = scratch_size / nnodes;
                    str << ":" << name << "=" << CResourceValue::GetSizeString(scratch_size);
                }
            }
        }
    }

// ------------
    rv_ptr = p_rl->FindResource("props");
    if( rv_ptr ){
        // syntax [^]property[=value][(#|:)other_property...]

        vector<string> slist;
        string         svalue(rv_ptr->GetValue());
        split(slist,svalue,is_any_of("#:"),boost::token_compress_on);

        vector<string>::iterator it = slist.begin();
        vector<string>::iterator ie = slist.end();

        while( it != ie ){
            string item = *it;
            it++;
            if( item.size() == 0 ) continue;
            string name = item;
            if ( item.find('=') != string::npos ){
                // item already contains property name and its value
                str << ":" << item;
            } else {
                // old fashion property specification
                string value = "true";
                if( item[0] == '^' ){
                    name = string(item.begin()+1,item.end());
                    value = "false";
                }
                str << ":" << name << "=" << value;
            }
        }
    }

    p_rl->AddRawResource("select",CSmallString(str.str().c_str()));
    return(true);
}

//------------------------------------------------------------------------------

bool CPBSProServer::SubmitJob(CJob& job,bool verbose)
{
    StartTimer();

    CFileName script    = job.GetMainScriptName();
    CFileName infout    = job.GetInfoutName();
    CFileName queue     = job.GetQueue();
    CFileName jobtitle  = job.GetJobTitle();
    CSmallString depjid = job.GetItem("basic/external","INF_EXTERNAL_START_AFTER");

    CSmallString item;
    CSmallString variables;
    CSmallString mailoptions;

    // setup variables
    variables <<  "INF_JOB_NAME=" << job.GetJobName();
    variables << ",INF_JOB_NAME_SUFFIX=" << job.GetJobNameSuffix();
    variables << ",INF_INPUT_MACHINE=" << job.GetInputMachine();
    variables << ",INF_INPUT_DIR=" << job.GetInputDir();
    variables << ",INF_STORAGE_MACHINE=" << job.GetStorageMachine();
    variables << ",INF_STORAGE_DIR=" << job.GetStorageDir();
    variables << ",INF_JOB_KEY=" << job.GetJobKey();
    variables << ",ABS_ROOT=" << CShell::GetSystemVariable("ABS_ROOT");

    if( ABSConfig.GetSystemConfigItem("INF_BOOT_SCRIPT",item) ){
        variables << ",INF_BOOT_SCRIPT=" << item;
    }

    variables << ",INF_SITE_ID=" << AMSGlobalConfig.GetActiveSiteID();
    variables << ",INF_ABS_VERSION=" << ABSConfig.GetABSModuleVersion();

    if( job.GetExternalVariables() != NULL ){
        string sl = string(job.GetExternalVariables());
        vector<string>  svariables;
        split(svariables,sl,is_any_of(","));

        vector<string>::iterator it = svariables.begin();
        vector<string>::iterator ie = svariables.end();

        while( it != ie ){
            CSmallString var = *it;
            CSmallString val = CShell::GetSystemVariable(var);
            variables << "," << var << "=" << val;
            it++;
        }
    }
    variables << job.ResourceList.GetVariables();
    variables << ",AMS_SITE_SUPPORT=" << CShell::GetSystemVariable("AMS_SITE_SUPPORT");
    // required for accounting?
    variables << ",PBS_O_LOGNAME=" << User.GetName();

    struct attropl* p_first = NULL;
    struct attropl* p_prev = NULL;

    set_attribute(p_first,ATTR_N,NULL,jobtitle);
    p_prev = p_first;

    set_attribute(p_prev,ATTR_o,NULL,infout);
    set_attribute(p_prev,ATTR_e,NULL,infout);
    set_attribute(p_prev,ATTR_j,NULL,"oe");

    // rerunable
    if( ABSConfig.GetUserConfigItem("INF_RERUNABLE_JOBS",item) ){
        if( item == "YES" ){
            set_attribute(p_prev,ATTR_r,NULL,"y");
        } else {
            set_attribute(p_prev,ATTR_r,NULL,"n");
        }
    } else {
        set_attribute(p_prev,ATTR_r,NULL,"n");
    }

    // mail options
    mailoptions = NULL;
    if( ABSConfig.GetUserConfigItem("INF_STOP_EMAIL",item) ){
        if( item == "YES" ){
            mailoptions << "e";
        }
    }
    if( ABSConfig.GetUserConfigItem("INF_ABORT_EMAIL",item) ){
        if( item == "YES" ){
            mailoptions << "a";
        }
    }
    if( ABSConfig.GetUserConfigItem("INF_START_EMAIL",item) ){
        if( item == "YES" ){
            mailoptions << "b";
        }
    }
    if( mailoptions == NULL ) mailoptions = "n";

    set_attribute(p_prev,ATTR_m,NULL,mailoptions);
    set_attribute(p_prev,ATTR_v,NULL,variables);

    CreateJobAttributes(p_prev,&(job.ResourceList));

    if( depjid != NULL ){
        depjid = "afterany:" + depjid;
        set_attribute(p_prev,ATTR_depend,NULL,depjid);
    }

//  DEBUG
    if( verbose ){
        PrintAttributes(cout,p_first);
    }

    // submit jobs
    char* p_jobid = pbspro_submit(ServerID,p_first,script,queue,NULL);

    if( p_jobid == NULL ){
        char* p_error = pbspro_geterrmsg(ServerID);
        job.WriteErrorSection(p_error);
        CSmallString error;
        error << "unable to submit job, error: "  << p_error;
        ES_TRACE_ERROR(error);
        return(false);
    }

    // write submit flag
    job.WriteSubmitSection(p_jobid);

    free(p_jobid);

    EndTimer();

    return(true);
}

//------------------------------------------------------------------------------

void CPBSProServer::CreateJobAttributes(struct attropl* &p_prev,CResourceList* p_rl)
{
    std::list<CResourceValuePtr>::iterator     it = p_rl->begin();
    std::list<CResourceValuePtr>::iterator     ie = p_rl->end();

    while( it != ie ){
        CResourceValuePtr p_rv = *it;
        CSmallString name,resource,value;
        p_rv->GetAttribute(name,resource,value);
        if( name != NULL ){
            if( resource == NULL ){
                // KEEP as it is here, resource argument to set_attribute MUST be NULL
                set_attribute(p_prev,name,NULL,value);
            } else {
                set_attribute(p_prev,name,resource,value);
            }
        }
        it++;
    }
}

//------------------------------------------------------------------------------

bool CPBSProServer::GetJobStatus(CJob& job)
{
    StartTimer();

    CSmallString jobid = job.GetJobID();

    batch_status* p_status = pbspro_statjob(ServerID,jobid.GetBuffer(),NULL,NULL);

    job.BatchJobComment = NULL;
    job.BatchJobStatus = EJS_ERROR;

    if( p_status == NULL ){
        return(true);
    }

    attrl* p_item = p_status->attribs;

    CPBSProJob::DecodeBatchJobComment(p_item,job.BatchJobComment);

    while( p_item != NULL ){
        // job status
        if( strcmp(p_item->name,"job_state") == 0 ){
            CSmallString status;
            status = p_item->value;
            if( status == "Q" ){
                job.BatchJobStatus = EJS_SUBMITTED;
            } else if( status == "H" ) {
                job.BatchJobStatus = EJS_SUBMITTED;
            } else if( status == "R" ) {
                job.BatchJobStatus = EJS_RUNNING;
            } else if( status == "C" ) {
                job.BatchJobStatus = EJS_FINISHED;
            } else if( status == "E" ) {
                job.BatchJobStatus = EJS_ERROR;
            } else if( status == "M" ) {
                job.BatchJobStatus = EJS_MOVED;
            }
        }

        p_item = p_item->next;
    }

    pbspro_statfree(p_status);

    EndTimer();

    return(true);
}

//------------------------------------------------------------------------------

bool CPBSProServer::KillJob(CJob& job)
{
    StartTimer();

    CSmallString jobid = job.GetJobID();
    int retval = pbspro_deljob(ServerID,jobid.GetBuffer(),NULL);

    EndTimer();

    return( retval == 0 );
}

//------------------------------------------------------------------------------

bool CPBSProServer::KillJobByID(const CSmallString& jobid)
{
    StartTimer();

    CSmallString lid(jobid);
    int retval = pbspro_deljob(ServerID,lid.GetBuffer(),NULL);

    EndTimer();

    return( retval == 0 );
}

//------------------------------------------------------------------------------

const CSmallString CPBSProServer::GetLastErrorMsg(void)
{
    StartTimer();
    CSmallString errmsg(pbspro_geterrmsg(ServerID));
    EndTimer();

    return(errmsg);
}

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

