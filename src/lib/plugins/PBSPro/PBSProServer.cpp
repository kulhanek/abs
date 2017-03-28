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
#include "PBSProQueue.hpp"
#include "PBSProNode.hpp"
#include "PBSProJob.hpp"

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

    pbs_connect = NULL;
    pbs_disconnect = NULL;
    pbs_statserver = NULL;
    pbs_statque = NULL;
    pbs_statnode = NULL;
    pbs_statjob = NULL;
    pbs_selstat = NULL;
    pbs_statfree = NULL;
    pbs_submit = NULL;
    pbs_deljob = NULL;
    pbs_geterrmsg = NULL;
    pbs_strerror = NULL;
    pbs_errno = NULL;
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

    if( PluginDatabase.FindObjectConfigValue(PBSProServerID,"libname",libs_tok) == false ){
        ES_ERROR("unable to get paths to PBSPro client library");
        return(false);
    }

    // find suitable library
    vector<string> libs;
    string         slibs_tok(libs_tok);
    split(libs,slibs_tok,is_any_of(":"),boost::token_compress_on);
    vector<string>::iterator it = libs.begin();
    vector<string>::iterator ie = libs.end();

    while( it != ie ){
        CSmallString lib(*it);
        if( CFileSystem::IsFile(lib) == true ){
            TorqueLibName = lib;
            break;
        }
        it++;
    }

    if( InitSymbols() == false ){
        ES_ERROR("unable to init symbols");
        return(false);
    }

    ServerName = server_name;
    ShortName = short_name;

    if( ConnectToServer() == false ){
        ES_ERROR("unable to connect to server");
        return(false);
    }

    return(true);
}

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

bool CPBSProServer::InitSymbols(void)
{
    if( TorqueLib.Open(TorqueLibName) == false ){
        ES_ERROR("unable to load torque library");
        return(false);
    }

    // load symbols
    bool status = true;
    pbs_connect     = (PBS_CONNECT)TorqueLib.GetProcAddress("pbs_connect");
    if( pbs_connect == NULL ){
        ES_ERROR("unable to bind to pbs_connect");
        status = false;
    }
    pbs_disconnect  = (PBS_DISCONNECT)TorqueLib.GetProcAddress("pbs_disconnect");
    if( pbs_disconnect == NULL ){
        ES_ERROR("unable to bind to pbs_disconnect");
        status = false;
    }
    pbs_statserver  = (PBS_STATSERVER)TorqueLib.GetProcAddress("pbs_statserver");
    if( pbs_statserver == NULL ){
        ES_ERROR("unable to bind to pbs_statserver");
        status = false;
    }
    pbs_statque  = (PBS_STATQUE)TorqueLib.GetProcAddress("pbs_statque");
    if( pbs_statque == NULL ){
        ES_ERROR("unable to bind to pbs_statque");
        status = false;
    }
    pbs_statnode  = (PBS_STATNODE)TorqueLib.GetProcAddress("pbs_statnode");
    if( pbs_statnode == NULL ){
        ES_ERROR("unable to bind to pbs_statnode");
        status = false;
    }
    pbs_statjob  = (PBS_STATJOB)TorqueLib.GetProcAddress("pbs_statjob");
    if( pbs_statjob == NULL ){
        ES_ERROR("unable to bind to pbs_statjob");
        status = false;
    }
    pbs_selstat  = (PBS_SELSTATJOB)TorqueLib.GetProcAddress("pbs_selstat");
    if( pbs_selstat == NULL ){
        ES_ERROR("unable to bind to pbs_selstat");
        status = false;
    }
    pbs_statfree  = (PBS_STATFREE)TorqueLib.GetProcAddress("pbs_statfree");
    if( pbs_statfree == NULL ){
        ES_ERROR("unable to bind to pbs_statfree");
        status = false;
    }
    pbs_submit  = (PBS_SUBMIT)TorqueLib.GetProcAddress("pbs_submit");
    if( pbs_submit == NULL ){
        ES_ERROR("unable to bind to pbs_submit");
        status = false;
    }
    pbs_deljob  = (PBS_DELJOB)TorqueLib.GetProcAddress("pbs_deljob");
    if( pbs_deljob == NULL ){
        ES_ERROR("unable to bind to pbs_deljob");
        status = false;
    }
    pbs_geterrmsg  = (PBS_GETERRMSG)TorqueLib.GetProcAddress("pbs_geterrmsg");
    if( pbs_geterrmsg == NULL ){
        ES_ERROR("unable to bind to pbs_geterrmsg");
        status = false;
    }
//    pbs_errno  = (PBS_ERRNO)TorqueLib.GetObjAddress("pbs_errno");
//    if( pbs_errno == NULL ){
//        ES_ERROR("unable to bind to pbs_errno");
//        status = false;
//    }
    return(status);
}

//------------------------------------------------------------------------------

bool CPBSProServer::ConnectToServer(void)
{
    ServerID = pbs_connect(ServerName);
    if( ServerID <= 0 ){    
        CSmallString error;
        error << "unable to connect to server";
        ES_ERROR(error);
        return(false);
    }
    return(true);
}

//------------------------------------------------------------------------------

bool CPBSProServer::DisconnectFromServer(void)
{
    if( ServerID <= 0 ) return(true);

    int error = pbs_disconnect(ServerID);
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
        if( p_as->resource ){
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
        if( p_as->resource ){
        sout << "." << p_as->resource;
        }
        sout << " = " << p_as->value << endl;
        p_as = p_as->next;
    }
}

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

bool CPBSProServer::GetQueues(CQueueList& queues)
{
    struct batch_status* p_queue_attrs = pbs_statque(ServerID,NULL,NULL,NULL);

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

    if( p_queue_attrs ) pbs_statfree(p_queue_attrs);

    return(result);
}

//------------------------------------------------------------------------------

bool CPBSProServer::GetNodes(CNodeList& nodes)
{
    struct batch_status* p_node_attrs = pbs_statnode(ServerID,NULL,NULL,NULL);

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

    if( p_node_attrs ) pbs_statfree(p_node_attrs);

    return(result);
}

//------------------------------------------------------------------------------

bool CPBSProServer::GetAllJobs(CJobList& jobs,bool finished)
{
// it does not work in PBSPro
//    struct attrl* p_first = NULL;
//    struct attrl* p_prev = NULL;

//    set_attribute(p_first,ATTR_JOB_STATE,NULL,NULL);
//    p_prev = p_first;
//    set_attribute(p_prev,ATTR_JOB_TITLE,NULL,NULL);
//    set_attribute(p_prev,ATTR_JOB_OWNER,NULL,NULL);
//    set_attribute(p_prev,ATTR_JOB_QUEUE,NULL,NULL);
//    set_attribute(p_prev,ATTR_JOB_COMMENT,NULL,NULL);
//    set_attribute(p_prev,ATTR_JOB_OUTPUT_PATH,NULL,NULL);
//    set_attribute(p_prev,ATTR_JOB_EXEC_HOST,NULL,NULL);
//    set_attribute(p_prev,ATTR_JOB_VARIABLE_LIST,NULL,NULL);
//    set_attribute(p_prev,ATTR_JOB_RESOURCE_LIST,NULL,NULL);
//    set_attribute(p_prev,ATTR_JOB_CREATE_TIME,NULL,NULL);
//    set_attribute(p_prev,ATTR_JOB_SUBMIT_TIME,NULL,NULL);
//    set_attribute(p_prev,ATTR_JOB_START_TIME,NULL,NULL);
//    set_attribute(p_prev,ATTR_JOB_FINISH_TIME,NULL,NULL);
//    set_attribute(p_prev,ATTR_JOB_HOLD_TIME,NULL,NULL);

    CSmallString extend;
    if( finished ){
        extend << "x";
    }

    struct batch_status* p_jobs;
    p_jobs = pbs_statjob(ServerID,NULL,NULL,extend);

    bool result = true;
    while( p_jobs != NULL ){
        CPBSProJob* p_job = new CPBSProJob;
        if( p_job->Init(ShortName,p_jobs) == false ){
            ES_ERROR("unable to init job");
            result = false;
            delete p_job;
        } else {
            CJobPtr job(p_job);
            jobs.push_back(job);
        }
        p_jobs = p_jobs->next;
    }

    if( p_jobs ) pbs_statfree(p_jobs);

    return(result);
}

//------------------------------------------------------------------------------

bool CPBSProServer::GetQueueJobs(CJobList& jobs,const CSmallString& queue_name,bool finished)
{
// it does not work in PBSPro
//    struct attrl* p_first = NULL;
//    struct attrl* p_prev = NULL;

//    set_attribute(p_first,ATTR_JOB_STATE,NULL,NULL);
//    p_prev = p_first;
//    set_attribute(p_prev,ATTR_JOB_TITLE,NULL,NULL);
//    set_attribute(p_prev,ATTR_JOB_OWNER,NULL,NULL);
//    set_attribute(p_prev,ATTR_JOB_QUEUE,NULL,NULL);
//    set_attribute(p_prev,ATTR_JOB_COMMENT,NULL,NULL);
//    set_attribute(p_prev,ATTR_JOB_OUTPUT_PATH,NULL,NULL);
//    set_attribute(p_prev,ATTR_JOB_EXEC_HOST,NULL,NULL);
//    set_attribute(p_prev,ATTR_JOB_VARIABLE_LIST,NULL,NULL);
//    set_attribute(p_prev,ATTR_JOB_RESOURCE_LIST,NULL,NULL);
//    set_attribute(p_prev,ATTR_JOB_CREATE_TIME,NULL,NULL);
//    set_attribute(p_prev,ATTR_JOB_SUBMIT_TIME,NULL,NULL);
//    set_attribute(p_prev,ATTR_JOB_START_TIME,NULL,NULL);
//    set_attribute(p_prev,ATTR_JOB_FINISH_TIME,NULL,NULL);
//    set_attribute(p_prev,ATTR_JOB_HOLD_TIME,NULL,NULL);

    struct batch_status* p_jobs;
    p_jobs = pbs_statjob(ServerID,(char*)queue_name.GetBuffer(),NULL,NULL);

    bool result = true;
    while( p_jobs != NULL ){
        CPBSProJob* p_job = new CPBSProJob;
        if( p_job->Init(ShortName,p_jobs) == false ){
            ES_ERROR("unable to init job");
            result = false;
            delete p_job;
        } else {
            CJobPtr job(p_job);
            jobs.push_back(job);
        }
        p_jobs = p_jobs->next;
    }

    if( p_jobs ) pbs_statfree(p_jobs);

    return(result);
}

//------------------------------------------------------------------------------

bool CPBSProServer::GetUserJobs(CJobList& jobs,const CSmallString& user,bool finished)
{
    struct attropl* p_first = NULL;
    set_attribute(p_first,ATTR_USER_LIST,NULL,user,EQ);

    struct batch_status* p_jobs;
    p_jobs = pbs_selstat(ServerID,p_first,NULL,NULL);

    bool result = true;
    while( p_jobs != NULL ){
        CPBSProJob* p_job = new CPBSProJob;
        if( p_job->Init(ShortName,p_jobs) == false ){
            ES_ERROR("unable to init job");
            result = false;
            delete p_job;
        } else {
            CJobPtr job(p_job);
            jobs.push_back(job);
        }
        p_jobs = p_jobs->next;
    }

    if( p_jobs ) pbs_statfree(p_jobs);

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
// it crashes in PBSPro
//    struct attrl* p_first = NULL;
//    struct attrl* p_prev = NULL;

//    set_attribute(p_first,ATTR_JOB_STATE,NULL,NULL);
//    p_prev = p_first;
//    set_attribute(p_prev,ATTR_JOB_TITLE,NULL,NULL);
//    set_attribute(p_prev,ATTR_JOB_OWNER,NULL,NULL);
//    set_attribute(p_prev,ATTR_JOB_QUEUE,NULL,NULL);
//    set_attribute(p_prev,ATTR_JOB_COMMENT,NULL,NULL);
//    set_attribute(p_prev,ATTR_JOB_OUTPUT_PATH,NULL,NULL);
//    set_attribute(p_prev,ATTR_JOB_EXEC_HOST,NULL,NULL);
//    set_attribute(p_prev,ATTR_JOB_VARIABLE_LIST,NULL,NULL);
//    set_attribute(p_prev,ATTR_JOB_RESOURCE_LIST,NULL,NULL);

//    switch(ABSConfig.GetTorqueMode()){
//        case ETM_TORQUE:
//        case ETM_TORQUE_METAVO:
//            set_attribute(p_prev,ATTR_JOB_CREATE_TIME,NULL,NULL);
//            set_attribute(p_prev,ATTR_JOB_SUBMIT_TIME,NULL,NULL);
//            set_attribute(p_prev,ATTR_JOB_START_TIME,NULL,NULL);
//            set_attribute(p_prev,ATTR_JOB_FINISH_TIME,NULL,NULL);
//            set_attribute(p_prev,ATTR_JOB_HOLD_TIME,NULL,NULL);
//        break;
//        case ETM_PBSPRO:
//            set_attribute(p_prev,ATTR_JOB_CREATE_TIME,NULL,NULL);
//            set_attribute(p_prev,ATTR_JOB_SUBMIT_TIME,NULL,NULL);
//            set_attribute(p_prev,ATTR_JOB_START_TIME_2,NULL,NULL);
//            set_attribute(p_prev,ATTR_JOB_FINISH_TIME,NULL,NULL);
//            set_attribute(p_prev,ATTR_JOB_HOLD_TIME,NULL,NULL);
//        break;
//    }

    CSmallString full_job_id;
    // FIXME
    // CSmallString server = ABSConfig.GetServerName();
    CSmallString server;
    full_job_id = jobid;
    if( full_job_id.FindSubString(server) == -1 ){
        full_job_id += "." + server;
    }

    struct batch_status* p_jobs;
    p_jobs = pbs_statjob(ServerID,(char*)full_job_id.GetBuffer(),NULL,NULL);

    if( p_jobs == NULL ){
        char* p_error = pbs_geterrmsg(ServerID);
        if( p_jobs ) ES_TRACE_ERROR(p_error);
    }

    CJobPtr result;

    if( p_jobs != NULL ){
        CPBSProJob* p_job = new CPBSProJob;
        if( p_job->Init(ShortName,p_jobs) == false ){
            delete p_job;
            ES_ERROR("unable to init job");
        } else {
            result = CJobPtr(p_job);
        }
    }

    if( p_jobs ) pbs_statfree(p_jobs);

    return(result);
}

//------------------------------------------------------------------------------

bool CPBSProServer::PrintQueues(std::ostream& sout)
{
    struct batch_status* p_queues = pbs_statque(ServerID,NULL,NULL,NULL);
    if( p_queues != NULL ) {
        PrintBatchStatus(sout,p_queues);
        pbs_statfree(p_queues);
    }
    return(true);
}

//------------------------------------------------------------------------------

bool CPBSProServer::PrintNodes(std::ostream& sout)
{
    struct batch_status* p_nodes = pbs_statnode(ServerID,NULL,NULL,NULL);
    if( p_nodes != NULL ) {
        PrintBatchStatus(sout,p_nodes);
        pbs_statfree(p_nodes);
    }
    return(true);
}

//------------------------------------------------------------------------------

bool CPBSProServer::PrintNode(std::ostream& sout,const CSmallString& name)
{
    struct batch_status* p_nodes = pbs_statnode(ServerID,(char*)name.GetBuffer(),NULL,NULL);
    if( p_nodes != NULL ) {
        PrintBatchStatus(sout,p_nodes);
        pbs_statfree(p_nodes);
    }
    return(true);
}

//------------------------------------------------------------------------------

bool CPBSProServer::PrintJobs(std::ostream& sout)
{
    struct batch_status* p_jobs = pbs_statjob(ServerID,NULL,NULL,NULL);
    if( p_jobs != NULL ) {
        PrintBatchStatus(sout,p_jobs);
        pbs_statfree(p_jobs);
    }
    return(true);
}

//------------------------------------------------------------------------------

bool CPBSProServer::PrintJob(std::ostream& sout,const CSmallString& name)
{
    CSmallString full_name;
    CSmallString server_name = ABSConfig.GetSystemConfigItem("INF_SERVER_NAME");
    full_name = name;
    if( name.FindSubString(server_name) == -1 ){
        full_name = full_name + "." + server_name;
    }

    struct batch_status* p_jobs = pbs_statjob(ServerID,(char*)full_name.GetBuffer(),NULL,NULL);
    if( p_jobs != NULL ) {
        PrintBatchStatus(sout,p_jobs);
        pbs_statfree(p_jobs);
    }
    return(true);
}

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

bool CPBSProServer::SubmitJob(CJob& job)
{
    CFileName script    = job.GetMainScriptName();
    CFileName infout    = job.GetInfoutName();
    CFileName queue     = job.GetQueue();
    CFileName jobtitle  = job.GetJobTitle();
    CSmallString sres   = job.GetItem("specific/resources","INF_RESOURCES");
    CSmallString depjid = job.GetItem("basic/external","INF_EXTERNAL_START_AFTER");

    CSmallString item;
    CSmallString variables;
    CSmallString mailoptions;

    // setup variables
    variables <<  "INF_JOB_NAME=" << job.GetJobName();
    variables << ",INF_JOB_NAME_SUFFIX=" << job.GetJobNameSuffix();
    variables << ",INF_JOB_PATH=" << job.GetJobPath();
    variables << ",INF_JOB_MACHINE=" << job.GetJobMachine();
    variables << ",INF_SURROGATE_MACHINE=" << job.GetSurrogateMachine();
    variables << ",INF_JOB_KEY=" << job.GetJobKey();
    variables << ",ABS_ROOT=" << CShell::GetSystemVariable("ABS_ROOT");

    if( ABSConfig.GetSystemConfigItem("INF_BOOT_SCRIPT",item) ){
        variables << ",INF_BOOT_SCRIPT=" << item;
    }

    variables << ",INF_SITE_ID=" << AMSGlobalConfig.GetActiveSiteID();
    variables << ",INF_ABS_VERSION=" << ABSConfig.GetABSModuleVersion();
    variables << ",INF_UGROUP=" << job.GetUserGroup();
    variables << ",INF_UMASK=" << job.GetUMask();

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

    // FIXME
//    switch( ABSConfig.GetTorqueMode() ){
//        case ETM_TORQUE:
//        case ETM_TORQUE_METAVO:{
//            // get umask in decimal representation
//            int umask  = job.GetUMaskNumber();
//            CSmallString sumask(umask);
//            set_attribute(p_prev,ATTR_umask,NULL,sumask);
//            CSmallString egroup = job.GetUserGroup();
//            set_attribute(p_prev,ATTR_group_list,NULL,egroup);
//        }
//        break;
//        case ETM_PBSPRO:{
            // set mask in octal form
            CSmallString sumask = job.GetUMask();
            set_attribute(p_prev,ATTR_umask,NULL,sumask);
//        }
//        break;
//    }

    if( depjid != NULL ){
        depjid = "afterany:" + depjid;
        set_attribute(p_prev,ATTR_depend,NULL,depjid);
    }

    CResourceList res;
    res.Parse(sres);

    CSmallString account   = res.GetResourceValue("account");
    if( account != NULL ){
        set_attribute(p_prev,ATTR_A,NULL,account);
    }

    res.Finalize();
    res.GetTorqueResources(p_prev);

//  DEBUG
//    PrintAttributes(cout,p_first);

    // submit jobs
    char* p_jobid = pbs_submit(ServerID,p_first,script,queue,NULL);

    if( p_jobid == NULL ){
        char* p_error = pbs_geterrmsg(ServerID);
        job.WriteErrorSection(p_error);
        CSmallString error;
        error << "unable to submit job, error: "  << p_error;
        ES_TRACE_ERROR(error);
        return(false);
    }

    // write submit flag
    job.WriteSubmitSection(p_jobid);

    free(p_jobid);
    return(true);
}

//------------------------------------------------------------------------------

bool CPBSProServer::GetJobStatus(CJob& job)
{
    CSmallString jobid = job.GetJobID();

    batch_status* p_status = pbs_statjob(ServerID,jobid.GetBuffer(),NULL,NULL);

    job.BatchJobComment = NULL;
    job.BatchJobStatus = EJS_ERROR;

    if( p_status == NULL ){
        return(true);
    }

    attrl* p_item = p_status->attribs;

    DecodeBatchJobComment(p_item,job.BatchJobComment);

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
            }
        }

        p_item = p_item->next;
    }

    pbs_statfree(p_status);

    return(true);
}

//------------------------------------------------------------------------------

void CPBSProServer::DecodeBatchJobComment(attrl* p_item,CSmallString& comment)
{
    comment = "";

    while( p_item != NULL ){
        // job comment
        if( strcmp(p_item->name,ATTR_JOB_COMMENT) == 0 ){
            if( comment == NULL ) {
                comment = p_item->value;
            }
        }

        if( strcmp(p_item->name,ATTR_JOB_PLANNED_START) == 0 ){
            CSmallString tmp = p_item->value;

            CSmallTimeAndDate   date(tmp.ToLInt());
            CSmallTimeAndDate   cdate;
            CSmallTime          ddiff;
            cdate.GetActualTimeAndDate();
            ddiff = date - cdate;
            if( ddiff > 0 ){
                comment = "planned start within " + ddiff.GetSTimeAndDay();
            }
        }

        if( strcmp(p_item->name,ATTR_JOB_PLANNED_NODES) == 0 ){
            vector<string> nodes;
            string         list(p_item->value);
            split(nodes,list,is_any_of(" ,"),boost::token_compress_on);
            if( nodes.size() >= 1 ){
                comment += " (" + nodes[0];
                if( nodes.size() > 1 ) comment += ",+";
                comment += ")";
            }
        }

        p_item = p_item->next;
    }
}

//------------------------------------------------------------------------------

bool CPBSProServer::KillJob(CJob& job)
{
    CSmallString jobid = job.GetJobID();
    int retval = pbs_deljob(ServerID,jobid.GetBuffer(),NULL);
    return( retval == 0 );
}

//------------------------------------------------------------------------------

bool CPBSProServer::KillJobByID(const CSmallString& jobid)
{
    CSmallString lid(jobid);
    int retval = pbs_deljob(ServerID,lid.GetBuffer(),NULL);
    return( retval == 0 );
}

//------------------------------------------------------------------------------

const CSmallString CPBSProServer::GetLastErrorMsg(void)
{
    return(pbs_geterrmsg(ServerID));
}

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

