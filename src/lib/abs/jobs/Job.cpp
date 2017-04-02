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

#include <Job.hpp>
#include <XMLIterator.hpp>
#include <ErrorSystem.hpp>
#include <iomanip>
#include <Shell.hpp>
#include <DirectoryEnum.hpp>
#include <FileSystem.hpp>
#include <XMLPrinter.hpp>
#include <NodeList.hpp>
#include <string.h>
#include <ABSConfig.hpp>
#include <XMLParser.hpp>
#include <AliasList.hpp>
#include <Utils.hpp>
#include <ShellProcessor.hpp>
#include <boost/algorithm/string/split.hpp>
#include <boost/algorithm/string/classification.hpp>
#include <boost/algorithm/string/join.hpp>
#include <JobList.hpp>
#include <PluginDatabase.hpp>
#include <PluginObject.hpp>
#include <CategoryUUID.hpp>
#include <JobType.hpp>
#include <iostream>
#include <vector>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <AMSGlobalConfig.hpp>
#include <BatchServers.hpp>
#include <ResourceList.hpp>
#include <sys/types.h>
#include <grp.h>
#include <sstream>
#include <Host.hpp>

//------------------------------------------------------------------------------

using namespace std;
using namespace boost;
using namespace boost::algorithm;

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

CJob::CJob(void)
{
    InfoFileLoaded = false;
    DoNotSave = false;
}

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

bool CJob::LoadInfoFile(const CSmallString& filename)
{
    // clean
    RemoveAllChildNodes();

    InfoFileLoaded = false;

    ifstream ifs(filename);
    string header;
    ifs >> header;
    ifs.close();
    if( header != "<?xml"){
        CSmallString error;
        error << "unable to parse info file '" << filename << "' - not XML file";
        ES_TRACE_ERROR(error);
        return(false);
    }

    try {
        CXMLParser xml_parser;
        xml_parser.SetOutputXMLNode(this);

        if( xml_parser.Parse(filename) == false ){
            CSmallString error;
            error << "unable to parse info file '" << filename << "'";
            ES_TRACE_ERROR(error);
            return(false);
        }
    } catch(...){
        CSmallString error;
        error << "unable to parse info file '" << filename << "'";
        ES_TRACE_ERROR(error);
        return(false);
    }

    InfoFileLoaded = true;
    return(true);
}

//------------------------------------------------------------------------------

bool CJob::LoadInfoFile(void)
{
    CFileName name = GetJobPath() / GetFullJobName() + ".info";
    if( CFileSystem::IsFile(name) == false ){
        DoNotSave = true;
        return(false);
    }
    if( LoadInfoFile(name) == false ){
        DoNotSave = true;
        return(false);
    }
    return(true);
}

//------------------------------------------------------------------------------

bool CJob::SaveInfoFile(void)
{
    if( DoNotSave == true ) return(true);
    CFileName name = GetJobPath() / GetFullJobName() + ".info";
    return( SaveInfoFile(name) );
}

//------------------------------------------------------------------------------

bool CJob::SaveInfoFileWithPerms(void)
{
    if( DoNotSave == true ) return(true);
    CFileName name = GetJobPath() / GetFullJobName() + ".info";
    bool result = SaveInfoFile(name);
    if( result == false ) return(false);

// permissions
    // FIXME
    CSmallString sumask = GetItem("specific/resources","INF_UMASK");
    mode_t umask = CUser::GetUMaskMode(sumask);

    int mode = 0666;
    int fmode = (mode & (~ umask)) & 0777;
    CFileSystem::SetPosixMode(name,fmode);

    CSmallString sgroup = GetItem("specific/resources","INF_UGROUP");
    gid_t group = User.GetGroupID(sgroup);
    int ret = chown(name,-1,group);
    if( ret != 0 ){
        CSmallString warning;
        warning << "unable to set owner and group of file '" << name << "' (" << ret << ")";
        ES_WARNING(warning);
    }

    return(true);
}

//------------------------------------------------------------------------------

bool CJob::SaveInfoFile(const CFileName& name)
{
    CXMLPrinter xml_printer;

    xml_printer.SetPrintedXMLNode(this);
    if( xml_printer.Print(name) == false ){
        CSmallString error;
        error << "unable to save the job info file '" << name << "'";
        ES_ERROR(error);
        return(false);
    }

    return(true);
}

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

void CJob::CreateHeader(void)
{
    CreateChildDeclaration();

    CXMLElement* p_header = CreateChildElementByPath("job_info/infinity");

    CSmallString version;

    version = "INFINITY_INFO_v_3_0";

    p_header->SetAttribute("version",version);
    p_header->SetAttribute("site",AMSGlobalConfig.GetActiveSiteID());
}

//------------------------------------------------------------------------------

void CJob::CreateBasicSection(void)
{
    CreateSection("basic");
}

//------------------------------------------------------------------------------

void CJob::SetExternalOptions(void)
{
    WriteInfo("basic/external","INF_EXTERNAL_NAME_SUFFIX",false);
    WriteInfo("basic/external","INF_EXTERNAL_VARIABLES",false);
    WriteInfo("basic/external","INF_EXTERNAL_START_AFTER",false);
}

//------------------------------------------------------------------------------

bool CJob::SetArguments(const CSmallString& dst,const CSmallString& js,
                        const CSmallString& res)
{
    SetItem("basic/arguments","INF_ARG_DESTINATION",dst);
    SetItem("basic/arguments","INF_ARG_JOB",js);
    SetItem("basic/arguments","INF_ARG_RESOURCES",res);
    return(true);
}

//------------------------------------------------------------------------------

bool CJob::SetOutputNumber(std::ostream& sout,int number)
{
    if( (number < 0) || (number > 999) ){
        sout << endl;
        sout << "<b><red> ERROR: Parametric job number (" << number << ") must be in the range from 1 to 999!</red></b>" << endl;
        return(false);
    }

    stringstream str1;
    str1 << "#" << setw(3) << setfill('0') << number;
    SetItem("basic/external","INF_EXTERNAL_NAME_SUFFIX",str1.str());

    stringstream str2;
    str2 << "pjob" << setw(3) << setfill('0') << number;

    CSmallString tmp = CSmallString(str2.str());

    SetItem("basic/arguments","INF_OUTPUT_SUFFIX",tmp);

    if( CFileSystem::CreateDir(str2.str()) == false ){
        CSmallString error;
        error << "unable to create job output directory '" << tmp << "'";
        ES_TRACE_ERROR(error);
        return(false);
    }

    return(true);
}

//------------------------------------------------------------------------------

bool CJob::CheckRuntimeFiles(std::ostream& sout,bool ignore)
{
    bool            runtime_files = false;
    CSmallString    filters = "*.info;*.stdout;*.infex;*.infout;*.nodes;*.gpus;*.infkey;___JOB_IS_RUNNING___";
    char*           p_filter;
    char*           p_buffer = NULL;

    p_filter = strtok_r(filters.GetBuffer(),";",&p_buffer);
    while( p_filter ){
        // detect run-time file
        CDirectoryEnum direnum;
        CFileName      file;
        direnum.SetDirName(".");
        direnum.StartFindFile(p_filter);
        runtime_files = direnum.FindFile(file);
        if( runtime_files ) break;
        p_filter = strtok_r(NULL,";",&p_buffer);
    }

    if( runtime_files == false ) return(true);

    CSmallString var;
    var = CShell::GetSystemVariable("INF_IGNORE_RUNTIME_FILES");
    if( var == NULL ){
        ABSConfig.GetUserConfigItem("INF_IGNORE_RUNTIME_FILES",var);
    }

    if( (var == "YES") || (ignore == true) ){
        sout << endl;
        sout << "<b><blue> WARNING: Input directory contains runtime files!</blue></b>" << endl;
        return(true);
    }

    return(false);
}

//------------------------------------------------------------------------------

ERetStatus CJob::JobInput(std::ostream& sout)
{
    // test arg job name
    if( CheckJobName(sout) == false ){
        ES_TRACE_ERROR("illegal job name");
        return(ERS_FAILED);
    }

    CSmallString tmp;

    // get job directory -------------------------

// this does not follow symlinks
//    CFileSystem::GetCurrentDir(tmp);
    tmp = GetJobInputPath();

    tmp = JobPathCheck(tmp,sout);
    if( tmp == NULL ){
        ES_TRACE_ERROR("illegal job dir");
        return(ERS_FAILED);
    }   
    SetItem("basic/jobinput","INF_JOB_PATH",tmp);

    tmp = CShell::GetSystemVariable("HOSTNAME");
    SetItem("basic/jobinput","INF_JOB_MACHINE",tmp);

    // detect job type
    ERetStatus rstat = DetectJobType(sout);
    if( rstat == ERS_FAILED ){
        ES_TRACE_ERROR("error during job type detection");
        return(ERS_FAILED);
    }
    if( rstat == ERS_TERMINATE ){
        return(ERS_TERMINATE);
    }

    // detect job project
    DetectJobProject();

    // detect collection, if any
    DetectJobCollection();

    // -------------------------------------------
    CSmallString name,affix,title;

    name = GetItem("basic/jobinput","INF_JOB_NAME");
    affix = GetItem("basic/external","INF_EXTERNAL_NAME_SUFFIX");

    if( affix != NULL ){
        title = name + affix;
    } else {
        title = name;
    }

    if( title == NULL ){
        ES_ERROR("job initial title is empty string");
        return(ERS_FAILED);
    }

    // check first character
    if( isalpha(title.GetBuffer()[0]) == 0 ){
        sout << endl;
        sout << "<blue> WARNING: The job name contains a non-alphabetical character at the first position!</blue>" << endl;
        sout << "          The character 'P' will be prepend to the job name." << endl;
        sout << "          (The change affects the job title only.)" << endl;
        title = "P" + title;
    }

    // check length
    if( title.GetLength() > 15 ){
        sout << endl;
        sout << "<blue> WARNING: The job name is longer than 15 characters!</blue>" << endl;
        sout << "          The name will be truncated." << endl;
        sout << "          (The change affects the job title only.)" << endl;

        if( affix == NULL ){
            title = title.GetSubString(0,15);
        } else {
            int affixlen = affix.GetLength();
            if( affixlen > 4 ){
                affixlen = 4;
            }
            int namelen = name.GetLength();
            if( namelen+affixlen > 15 ){
                namelen = 15 - affixlen;
            }
            if( name.GetLength() + 4 > 15 ){
                title = name.GetSubString(0,namelen) +
                            affix.GetSubString(0,affixlen);
            }
        }
    }

    SetItem("basic/jobinput","INF_JOB_TITLE",title);
    SetItem("basic/modules","INF_EXPORTED_MODULES",AMSGlobalConfig.GetExportedModules());

    // unique job key
    SetItem("basic/jobinput","INF_JOB_KEY",CUtils::GenerateUUID());

    return(ERS_OK);
}

//------------------------------------------------------------------------------

bool CJob::DecodeResources(std::ostream& sout,bool expertmode)
{
    // input directory
    if( InputDirectory() == false ){
        ES_TRACE_ERROR("unable to setup resources for the input directory");
        return(false);
    }

    // input from user
    CSmallString dest = GetItem("basic/arguments","INF_ARG_DESTINATION");
    CSmallString sres = GetItem("basic/arguments","INF_ARG_RESOURCES");

    // decode destination if necessary
    CSmallString queue;
    if( BatchServers.DecodeQueueName(dest,BatchServerName,ShortServerName,queue) == false ){
        ES_TRACE_ERROR("unable to decode job destination (queue[@server])");
        return(false);
    }

    bool result = true;

    // add default resources
    CSmallString  sdef_res;
    if( ABSConfig.GetSystemConfigItem("INF_DEFAULT_RESOURCES",sdef_res) ){
        ResourceList.AddResources(sdef_res,sout,result,expertmode);
        if( result == false ){
            ES_TRACE_ERROR("default resources are invalid");
            return(false);
        }
        SetItem("specific/resources","INF_DEFAULT_RESOURCES",sdef_res);
    }

    // is destination alias?
    CAliasPtr p_alias = AliasList.FindAlias(dest);
    if( p_alias ){
        SetItem("specific/resources","INF_ALIAS",p_alias->GetName());
        // decode destination
        if( BatchServers.DecodeQueueName(p_alias->GetDestination(),BatchServerName,ShortServerName,queue) == false ){
            ES_TRACE_ERROR("unable to decode job destination (queue[@server])");
            return(false);
        }
        ResourceList.AddResources(p_alias->GetResources(),sout,result,expertmode);
        if( result == false ){
            ES_TRACE_ERROR("alias resources are invalid");
            return(false);
        }
        SetItem("specific/resources","INF_ALIAS_RESOURCES",p_alias->GetResources());
    } else {
        SetItem("specific/resources","INF_ALIAS","");
    }

    SetItem("specific/resources","INF_QUEUE",queue);
    SetItem("specific/resources","INF_SERVER",BatchServerName);

    // decode user resources
    ResourceList.AddResources(sres,sout,result,expertmode);
    if( result == false ){
        ES_TRACE_ERROR("user resources are invalid");
        return(false);
    }

    // resolve conflicts
    ResourceList.ResolveConflicts();

    // test resources
    ResourceList.TestResourceValues(sout,result);
    if( result == false ){
        ES_TRACE_ERROR("some resource value is invalid");
        return(false);
    }

    // calculate dynamic resources
    ResourceList.ResolveDynamicResources();

    // determine batch resources
    CBatchServerPtr srv_ptr = BatchServers.FindBatchServer(BatchServerName,true);
    if( srv_ptr == NULL ){
        ES_TRACE_ERROR("unable to init batch server");
        return(false);
    }
    if( srv_ptr->InitBatchResources(&ResourceList) == false ){
        ES_TRACE_ERROR("unable to init batch resources");
        return(false);
    }

    // set final resources
    SetItem("specific/resources","INF_NCPU",ResourceList.GetNumOfCPUs());
    SetItem("specific/resources","INF_NGPU",ResourceList.GetNumOfGPUs());
    SetItem("specific/resources","INF_NNODE",ResourceList.GetNumOfNodes());
    SetItem("specific/resources","INF_MEMORY",ResourceList.GetMemoryString());
    SetItem("specific/resources","INF_WALLTIME",ResourceList.GetWallTimeString());
    SetItem("specific/resources","INF_RESOURCES",ResourceList.ToString(false));

    // setup working directory
    if( WorkDirectory() == false ){
        ES_TRACE_ERROR("unable to setup resources for the working directory");
        return(false);
    }

    return(true);
}

//------------------------------------------------------------------------------

bool CJob::InputDirectory(void)
{
    CSmallString input_machine = GetItem("basic/jobinput","INF_JOB_MACHINE");
    CSmallString input_path = GetItem("basic/jobinput","INF_JOB_PATH");
    CSmallString cwd;
    CFileSystem::GetCurrentDir(cwd);
    string       input_path_raw(cwd);     // use cwd instead of PWD

// determine FS type of input directory and group namespace,
// storage name and storage directory

    struct stat job_dir_stat;
    if( stat(input_path_raw.c_str(),&job_dir_stat) != 0 ){
        ES_ERROR("unable to stat CWD");
        return(false);
    }

//    mode_t input_path_umask = (job_dir_stat.st_mode ^ 0x777) & 0x777;
//    gid_t  input_path_uid = job_dir_stat.st_uid;
//    gid_t  input_path_gid = job_dir_stat.st_gid;

    unsigned int minid = minor(job_dir_stat.st_dev);
    unsigned int majid = major(job_dir_stat.st_dev);
    stringstream sdev;
    sdev << majid << ":" << minid;

// find mount point
    ifstream mountinfo("/proc/self/mountinfo");
    string   mntpoint;
    getline(mountinfo,mntpoint);
    bool    found = false;
    while( mountinfo ){
        stringstream smntpoint(mntpoint);
        string n1,n2,s1;
        smntpoint >> n1 >> n2 >> s1;
        if( s1 == sdev.str() ){
            found = true;
            break;
        }
        mntpoint = "";
        getline(mountinfo,mntpoint);
    }

    if( ! found ){
        ES_ERROR("unable to find mount point");
        return(false);
    }

// parse mntpoint
    vector<string> items;
    split(items,mntpoint,is_any_of(" "),boost::token_compress_on);
    size_t p1 = 0;
    size_t p2 = 0;
    bool   nonopt = false;
    vector<string>::iterator it = items.begin();
    vector<string>::iterator ie = items.end();

    string dest, fstype, src, opts;

    while( it != ie ){
        p1++;
        if( nonopt ) p2++;
        if( p1 == 4 ) dest = *it;
        if( p2 == 1 ) fstype = *it;
        if( p2 == 2 ) src = *it;
        if( p2 == 3 ) opts = *it;
        if( *it == "-" ) nonopt = true;
        it++;
    }

// fix fstype for nfs4
    if( fstype == "nfs4" ){
        if( opts.find("krb5") != string::npos ){
            fstype = "nfs4:krb5";
        } else {
            fstype = "nfs4:sys";
        }
    }

// determine input machine, storage machine and batch system group namespaces
    CSmallString input_machine_groupns      = Host.GetGroupNS();
    CSmallString storage_machine_groupns    = Host.GetGroupNS();
    CSmallString batch_server_groupns       = BatchServers.GetBatchGroupNS();

// determine storage machine and storage path
    CSmallString storage_machine = input_machine;
    CSmallString storage_path = input_path;

    if( (fstype == "nfs4:krb5") || (fstype == "nfs4:sys") ){
        // determine server name and data directory
        string smach = src.substr(0,src.find(":"));
        string spath = src.substr(src.find(":")+1,string::npos);
        if( input_path_raw.find(dest) == string::npos ){
            CSmallString error;
            error << "mnt dest (" << dest << ") is not root of cwd (" << input_path_raw << ")";
            ES_ERROR(error);
            return(false);
        }
        if( spath == "/" ) spath = ""; // remove root '/' character
        spath = spath + input_path_raw.substr(dest.length(),string::npos);
        storage_machine_groupns = Host.GetGroupNS(smach);

        // FIXME
        storage_machine = smach;
        storage_path = spath;
    }

// input storage
    SetItem("specific/resources","INF_INPUT_PATH_FSTYPE",fstype);

    SetItem("specific/resources","INF_STORAGE_MACHINE",storage_machine);
    SetItem("specific/resources","INF_STORAGE_PATH",storage_path);
    SetItem("specific/resources","INF_STORAGE_GROUPNS",storage_machine_groupns);

    SetItem("specific/resources","INF_INPUT_MACHINE_GROUPNS",input_machine_groupns);
    SetItem("specific/resources","INF_BATCH_SERVER_GROUPNS",batch_server_groupns);

    // set default user group and umask
    SetItem("specific/resources","INF_UGROUP",ResourceList.GetResourceValue("group"));
    SetItem("specific/resources","INF_UMASK",ResourceList.GetResourceValue("umask"));
    return(true);
}

//------------------------------------------------------------------------------

bool CJob::WorkDirectory(void)
{
    // setup specific items for working directory
    SetItem("specific/resources","INF_DATAIN",ResourceList.GetResourceValue("datain"));
    SetItem("specific/resources","INF_DATAOUT",ResourceList.GetResourceValue("dataout"));
    SetItem("specific/resources","INF_WORK_DIR_TYPE",ResourceList.GetResourceValue("workdir"));
    SetItem("specific/resources","INF_WORK_SIZE",ResourceList.GetWorkSizeString());
    return(true);
}

//------------------------------------------------------------------------------

bool CJob::LastJobCheck(std::ostream& sout)
{
    CSmallString check_suuid = GetItem("basic/jobinput","INF_JOB_CHECK",true);
    if( check_suuid == NULL ) return(true); // no last check was requested

    CSmallString job_type = GetItem("basic/jobinput","INF_JOB_TYPE");
    CExtUUID check_uuid(check_suuid);

    CComObject* p_obj = PluginDatabase.CreateObject(check_uuid);
    if( p_obj == NULL ){
        CSmallString error;
        error << "unable to create object for job type '" << job_type
              << "' which should be '" << check_uuid.GetFullStringForm() << "'";
        ES_ERROR(error);
        return(false);
    }
    CJobType* p_jobj = dynamic_cast<CJobType*>(p_obj);
    if( p_jobj == NULL ){
        delete p_obj;
        CSmallString error;
        error << "object for job type '" << job_type
              << "', which is '" << check_uuid.GetFullStringForm() << "', is not CJobType";
        ES_ERROR(error);
        return(false);
    }
    if( p_jobj->CheckInputFile(*this,sout) == false ){
        delete p_obj;
        ES_TRACE_ERROR("job check failed");
        return(false);
    }
    delete p_obj;
    return(true);
}

//------------------------------------------------------------------------------

bool CJob::ShouldSubmitJob(std::ostream& sout,bool assume_yes)
{
    // do not ask if the job is part of collection
    if( GetItem("basic/collection","INF_COLLECTION_NAME",true) != NULL ) return(true);

    // do not ask if it is not required by options
    if( assume_yes ) return(true);

    CSmallString var;
    var = CShell::GetSystemVariable("INF_CONFIRM_SUBMIT");
    if( var == NULL ){
        ABSConfig.GetUserConfigItem("INF_CONFIRM_SUBMIT",var);
    }
    var.ToUpperCase();
    if( var == "NO" ) return(true);


    sout << endl;
    sout << "Do you want to submit the job to the Torque server (YES/NO)?" << endl;
    sout << "> ";

    string answer;
    cin >> answer;

    CSmallString sanswer(answer.c_str());
    sanswer.ToUpperCase();

    if( sanswer == "YES" ) return(true);

    sout << "Job was NOT submited to the Torque server!" << endl;

    return(false);
}

//------------------------------------------------------------------------------

bool CJob::SubmitJob(const CJobPtr& self,std::ostream& sout,bool siblings)
{   
    CFileName job_script;
    job_script = GetFullJobName() + ".infex";

    // copy execution script to job directory
    CFileName infex_script;
    infex_script = ABSConfig.GetABSRootDir() / "share" / "scripts" / "abs-execution-script-L0";

    if( CFileSystem::CopyFile(infex_script,job_script,true) == false ){
        ES_ERROR("unable to copy startup script to the job directory");
        return(false);
    }

    CFileName user_script;
    user_script = GetItem("basic/jobinput","INF_JOB_NAME",true);

    if( ! siblings ) {
        // FIXME
        CSmallString sumask = GetItem("specific/resources","INF_UMASK");
        mode_t umask = CUser::GetUMaskMode(sumask);

        int mode = 0766;
        int fmode = (mode & (~ umask)) & 0777;
        CFileSystem::SetPosixMode(job_script,fmode);

        CSmallString sgroup = GetItem("specific/resources","INF_UGROUP");
        gid_t group = User.GetGroupID(sgroup);

        int ret = chown(job_script,-1,group);
        if( ret != 0 ){
            CSmallString warning;
            warning << "unable to set owner and group of file '" << job_script << "' (" << ret << ")";
            ES_WARNING(warning);
        }

        if( IsInteractiveJob() == false ){
            CFileSystem::SetPosixMode(user_script,fmode);
            int ret = chown(user_script,-1,group);
            if( ret != 0 ){
                CSmallString warning;
                warning << "unable to set owner and group of file '" << user_script << "' (" << ret << ")";
                ES_WARNING(warning);
            }
        }
    }

    if( GetItem("basic/collection","INF_COLLECTION_NAME",true) == NULL ) {
        // submit job to torque
        if( BatchServers.SubmitJob(*this) == false ){
            if( ! siblings ){
                sout << "<b><red>Job was NOT submited to the Torque server!</red></b>" << endl;
                sout << "  > Reason: " << GetLastError() << endl;
            } else {
                CSmallString tmp = GetItem("basic/arguments","INF_OUTPUT_SUFFIX");
                sout << "<b><red>" << tmp << " -> unable to submit the job: " << GetLastError() << "</red></b>" << endl;
            }
            // remove infex script
            if( ! siblings ) {
                CFileSystem::RemoveFile(job_script);
            }
            ES_TRACE_ERROR("unable to submit job");
            return(false);
        }
        if( ! siblings ){
            sout << "Job was sucessfully submited to the Torque server!" << endl;
            sout << "  > Job ID: " << low << GetJobID() << endl << medium;

            if( user_script == "cli" ){
                sout << endl;
                sout << "<b><blue>NOTE: This is an interactive job employing a command line interface (CLI).</blue></b>" << endl;
                sout << "<b><blue>      Once the job is running, use the pgo command to establish an interactive connection.</blue></b>" << endl;
            }

            if( user_script == "gui" ){
                sout << endl;
                sout << "<b><blue>NOTE: This is an interactive job employing a graphics user interface (GUI).</blue></b>" << endl;
                sout << "<b><blue>      Once the job is running, use the pgo command to establish an interactive connection.</blue></b>" << endl;
            }

        } else {
            CSmallString tmp = GetItem("basic/arguments","INF_OUTPUT_SUFFIX");
            sout << "  > " << tmp << " -> " << GetJobID() << endl;
        }

    } else {
        CJobList     collection;
        CSmallString coll_name = GetItem("basic/collection","INF_COLLECTION_NAME");
        CSmallString coll_path = GetItem("basic/collection","INF_COLLECTION_PATH");
        CSmallString coll_id = GetItem("basic/collection","INF_COLLECTION_ID");
        CSmallString site_name = GetSiteName();

        if( collection.LoadCollection(coll_path,coll_name,coll_id,site_name) == false ){
            ES_ERROR("unable to load collection");
            return(false);
        }

        // add this job into collection
        collection.AddJob(self);

        // save collection file
        if( collection.SaveCollection() == false ){
            ES_ERROR("unable to save collection");
            return(false);
        }
        if( ! siblings ){
            sout << "Job was sucessfully added into the collection '" << coll_name << "'!" << endl;
        } else {
            CSmallString tmp = GetItem("basic/arguments","INF_OUTPUT_SUFFIX");
            sout << "  > " << tmp << " -> added into the collection" << endl;
        }
    }

    return(true);
}

//------------------------------------------------------------------------------

bool CJob::ResubmitJob(void)
{
    // destroy start/stop/kill sections
    DestroySection("submit");
    DestroySection("start");
    DestroySection("stop");
    DestroySection("kill");

    CSmallString curr_dir;
//  this does not follow symlinks
//  CFileSystem::GetCurrentDir(curr_dir);
    curr_dir = GetJobInputPath();

    // go to job directory
    CFileSystem::SetCurrentDir(GetJobPath());

    // submit job to torque
    if( BatchServers.SubmitJob(*this) == false ){
        ES_TRACE_ERROR("unable to resubmit job to torque");
        BatchJobComment = GetLastError();
        CFileSystem::SetCurrentDir(curr_dir);
        return(false);
    }

    // save info file
    if( SaveInfoFile() == false ){
        ES_TRACE_ERROR("unable to save job info file");
        CFileSystem::SetCurrentDir(curr_dir);
        return(false);
    }

    CFileSystem::SetCurrentDir(curr_dir);
    return(true);
}

//------------------------------------------------------------------------------

void CJob::WriteSubmitSection(char* p_jobid)
{
    CreateSection("submit");
    SetItem("submit/job","INF_JOB_ID",p_jobid);
}

//------------------------------------------------------------------------------

void CJob::WriteErrorSection(char* p_error)
{
    CreateSection("error");
    SetItem("error/last","INF_ERROR",p_error);
}

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

bool CJob::GetJobInfoFileName(CSmallString& info_file_name)
{
    CSmallString ext;

    // complete info file name ----------------------
    info_file_name = CShell::GetSystemVariable("INF_JOB_NAME");
    if( info_file_name == NULL ){
        ES_ERROR("INF_JOB_NAME is empty string");
        return(false);
    }
    ext = CShell::GetSystemVariable("INF_JOB_NAME_SUFFIX");
    if( ext != NULL ){
        info_file_name += ext;
    }
    info_file_name += ".info";
    return(true);
}

//------------------------------------------------------------------------------

bool CJob::WriteStart(void)
{
// create section -------------------------------
    CreateSection("start");

// workdir --------------------------------------
    if( WriteInfo("start/workdir","INF_MAIN_NODE",true) == false ){
        ES_ERROR("unable to get INF_MAIN_NODE");
        return(false);
    }
    if( WriteInfo("start/workdir","INF_WORK_DIR",true) == false ){
        ES_ERROR("unable to get INF_WORK_DIR");
        return(false);
    }

// nodes ----------------------------------------
    CXMLElement* p_ele;
    p_ele = GetElementByPath("start/nodes",true);

    CSmallString nodefile;
    nodefile = CShell::GetSystemVariable("INF_NODEFILE");
    if( nodefile == NULL ){
        ES_ERROR("node file is not set in INF_NODEFILE");
        return(false);
    }

    ifstream snodefile;
    snodefile.open(nodefile);
    if( ! snodefile ){
        CSmallString error;
        error << "unable to open node file '" << nodefile << "'";
        ES_ERROR(error);
        return(false);
    }

    string node;
    while( snodefile && getline(snodefile,node) ){
        CXMLElement* p_sele = p_ele->CreateChildElement("node");
        p_sele->SetAttribute("name",node);
    }

    snodefile.close();

// gpus ----------------------------------------
    p_ele = GetElementByPath("start/gpus",true);

    nodefile = CShell::GetSystemVariable("INF_GPUFILE");
    if( nodefile == NULL ){
        return(true);       // END - no gpu nodes
    }

    snodefile.open(nodefile);
    if( ! snodefile ){
        CSmallString error;
        error << "unable to open gpu node file '" << nodefile << "'";
        ES_ERROR(error);
        return(false);
    }

    while( snodefile && getline(snodefile,node) ){
        if( node.find("-gpu") == string::npos ) continue;
        // only gpu nodes
        CXMLElement* p_sele = p_ele->CreateChildElement("gpu");
        p_sele->SetAttribute("name",node);
    }

    snodefile.close();

    return(true);
}

//------------------------------------------------------------------------------

bool CJob::WriteTerminalReady(void)
{
// create section -------------------------------
    CreateSection("terminal");

    return(true);
}

//------------------------------------------------------------------------------

bool CJob::WriteStop(void)
{
// create section -------------------------------
    CreateSection("stop");

// jobstatus ------------------------------------
    if( WriteInfo("stop/result","INF_JOB_EXIT_CODE",true) == false ){
        ES_ERROR("unable to get INF_JOB_EXIT_CODE");
        return(false);
    }

    return(true);
}

//------------------------------------------------------------------------------

bool CJob::WriteInfo(const CSmallString& category,const CSmallString& name,bool error_on_empty_value)
{
    if( name == NULL ){
        ES_ERROR("name is empty string");
        return(false);
    }

    CSmallString value;
    value = CShell::GetSystemVariable(name);
    if( (value == NULL) && (error_on_empty_value == true) ){
        CSmallString error;
        error << "value is empty for " << name << " key";
        ES_ERROR(error);
        return(false);
    }

    SetItem(category,name,value);
    return(true);
}

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

bool CJob::KillJob(bool force)
{
    if( force ){
        CSmallString id = GetItem("submit/job","INF_JOB_ID",true);
        if( id == NULL ){
            id = GetItem("batch/job","INF_JOB_ID",true);
            id += "." + BatchServerName;
        }
        return(BatchServers.KillJobByID(id));
    }

    // normal termination
    if( GetSiteID() != AMSGlobalConfig.GetActiveSiteID() ){
        // job was run under different site
        ES_TRACE_ERROR("job runs under different site");
        return(false);
    }

    switch( GetJobStatus() ){
        case EJS_SUBMITTED:
        case EJS_RUNNING:
            // try to kill job
            return(BatchServers.KillJob(*this));
        default:
            ES_ERROR("unable to kill job that is not running or queued");
            return(false);
    }

    // update info file
    CreateSection("kill");

    return(true);
}

//------------------------------------------------------------------------------

bool CJob::UpdateJobStatus(void)
{
    if( GetSiteID() != AMSGlobalConfig.GetActiveSiteID() ){
        // job was run under different site
        ES_TRACE_ERROR("job runs under different site");
        return(true);
    }

    switch( GetJobInfoStatus() ){
        case EJS_SUBMITTED:
        case EJS_RUNNING:
            break;
        default:
            // job was not submitted neither is running
            BatchJobStatus = GetJobInfoStatus();
            return(true);
    }

    // get job status and reason

    if( BatchServers.GetJobStatus(*this) == false ){
        ES_ERROR("unable to update job status");
        return(false);
    }

    return(true);
}

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

bool CJob::CheckJobName(std::ostream& sout)
{
    string arg_job_name;
    arg_job_name = GetItem("basic/arguments","INF_ARG_JOB");

    string legall_characters = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ01234567890_+-.#";
    string legall_char_short = "a-z A-Z 0-9 _+-.#";

    if( arg_job_name.find_first_not_of(legall_characters) != string::npos ){
        sout << endl;
        sout << "<b><red> ERROR: The job name '<u>" << arg_job_name << "</u>' contains illegal characters!</red></b>" << endl;
        sout <<         "        Legal characters are : " << legall_char_short << endl;
        return(false);
    }

    if( (arg_job_name != "cli") && (arg_job_name != "gui") ){
        // check if job input data or script exists --
        if( ! CFileSystem::IsFile(arg_job_name.c_str()) ){
            sout << endl;
            sout << "<b><red> ERROR: The job name '<u>" << arg_job_name << "</u>' does not represent any file!</red></b>";
            sout << endl;
            return(false);
        }
    }

    return(true);
}

//------------------------------------------------------------------------------

const CSmallString CJob::JobPathCheck(const CSmallString& inpath,std::ostream& sout)
{
    CSmallString outpath = inpath;

// FIXME
//    string inrules;
//    inrules = ABSConfig.GetSystemConfigItem("INF_JOB_PATH_CHECK");

//    if( ! inrules.empty() ){
//        // try to remove specified prefixies until success
//        vector<string> rules;
//        split(rules,inrules,is_any_of(";"));

//        vector<string>::iterator it = rules.begin();
//        vector<string>::iterator ie = rules.end();
//        string sinpath(inpath);

//        while( it != ie ){
//            string rule = *it;
//            it++;
//            if( rule.empty() ) continue;

//            string::iterator ipos = find(rule.begin(), rule.end(), '|');
//            string prefix;
//            string substi;
//            if( ipos != rule.end() ){
//                prefix = string(rule.begin(), ipos);
//                substi = string(ipos + 1, rule.end());
//            } else {
//                prefix = rule;
//            }

//            size_t pos = sinpath.find(prefix);
//            if( pos == 0 ){
//                // strip down prefix
//                string new_path = sinpath.substr(prefix.size());
//                if( ! substi.empty() ){
//                    new_path = substi + new_path;
//                }
//                // does it exist?
//                if( CFileSystem::IsDirectory(new_path.c_str()) == true ){
//                    outpath = new_path;
//                    break;
//                }
//            }
//        }
//    }

    string legall_characters = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ01234567890_+-.#/";
    string legall_char_short = "a-z A-Z 0-9 _+-.#/";

    string soutpath(outpath);
    if( soutpath.find_first_not_of(legall_characters) != string::npos ){
        sout << endl;
        sout << "<b><red> ERROR: The job directory name '<u>" << soutpath << "</u>' contains illegal characters!</red></b>" << endl;
        sout <<         "        Legal characters are : " << legall_char_short << endl;
        return("");
    }

    // check if job dir is really directory
    if( ! CFileSystem::IsDirectory(outpath) ){
        sout << endl;
        sout << "<b><red> ERROR: The job directory '<u>" << outpath << "</u>' is not a directory!</red></b>";
        return("");
    }

    // is it writable?
    CFileName test_file;
    test_file = CFileName(outpath) / ".________INFINITY_______";
    ofstream ofs(test_file);
    ofs << "________INFINITY_______" << endl;
    if( ! ofs ){
        sout << endl;
        sout << "<b><red> ERROR: The job directory '<u>" << outpath << "</u>' is not writable!</red></b>" << endl;
        return("");
    } else {
        CFileSystem::RemoveFile(test_file);
    }

    // FIXME
    // warning if directory volume is nearly full

    return(outpath);
}

//------------------------------------------------------------------------------

ERetStatus CJob::DetectJobType(std::ostream& sout,CSmallString job_type)
{
    CSmallString arg_job_name;

    // default setup
    arg_job_name = GetItem("basic/arguments","INF_ARG_JOB");
    SetItem("basic/jobinput","INF_JOB_NAME",arg_job_name);
    SetItem("basic/jobinput","INF_JOB_TYPE","generic");

    // handle interactive jobs
    if( arg_job_name == "cli" ){
        SetItem("basic/jobinput","INF_JOB_TYPE","interactive");
        SetItem("basic/jobinput","INF_EXCLUDED_FILES","");
        return(ERS_OK);
    }
    if( arg_job_name == "gui" ){
        SetItem("basic/jobinput","INF_JOB_TYPE","interactive");
        SetItem("basic/jobinput","INF_EXCLUDED_FILES","");
        return(ERS_OK);
    }

    // handle exluded files
    CSmallString excluded;
    GetJobFileConfigItem("ExcludedFiles",excluded);
    SetItem("basic/jobinput","INF_EXCLUDED_FILES",excluded);

    // try to get job type
    if( (job_type != NULL) || GetJobFileConfigItem("JobType",job_type) ){
        // job is specified in the job input file

        // try to find job type plugin
        CSimpleIteratorC<CPluginObject>  I(PluginDatabase.GetObjectList());
        CPluginObject*                  p_pobj;
        bool                            detected = false;

        while( (p_pobj = I.Current()) != NULL ){
            I++;
            if( p_pobj->GetCategoryUUID() != JOB_TYPE_CAT ) continue;
            CSmallString key_value;
            if( PluginDatabase.FindObjectConfigValue(p_pobj->GetObjectUUID(),"_type",key_value) == false ){
                continue;
            }
            if( key_value != job_type ) continue;
            CComObject* p_obj = PluginDatabase.CreateObject(p_pobj->GetObjectUUID());
            if( p_obj == NULL ){
                CSmallString error;
                error << "unable to create object for job type '" << job_type
                      << "' which should be '" << p_pobj->GetObjectUUID().GetFullStringForm() << "'";
                ES_ERROR(error);
                break;
            }
            CJobType* p_jobj = dynamic_cast<CJobType*>(p_obj);
            if( p_jobj == NULL ){
                delete p_obj;
                CSmallString error;
                error << "object for job type '" << job_type
                      << "', which is '" << p_pobj->GetObjectUUID().GetFullStringForm() << "', is not CJobType";
                ES_ERROR(error);
                break;
            }
            ERetStatus retstat = p_jobj->DetectJobType(*this,detected,sout);
            delete p_obj;
            if( retstat == ERS_FAILED ){
                ES_TRACE_ERROR("job detection failed");
                return(retstat);
            }
            if( retstat == ERS_TERMINATE ){
                return(retstat);
            }
            break;
        }
        if( detected == false ){
            ES_TRACE_ERROR("unable to validate the job type specified in the job input file");
            sout << endl;
            sout << "<red> ERROR: Unable to validate the job type (" << job_type << ") specified in the job input file!</red>" << endl;
            sout << "<red>        Remove the line '# INFINITY JobType=" << job_type << "' from the input file.</red>" << endl;
            return(ERS_FAILED);
        }
        return(ERS_OK);
    }

    // try to autodetect job type
    CSimpleIteratorC<CPluginObject>  I(PluginDatabase.GetObjectList());
    CPluginObject*                  p_pobj;

    while( (p_pobj = I.Current()) != NULL ){
        I++;
        if( p_pobj->GetCategoryUUID() != JOB_TYPE_CAT ) continue;

        CSmallString mode_value;
        if( PluginDatabase.FindObjectConfigValue(p_pobj->GetObjectUUID(),"_mode",mode_value) == false ){
            continue;
        }
        // only implicit autodetection shoudl be probed here
        // explicit autodetection is probed above
        if( mode_value != "implicit" ) continue;

        CSmallString job_type;
        if( PluginDatabase.FindObjectConfigValue(p_pobj->GetObjectUUID(),"_type",job_type) == false ){
            // plugin must provide short name for given type
            continue;
        }

        CComObject* p_obj = PluginDatabase.CreateObject(p_pobj->GetObjectUUID());
        if( p_obj == NULL ){
            CSmallString error;
            error << "unable to create object '" << p_pobj->GetObjectUUID().GetFullStringForm() << " ' for job type autodetection";
            ES_ERROR(error);
            break;
        }
        CJobType* p_jobj = dynamic_cast<CJobType*>(p_obj);
        if( p_jobj == NULL ){
            delete p_obj;
            CSmallString error;
            error << "object '" << p_pobj->GetObjectUUID().GetFullStringForm() << "' is not CJobType";
            ES_ERROR(error);
            break;
        }
        bool detected = false;
        ERetStatus retstat = p_jobj->DetectJobType(*this,detected,sout);
        delete p_obj;
        if( retstat == ERS_FAILED ){
            ES_TRACE_ERROR("job detection failed");
            return(ERS_FAILED);
        }
        if( retstat == ERS_TERMINATE ) return(ERS_TERMINATE);
        if( detected == true ){
            break;
        }
    }

    return(ERS_OK);
}

//------------------------------------------------------------------------------

bool CJob::GetJobFileConfigItem(const CSmallString& key,CSmallString& value)
{
    value = NULL;
    CSmallString arg_job_name = GetItem("basic/arguments","INF_ARG_JOB");

    ifstream ifs(arg_job_name);
    if( ! ifs ){
        // no file
        return(false);
    }

    string line;
    while( getline(ifs,line) ){
        // split line into words
        vector<string> words;
        split(words,line,is_any_of("\t ="),boost::token_compress_on);
        if( words.size() == 4 ){
            if( (words[0] == "#") && (words[1] == "INFINITY") && (words[2] == string(key)) ){
                value = words[3];
                return(true);
            }
        }
    }

    return(false);
}

//------------------------------------------------------------------------------

void CJob::DetectJobProject(void)
{
    CSmallString project;
    CSmallString curdir;
// this does not follow symlinks
//    if( CFileSystem::GetCurrentDir(curdir) == false ){
//        ES_ERROR("unable to get current directory");
//        return;
//    }
    curdir = GetJobInputPath();
    project = GetJobProject(curdir);
    SetItem("basic/jobinput","INF_JOB_PROJECT",project);
}

//------------------------------------------------------------------------------

CSmallString CJob::GetJobProject(const CFileName& dir)
{
    CSmallString project;

    // first one level up
    CFileName updir = dir.GetFileDirectory();
    if( updir != NULL ){
        CSmallString ppath = GetJobProject(updir);
        project << ppath;
    }

    CFileName pfile = dir / ".project";
    if( ! CFileSystem::IsFile(pfile) ) {
        return(project);
    }

    // read project file
    ifstream ifs(pfile);
    string line;
    if( getline(ifs,line) ){
        if( project != NULL ) project << "/";
        project << line.c_str();
    }

    return(project);
}

//------------------------------------------------------------------------------

void CJob::DetectJobCollection(void)
{
    CSmallString coll_path = CShell::GetSystemVariable("INF_COLLECTION_PATH");
    CSmallString coll_name = CShell::GetSystemVariable("INF_COLLECTION_NAME");
    CSmallString coll_id   = CShell::GetSystemVariable("INF_COLLECTION_ID");

    if( (coll_path == NULL) || (coll_name == NULL) || (coll_id == NULL) ) {
        // no open collection
        return;
    }

    // setup collection
    SetItem("basic/collection","INF_COLLECTION_PATH",coll_path);
    SetItem("basic/collection","INF_COLLECTION_NAME",coll_name);
    SetItem("basic/collection","INF_COLLECTION_ID",coll_id);
}

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

bool CJob::CreateSection(const CSmallString& section)
{
    if( section == NULL ){
        ES_ERROR("section name is empty string");
        return(false);
    }

    CXMLElement* p_ele = GetElementByPath(section,true);
    if( p_ele == NULL ){
        CSmallString error;
        error << "unable to create section element by path '" << section << "'";
        ES_ERROR(error);
        return(false);
    }

    // remove previous contents
    p_ele->RemoveAllAttributes();
    p_ele->RemoveAllChildNodes();

    // set timing
    CSmallTimeAndDate timedate;
    timedate.GetActualTimeAndDate();

    p_ele->SetAttribute("timedate",timedate);
    p_ele->SetAttribute("stimedate",timedate.GetSDateAndTime());

    return(true);
}

//------------------------------------------------------------------------------

void CJob::DestroySection(const CSmallString& section)
{
    CXMLElement* p_ele = GetElementByPath(section,false);
    if( p_ele != NULL ) delete p_ele;
}

//------------------------------------------------------------------------------

CXMLElement* CJob::GetElementByPath(const CSmallString& p_path,bool create)
{
    return(GetChildElementByPath("job_info/"+p_path,create));
}

//------------------------------------------------------------------------------

CXMLElement* CJob::FindItem(CXMLElement* p_root,const char* p_name)
{
    CXMLIterator    I(p_root);
    CXMLElement*    p_sele;

    while( (p_sele = I.GetNextChildElement("item")) != NULL ){
        CSmallString name;
        bool result = true;
        result &= p_sele->GetAttribute("name",name);
        if( result == false ) return(NULL);
        if( strcmp(name,p_name) == 0 ) return(p_sele);
    }

    return(NULL);
}

//------------------------------------------------------------------------------

void CJob::SetItem(const CSmallString& path,const CSmallString& name,
                   const CSmallString& value)
{
    // find element ----------------------------
    CXMLElement* p_rele = GetElementByPath(path,true);

    CXMLElement* p_iele = FindItem(p_rele,name);
    if( p_iele == NULL ){
        p_iele = p_rele->CreateChildElement("item");
    }

    p_iele->SetAttribute("name",name);
    p_iele->SetAttribute("value",value);
}

//------------------------------------------------------------------------------

void CJob::SetTime(const CSmallString& path,const CSmallString& name,
                   const CSmallTime& value)
{
    // find element ----------------------------
    CXMLElement* p_rele = GetElementByPath(path,true);

    CXMLElement* p_iele = FindItem(p_rele,name);
    if( p_iele == NULL ){
        p_iele = p_rele->CreateChildElement("item");
    }

    p_iele->SetAttribute("name",name);
    p_iele->SetAttribute("value",value);
}

//------------------------------------------------------------------------------

bool CJob::GetItem(const CSmallString& path,const CSmallString& name,
                    CSmallString& value,bool noerror)
{
    CXMLElement* p_rele = GetElementByPath(path,false);
    if( p_rele == NULL ){
        CSmallString error;
        error << "unable to open element '" << path << "'";
        if( ! noerror ) ES_ERROR(error);
        return(false);
    }

    CXMLElement* p_iele = FindItem(p_rele,name);
    if( p_iele == NULL ){
        CSmallString error;
        error << "unable to find item '" << name << "'";
        if( ! noerror ) ES_ERROR(error);
        return(false);
    }

    p_iele->GetAttribute("value",value);

    return(false);
}

//------------------------------------------------------------------------------

bool CJob::GetTime(const CSmallString& path,const CSmallString& name,
                    CSmallTime& value,bool noerror)
{
    CXMLElement* p_rele = GetElementByPath(path,false);
    if( p_rele == NULL ){
        CSmallString error;
        error << "unable to open element '" << path << "'";
        if( ! noerror ) ES_ERROR(error);
        return(false);
    }

    CXMLElement* p_iele = FindItem(p_rele,name);
    if( p_iele == NULL ){
        CSmallString error;
        error << "unable to find item '" << name << "'";
        if( ! noerror ) ES_ERROR(error);
        return(false);
    }

    p_iele->GetAttribute("value",value);

    return(false);
}

//------------------------------------------------------------------------------

const CSmallString CJob::GetItem(const CSmallString& path,
                        const CSmallString& name,bool noerror)
{
    CSmallString value;
    GetItem(path,name,value,noerror);
    return(value);
}

//------------------------------------------------------------------------------

const CSmallString CJob::GetSiteName(void)
{
    CXMLElement* p_rele = GetElementByPath("infinity",false);
    if( p_rele == NULL ){
        ES_ERROR("infinity element was not found");
        return("");
    }

    CSmallString siteid,name;
    p_rele->GetAttribute("site",siteid);

    name = CUtils::GetSiteName(siteid);
    if( name == NULL ){
        name = "-unknown-";
    }

    return(name);
}

//------------------------------------------------------------------------------

const CSmallString CJob::GetSiteID(void)
{
    CXMLElement* p_rele = GetElementByPath("infinity",false);
    if( p_rele == NULL ){
        ES_ERROR("infinity element was not found");
        return("");
    }

    CSmallString siteid;
    p_rele->GetAttribute("site",siteid);

    return(siteid);
}

//------------------------------------------------------------------------------

CSmallString CJob::GetInfoFileVersion(void)
{
    CXMLElement* p_rele = GetElementByPath("infinity",false);
    if( p_rele == NULL ){
        ES_ERROR("infinity element was not found");
        return("");
    }

    CSmallString ver;
    p_rele->GetAttribute("version",ver);

    return(ver);
}

//------------------------------------------------------------------------------

const CSmallString CJob::GetServerName(void)
{
    return(GetItem("specific/resources","INF_SERVER"));
}

//------------------------------------------------------------------------------

const CSmallString CJob::GetShortServerName(void)
{
    return(ShortServerName);
}

//------------------------------------------------------------------------------

const CSmallString CJob::GetServerNameV2(void)
{
    CXMLElement* p_rele = GetElementByPath("infinity",false);
    if( p_rele == NULL ){
        ES_ERROR("infinity element was not found");
        return("");
    }

    CSmallString mode;
    p_rele->GetAttribute("server",mode);
    return(mode);
}

//------------------------------------------------------------------------------

const CSmallTime CJob::GetQueuedTime(void)
{
    CSmallTimeAndDate stad;
    HasSection("submit",stad);

    CSmallTimeAndDate btad;
    HasSection("start",btad);

    CSmallTime qtime;
    qtime = btad - stad;
    return(qtime);
}

//------------------------------------------------------------------------------

const CSmallTime CJob::GetRunningTime(void)
{
    CSmallTimeAndDate btad;
    HasSection("start",btad);

    CSmallTimeAndDate etad;
    HasSection("stop",etad);

    CSmallTime qtime;
    qtime = etad - btad;
    return(qtime);
}

//------------------------------------------------------------------------------

const CSmallTimeAndDate CJob::GetTimeOfLastChange(void)
{
    CSmallTimeAndDate btad;

    switch( GetJobStatus() ){
        case EJS_NONE:
        case EJS_INCONSISTENT:
        case EJS_ERROR:
            // nothing to be here
            break;
        case EJS_PREPARED:
            HasSection("basic",btad);
            break;
        case EJS_SUBMITTED:
            HasSection("submit",btad);
            break;
        case EJS_RUNNING:
            HasSection("start",btad);
            break;
        case EJS_FINISHED:
            HasSection("stop",btad);
            break;
        case EJS_KILLED:
            HasSection("kill",btad);
            break;
    }

    return(btad);
}

//------------------------------------------------------------------------------

int CJob::GetNCPU(void)
{
    CSmallString ncpu = GetItem("specific/resources","INF_NCPU");
    return(ncpu.ToInt());
}

//------------------------------------------------------------------------------

bool CJob::IsJobDirLocal(bool no_deep)
{
    if( GetItem("basic/jobinput","INF_JOB_MACHINE",false) == ABSConfig.GetHostName() ){
        return(true);
    }
    if( no_deep ) return(false); // no deep checking

    // FIXME
    // try to test for job directory on remote machine, test if keys match
    return(false);
}

//------------------------------------------------------------------------------

void CJob::SetSimpleJobIdentification(const CSmallString& name, const CSmallString& machine,
                                      const CSmallString& path)
{
    SetItem("basic/jobinput","INF_JOB_TITLE",name);
    SetItem("basic/jobinput","INF_JOB_NAME",name);
    SetItem("basic/jobinput","INF_JOB_MACHINE",machine);
    SetItem("basic/jobinput","INF_JOB_PATH",path);
}

//------------------------------------------------------------------------------

int CJob::GetNumberOfRecycleJobs(void)
{
    CSmallString start;
    GetItem("basic/recycle","INF_RECYCLE_START",start,true);
    if( start == NULL ) return(1);

    CSmallString stop;
    GetItem("basic/recycle","INF_RECYCLE_STOP",stop,true);

    return( stop.ToInt() - start.ToInt() + 1);
}

//------------------------------------------------------------------------------

int CJob::GetCurrentOfRecycleJob(void)
{
    CSmallString start;
    GetItem("basic/recycle","INF_RECYCLE_START",start,true);
    if( start == NULL ) return(1);

    CSmallString current;
    GetItem("basic/recycle","INF_RECYCLE_CURRENT",current,true);

    return( current.ToInt() - start.ToInt() + 1);
}

//------------------------------------------------------------------------------

bool CJob::IsInfoFileLoaded(void)
{
    return(InfoFileLoaded);
}

//------------------------------------------------------------------------------

void CJob::RestoreEnv(void)
{
    RestoreEnv(GetChildElementByPath("job_info/basic"));
    RestoreEnv(GetChildElementByPath("job_info/specific"));
    RestoreEnv(GetChildElementByPath("job_info/submit"));
}

//------------------------------------------------------------------------------

void CJob::PrepareGoWorkingDirEnv(bool noterm)
{
    CSmallString job_key = CShell::GetSystemVariable("INF_JOB_KEY");
    if( IsInteractiveJob() && job_key == GetJobKey() ) return;

    ShellProcessor.SetVariable("INF_GO_SITE_ID",GetSiteID());
    ShellProcessor.SetVariable("INF_GO_MAIN_NODE",GetItem("start/workdir","INF_MAIN_NODE"));
    ShellProcessor.SetVariable("INF_GO_WORK_DIR",GetItem("start/workdir","INF_WORK_DIR"));
    if( noterm == true ){
        ShellProcessor.SetVariable("INF_GO_JOB_NAME","noterm");
    } else {
        ShellProcessor.SetVariable("INF_GO_JOB_NAME",GetItem("basic/jobinput","INF_JOB_NAME"));
    }
    ShellProcessor.SetVariable("INF_GO_JOB_KEY",GetItem("basic/jobinput","INF_JOB_KEY"));
    ShellProcessor.SetVariable("INF_GO_UGROUP",GetItem("specific/resources","INF_GROUP"));
    ShellProcessor.SetVariable("INF_GO_UMASK",GetItem("specific/resources","INF_UMASK"));
}

//------------------------------------------------------------------------------

void CJob::PrepareGoInputDirEnv(void)
{
    ShellProcessor.SetVariable("INF_GO_MAIN_NODE","");
    ShellProcessor.SetVariable("INF_GO_INPUT_MACHINE",GetItem("basic/jobinput","INF_JOB_MACHINE"));
    ShellProcessor.SetVariable("INF_GO_INPUT_DIR",GetItem("basic/jobinput","INF_JOB_PATH"));
}

//------------------------------------------------------------------------------

void CJob::PrepareSyncWorkingDirEnv(void)
{
    ShellProcessor.SetVariable("INF_SYNC_MAIN_NODE",GetItem("start/workdir","INF_MAIN_NODE"));
    ShellProcessor.SetVariable("INF_SYNC_WORK_DIR",GetItem("start/workdir","INF_WORK_DIR"));
    ShellProcessor.SetVariable("INF_SYNC_JOB_MACHINE",GetItem("basic/jobinput","INF_JOB_MACHINE"));
    ShellProcessor.SetVariable("INF_SYNC_JOB_PATH",GetItem("basic/jobinput","INF_JOB_PATH"));
}

//------------------------------------------------------------------------------

void CJob::PrepareSoftKillEnv(void)
{
    ShellProcessor.SetVariable("INF_KILL_MAIN_NODE",GetItem("start/workdir","INF_MAIN_NODE"));
    ShellProcessor.SetVariable("INF_KILL_WORK_DIR",GetItem("start/workdir","INF_WORK_DIR"));
    ShellProcessor.SetVariable("INF_KILL_JOB_NAME",GetItem("basic/jobinput","INF_JOB_NAME"));
    ShellProcessor.SetVariable("INF_KILL_JOB_KEY",GetItem("basic/jobinput","INF_JOB_KEY"));
}

//------------------------------------------------------------------------------

void CJob::RestoreEnv(CXMLElement* p_ele)
{
    CXMLIterator   I(p_ele);
    CXMLElement*   p_sele;

    while( (p_sele = I.GetNextChildElement()) != NULL ){
        if( p_sele->GetName() == "item" ){
            CSmallString name;
            CSmallString value;
            p_sele->GetAttribute("name",name);
            p_sele->GetAttribute("value",value);
            if( name != NULL ){
                ShellProcessor.SetVariable(name,value);
            }
        } else {
            RestoreEnv(p_sele);
        }
    }
}

//------------------------------------------------------------------------------

const CSmallString CJob::GetLastError(void)
{
    CSmallString tmp;
    tmp = GetItem("error/last","INF_ERROR");
    return(tmp);
}

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

void CJob::PrintJobInfo(std::ostream& sout)
{
    CSmallString ver = GetInfoFileVersion();
    if( ver == "INFINITY_INFO_v_3_0"){
        PrintJobInfoV3(sout);
    }  else if( ver == "INFINITY_INFO_v_2_0"){
        PrintJobInfoV2(sout);
    }  else if( ver == "INFINITY_INFO_v_1_0" ){
        PrintJobInfoV1(sout);
    } else {
        ES_ERROR("unsupported version");
    }
}

//------------------------------------------------------------------------------

void CJob::PrintJobInfoCompact(std::ostream& sout,bool includepath,bool includecomment)
{
    CSmallString ver = GetInfoFileVersion();
    if( ver == "INFINITY_INFO_v_3_0"){
        PrintJobInfoCompactV3(sout,includepath,includecomment);
    }  else    if( ver == "INFINITY_INFO_v_2_0"){
        PrintJobInfoCompactV2(sout,includepath,includecomment);
    }  else if( ver == "INFINITY_INFO_v_1_0" ){
        PrintJobInfoCompactV1(sout,includepath);
    } else {
        ES_ERROR("unsupported version");
    }
}

//------------------------------------------------------------------------------

void CJob::PrintJobInfoForCollection(std::ostream& sout,bool includepath,bool includecomment)
{
//    sout << "# CID  ST    Job Name         Queue       NCPUs NGPUs NNods     Last change       Metric   Comp" << endl;
//    sout << "# ---- -- --------------- --------------- ----- ----- ----- -------------------- --------- ----" << endl;

    switch( GetJobStatus() ){
        case EJS_NONE:
            sout << "UN";
            break;
        case EJS_PREPARED:
            sout << "P ";
            break;
        case EJS_SUBMITTED:
            sout << "Q ";
            break;
        case EJS_RUNNING:
            sout << "R ";
            break;
        case EJS_FINISHED:
            sout << "F ";
            break;
        case EJS_KILLED:
            sout << "K ";
            break;
        case EJS_ERROR:
            sout << "ER";
            break;
        case EJS_INCONSISTENT:
            sout << "IN";
            break;
    }
    CSmallString name = GetItem("basic/jobinput","INF_JOB_NAME");
    if( name.GetLength() > 15 ){
        name = name.GetSubStringFromTo(0,14);
    }
    sout << " " << setw(15) << name;

    if( InfoFileLoaded ){
        sout << right;
        CSmallString queue = GetItem("specific/resources","INF_QUEUE");
        if( queue.GetLength() > 15 ){
            queue = queue.GetSubStringFromTo(0,14);
        }
        sout << " " << setw(15) << queue;
        sout << " " << setw(5) << GetItem("specific/resources","INF_NCPU");
        sout << " " << setw(5) << GetItem("specific/resources","INF_NGPU");
        sout << " " << setw(5) << GetItem("specific/resources","INF_NNODE");

        CSmallTimeAndDate   last_change;
        CSmallTimeAndDate   curr_time;
        CSmallTime          from_last_change;

        curr_time.GetActualTimeAndDate();
        last_change = GetTimeOfLastChange();

        sout << " " ;

        int tot = GetNumberOfRecycleJobs();
        int cur = GetCurrentOfRecycleJob();
        int per = 0;

        switch( GetJobStatus() ){
            case EJS_NONE:
            case EJS_INCONSISTENT:
            case EJS_ERROR:
                per = cur - 1;
                sout << "                    ";
                break;
            case EJS_PREPARED:
            case EJS_SUBMITTED:
            case EJS_RUNNING:
                per = cur - 1;
                from_last_change = curr_time - last_change;
                sout << right << setw(20) << from_last_change.GetSTimeAndDay();
                break;
            case EJS_FINISHED:
                per = cur;
                sout << right << setw(20) << last_change.GetSDateAndTime();
                break;
            case EJS_KILLED:
                per = cur - 1;
                sout << right << setw(20) << last_change.GetSDateAndTime();
                break;
        }
        if( per < 0 ) per = 0;
        if( tot > 0 ) per = 100*per / tot;

        sout << " " << right << setw(4) << cur << "/" << setw(4) << tot;
        sout << " " << setw(3) << per << "%";

    } else {
        sout << " <red>job info file was not found</red>";
    }

    sout << endl;

    if( includecomment ){
        switch( GetJobStatus() ){
            case EJS_NONE:
            case EJS_PREPARED:
            case EJS_FINISHED:
            case EJS_KILLED:
                // nothing to be here
                break;

            case EJS_ERROR:
            case EJS_INCONSISTENT:
            case EJS_SUBMITTED:
                sout << "       <green>" << BatchJobComment << "</green>" << endl;
                break;
            case EJS_RUNNING:
                sout << "       <green>" << GetItem("start/workdir","INF_MAIN_NODE") << "</green>" << endl;
                break;
        }
    }

    if( includepath ){
        if( IsJobDirLocal() ){
            sout << "       <blue>" << GetItem("basic/jobinput","INF_JOB_PATH") << "</blue>" << endl;
        } else {
            sout << "       <blue>" << GetItem("basic/jobinput","INF_JOB_MACHINE") << ":" << GetItem("basic/jobinput","INF_JOB_PATH") << "</blue>" << endl;
        }
    }
}

//------------------------------------------------------------------------------

void CJob::PrintJobInfoCompactV3(std::ostream& sout,bool includepath,bool includecomment)
{
//    sout << "# ST       Job ID            Job Title         Queue      NCPUs NGPUs NNods Last change         " << endl;
//    sout << "# -- -------------------- --------------- --------------- ----- ----- ----- --------------------" << endl;

    sout << "  ";
    switch( GetJobStatus() ){
        case EJS_NONE:
            sout << "UN";
            break;
        case EJS_PREPARED:
            sout << "<yellow>P</yellow> ";
            break;
        case EJS_SUBMITTED:
            sout << "<purple>Q</purple> ";
            break;
        case EJS_RUNNING:
            sout << "<green>R</green> ";
            break;
        case EJS_FINISHED:
            sout << "F ";
            break;
        case EJS_KILLED:
            sout << "<red>KI</red>";
            break;
        case EJS_ERROR:
            sout << "<red>ER</red>";
            break;
        case EJS_INCONSISTENT:
            sout << "<red>IN</red>";
            break;
    }
    if( GetItem("submit/job","INF_JOB_ID",true) != NULL ){
    CSmallString id = GetItem("submit/job","INF_JOB_ID");
    string stmp(id);
    vector<string> items;
    split(items,stmp,is_any_of("."));
    if( items.size() >= 1 ){
        id = items[0];
    }
    if( id.GetLength() > 20 ){
        id = id.GetSubStringFromTo(0,19);
    }
    sout << " " << left << setw(20) << id;
    } else {
    sout << "                     ";
    }
    CSmallString title = GetItem("basic/jobinput","INF_JOB_TITLE");
    sout << " " << setw(15) << title;
    CSmallString queue = GetItem("specific/resources","INF_QUEUE");
    if( queue.GetLength() > 15 ){
        queue = queue.GetSubStringFromTo(0,14);
    }
    sout << " " << setw(15) << left << queue;
    sout << right;
    sout << " " << setw(5) << GetItem("specific/resources","INF_NCPU");
    sout << " " << setw(5) << GetItem("specific/resources","INF_NGPU");
    sout << " " << setw(5) << GetItem("specific/resources","INF_NNODE");

    CSmallTimeAndDate   last_change;
    CSmallTimeAndDate   curr_time;
    CSmallTime          from_last_change;

    curr_time.GetActualTimeAndDate();
    last_change = GetTimeOfLastChange();

    sout << " " ;
    switch( GetJobStatus() ){
        case EJS_NONE:
        case EJS_INCONSISTENT:
        case EJS_ERROR:
            break;
        case EJS_PREPARED:
        case EJS_SUBMITTED:
        case EJS_RUNNING:
            from_last_change = curr_time - last_change;
            sout << right << setw(20) << from_last_change.GetSTimeAndDay();
            break;
        case EJS_FINISHED:
        case EJS_KILLED:
            sout << right << setw(20) << last_change.GetSDateAndTime();
            break;
    }

    sout << endl;

    if( includepath ){
        if( IsJobDirLocal() ){
            sout << "     <blue>> " << GetItem("basic/jobinput","INF_JOB_PATH") << "</blue>" << endl;
        } else {
            sout << "     <blue>> " << GetItem("basic/jobinput","INF_JOB_MACHINE") << ":" << GetItem("basic/jobinput","INF_JOB_PATH") << "</blue>" << endl;
        }
    }

    if( includecomment ){
        switch( GetJobStatus() ){
            case EJS_NONE:
            case EJS_PREPARED:
            case EJS_FINISHED:
            case EJS_KILLED:
                // nothing to be here
                break;

            case EJS_ERROR:
            case EJS_INCONSISTENT:
                sout << "     <red>" << GetJobBatchComment() << "</red>" << endl;
                break;
            case EJS_SUBMITTED:
                sout << "     <purple>" << GetJobBatchComment() << "</purple>" << endl;
                break;
            case EJS_RUNNING:
                sout << "     <green>" << GetItem("start/workdir","INF_MAIN_NODE");
                if( GetItem("specific/resources","INF_NNODE").ToInt() > 1 ){
                    sout << ",+";
                }
                sout << "</green>" << endl;
                break;
        }
    }
}

//------------------------------------------------------------------------------

void CJob::PrintJobInfoCompactV2(std::ostream& sout,bool includepath,bool includecomment)
{
//    sout << "# ST       Job ID            Job Title         Queue      NCPUs NGPUs NNods Last change         " << endl;
//    sout << "# -- -------------------- --------------- --------------- ----- ----- ----- --------------------" << endl;

    sout << "  ";
    switch( GetJobStatus() ){
        case EJS_NONE:
            sout << "UN";
            break;
        case EJS_PREPARED:
            sout << "<yellow>P</yellow> ";
            break;
        case EJS_SUBMITTED:
            sout << "<purple>Q</purple> ";
            break;
        case EJS_RUNNING:
            sout << "<green>R</green> ";
            break;
        case EJS_FINISHED:
            sout << "F ";
            break;
        case EJS_KILLED:
            sout << "<red>KI</red>";
            break;
        case EJS_ERROR:
            sout << "<red>ER</red>";
            break;
        case EJS_INCONSISTENT:
            sout << "<red>IN</red>";
            break;
    }
    if( GetItem("submit/job","INF_JOB_ID",true) != NULL ){
    CSmallString id = GetItem("submit/job","INF_JOB_ID");
    string stmp(id);
    vector<string> items;
    split(items,stmp,is_any_of("."));
    if( items.size() >= 1 ){
        id = items[0];
    }
    if( id.GetLength() > 20 ){
        id = id.GetSubStringFromTo(0,19);
    }
    sout << " " << left << setw(20) << id;
    } else {
    sout << "                     ";
    }
    CSmallString title = GetItem("basic/jobinput","INF_JOB_TITLE");
    sout << " " << setw(15) << title;
    CSmallString queue = GetItem("specific/resources","INF_QUEUE");
    if( queue.GetLength() > 15 ){
        queue = queue.GetSubStringFromTo(0,14);
    }
    sout << " " << setw(15) << left << queue;
    sout << right;
    sout << " " << setw(5) << GetItem("specific/resources","INF_NCPU");
    sout << " " << setw(5) << GetItem("specific/resources","INF_NGPU");
    sout << " " << setw(5) << GetItem("specific/resources","INF_NNODE");

    CSmallTimeAndDate   last_change;
    CSmallTimeAndDate   curr_time;
    CSmallTime          from_last_change;

    curr_time.GetActualTimeAndDate();
    last_change = GetTimeOfLastChange();

    sout << " " ;
    switch( GetJobStatus() ){
        case EJS_NONE:
        case EJS_INCONSISTENT:
        case EJS_ERROR:
            break;
        case EJS_PREPARED:
        case EJS_SUBMITTED:
        case EJS_RUNNING:
            from_last_change = curr_time - last_change;
            sout << right << setw(20) << from_last_change.GetSTimeAndDay();
            break;
        case EJS_FINISHED:
        case EJS_KILLED:
            sout << right << setw(20) << last_change.GetSDateAndTime();
            break;
    }

    sout << endl;

    if( includepath ){
        if( IsJobDirLocal() ){
            sout << "     <blue>> " << GetItem("basic/jobinput","INF_JOB_PATH") << "</blue>" << endl;
        } else {
            sout << "     <blue>> " << GetItem("basic/jobinput","INF_JOB_MACHINE") << ":" << GetItem("basic/jobinput","INF_JOB_PATH") << "</blue>" << endl;
        }
    }

    if( includecomment ){
        switch( GetJobStatus() ){
            case EJS_NONE:
            case EJS_PREPARED:
            case EJS_FINISHED:
            case EJS_KILLED:
                // nothing to be here
                break;

            case EJS_ERROR:
            case EJS_INCONSISTENT:
                sout << "     <red>" << GetJobBatchComment() << "</red>" << endl;
                break;
            case EJS_SUBMITTED:
                sout << "     <purple>" << GetJobBatchComment() << "</purple>" << endl;
                break;
            case EJS_RUNNING:
                sout << "     <green>" << GetItem("start/workdir","INF_MAIN_NODE");
                if( GetItem("specific/resources","INF_NNODE").ToInt() > 1 ){
                    sout << ",+";
                }
                sout << "</green>" << endl;
                break;
        }
    }
}

//------------------------------------------------------------------------------

void CJob::PrintJobInfoCompactV1(std::ostream& sout,bool includepath)
{
//    sout << "# ST       Job ID            Job Title         Queue      NCPUs NGPUs NNods Last change         " << endl;
//    sout << "# -- -------------------- --------------- --------------- ----- ----- ----- --------------------" << endl;

    if( includepath ){
        if( IsJobDirLocal() ){
            sout << "<blue>>" << GetItem("basic/jobinput","INF_JOB_PATH") << "</blue>" << endl;
        } else {
            sout << "<blue>>" << GetItem("basic/jobinput","INF_JOB_MACHINE") << ":" << GetItem("basic/jobinput","INF_JOB_PATH") << "</blue>" << endl;
        }
    }

    sout << "  ";
    switch( GetJobStatus() ){
        case EJS_NONE:
            sout << "UN";
            break;
        case EJS_PREPARED:
            sout << "P ";
            break;
        case EJS_SUBMITTED:
            sout << "Q ";
            break;
        case EJS_RUNNING:
            sout << "R ";
            break;
        case EJS_FINISHED:
            sout << "F ";
            break;
        case EJS_KILLED:
            sout << "KI";
            break;
        case EJS_ERROR:
            sout << "ER";
            break;
        case EJS_INCONSISTENT:
            sout << "IN";
            break;
    }
    if( GetItem("submit/job","INF_JOB_ID",true) != NULL ){
    CSmallString id = GetItem("submit/job","INF_JOB_ID");
    if( id.GetLength() > 20 ){
        id = id.GetSubStringFromTo(0,19);
    }
    sout << " " << left << setw(20) << id;
    } else {
    sout << "                    ";
    }
    sout << " " << setw(15) << GetItem("basic/jobinput","INF_JOB_TITLE");
    sout << right;
    CSmallString queue = GetItem("specific/resources","INF_QUEUE");
    if( queue.GetLength() > 15 ){
        queue = queue.GetSubStringFromTo(0,14);
    }
    sout << " " << setw(15) << queue;
    sout << " " << setw(5) << GetItem("specific/resources","INF_NCPU");
    int ngpus = 0;
    sout << " " << setw(5) << ngpus;
    sout << " " << setw(5) << GetItem("specific/resources","INF_NUM_OF_NODES");

    CSmallTimeAndDate   last_change;
    CSmallTimeAndDate   curr_time;
    CSmallTime          from_last_change;

    curr_time.GetActualTimeAndDate();
    last_change = GetTimeOfLastChange();

    sout << " " << right << setw(20);
    switch( GetJobStatus() ){
        case EJS_NONE:
        case EJS_INCONSISTENT:
        case EJS_ERROR:
            break;
        case EJS_PREPARED:
        case EJS_SUBMITTED:
        case EJS_RUNNING:
            from_last_change = curr_time - last_change;
            sout << from_last_change.GetSTimeAndDay();
            break;
        case EJS_FINISHED:
        case EJS_KILLED:
            sout << last_change.GetSDateAndTime();
            break;
    }

    sout << endl;
}

//------------------------------------------------------------------------------

void CJob::PrintJobInfoV3(std::ostream& sout)
{
    PrintBasicV3(sout);
    PrintResourcesV3(sout);

    if( HasSection("start") == true ){
        PrintExec(sout);
    }

    CSmallTimeAndDate ctad;
    ctad.GetActualTimeAndDate();

    CSmallTimeAndDate stad;
    if( HasSection("submit",stad) == true ){
        sout << "Job was submitted on " << stad.GetSDateAndTime() << endl;
    } else {
        HasSection("basic",stad);
        sout << "Job was prepared for submission on " << stad.GetSDateAndTime() << endl;
        return;
    }

    bool inconsistent = false;

    CSmallTimeAndDate btad;
    if( HasSection("start",btad) == true ){
        CSmallTime qtime;
        qtime = btad - stad;
        sout << "  and was queued for " << qtime.GetSTimeAndDay() << endl;
        sout << "Job was started on " << btad.GetSDateAndTime() << endl;
    } else {
        if( GetJobBatchStatus() == EJS_SUBMITTED ){
            CSmallTime qtime;
            qtime = ctad - stad;
            sout << "<purple>";
            sout << "  and is queued for " << qtime.GetSTimeAndDay() << endl;
            sout << "  >>> Comment: " << GetJobBatchComment() << endl;
            sout << "</purple>";
            return;
        }
        if( GetJobStatus() == EJS_INCONSISTENT ){
            CSmallTime qtime;
            qtime = ctad - stad;
            sout << "  and is queued for " << qtime.GetSTimeAndDay() << endl;
            inconsistent = true;
        }
    }

    CSmallTimeAndDate etad;
    if( HasSection("stop",etad) == true ){
        CSmallTime qtime;
        qtime = etad - btad;
        sout << "  and was running for " << qtime.GetSTimeAndDay() << endl;
        sout << "Job was finished on " << etad.GetSDateAndTime() << endl;
        return;
    } else {
        if( GetJobStatus() == EJS_RUNNING  ){
            CSmallTime qtime;
            qtime = ctad - btad;
            sout << "<green>";
            sout << "  and is running for " << qtime.GetSTimeAndDay() << endl;
            sout << "</green>";
            return;
        }
        if( GetJobBatchStatus() == EJS_RUNNING  ){
            sout << "<purple>Job was started but details are not known yet" << endl;
            sout << "    >>> Comment: " << GetJobBatchComment() << endl;
            sout << "</purple>";
            return;
        }
        if( (GetJobStatus() == EJS_INCONSISTENT) && (inconsistent == false) ){
            CSmallTime qtime;
            qtime = ctad - btad;
            sout << "  and is running for " << qtime.GetSTimeAndDay() << endl;
        }
    }

    CSmallTimeAndDate ktad;
    if( HasSection("kill",ktad) == true ){
        CSmallTime qtime;
        if( HasSection("start") == true ) {
            qtime = ktad - btad;
            sout << "  and was running for " << qtime.GetSTimeAndDay() << endl;
        } else {
            if( HasSection("submit") == true ) {
                qtime = ktad - stad;
                sout << "  and was queued for " << qtime.GetSTimeAndDay() << endl;
            }
        }
        sout << "Job was killed on " << ktad.GetSDateAndTime() << endl;
        return;
    }

    if( GetJobStatus() == EJS_INCONSISTENT ){
        sout << "<red>>>> Job is in inconsistent state!"<< endl;
        sout <<         "    Comment: " << GetJobBatchComment() << "</red>" << endl;
    }

    if( GetJobStatus() == EJS_ERROR){
        sout << "<red>>>> Job is in error state!"<< endl;
        sout <<         "    Comment: " << GetJobBatchComment() << "</red>" << endl;
    }
}

//------------------------------------------------------------------------------

void CJob::PrintJobInfoV2(std::ostream& sout)
{
    PrintBasicV2(sout);
    PrintResourcesV2(sout);

    if( HasSection("start") == true ){
        PrintExec(sout);
    }

    CSmallTimeAndDate ctad;
    ctad.GetActualTimeAndDate();

    CSmallTimeAndDate stad;
    if( HasSection("submit",stad) == true ){
        sout << "Job was submitted on " << stad.GetSDateAndTime() << endl;
    } else {
        HasSection("basic",stad);
        sout << "Job was prepared for submission on " << stad.GetSDateAndTime() << endl;
        return;
    }

    bool inconsistent = false;

    CSmallTimeAndDate btad;
    if( HasSection("start",btad) == true ){
        CSmallTime qtime;
        qtime = btad - stad;
        sout << "  and was queued for " << qtime.GetSTimeAndDay() << endl;
        sout << "Job was started on " << btad.GetSDateAndTime() << endl;
    } else {
        if( GetJobBatchStatus() == EJS_SUBMITTED ){
            CSmallTime qtime;
            qtime = ctad - stad;
            sout << "<purple>";
            sout << "  and is queued for " << qtime.GetSTimeAndDay() << endl;
            sout << "  >>> Comment: " << GetJobBatchComment() << endl;
            sout << "</purple>";
            return;
        }
        if( GetJobStatus() == EJS_INCONSISTENT ){
            CSmallTime qtime;
            qtime = ctad - stad;
            sout << "  and is queued for " << qtime.GetSTimeAndDay() << endl;
            inconsistent = true;
        }
    }

    CSmallTimeAndDate etad;
    if( HasSection("stop",etad) == true ){
        CSmallTime qtime;
        qtime = etad - btad;
        sout << "  and was running for " << qtime.GetSTimeAndDay() << endl;
        sout << "Job was finished on " << etad.GetSDateAndTime() << endl;
        return;
    } else {
        if( GetJobStatus() == EJS_RUNNING  ){
            CSmallTime qtime;
            qtime = ctad - btad;
            sout << "<green>";
            sout << "  and is running for " << qtime.GetSTimeAndDay() << endl;
            sout << "</green>";
            return;
        }
        if( GetJobBatchStatus() == EJS_RUNNING  ){
            sout << "<purple>Job was started but details are not known yet" << endl;
            sout << "    >>> Comment: " << GetJobBatchComment() << endl;
            sout << "</purple>";
            return;
        }
        if( (GetJobStatus() == EJS_INCONSISTENT) && (inconsistent == false) ){
            CSmallTime qtime;
            qtime = ctad - btad;
            sout << "  and is running for " << qtime.GetSTimeAndDay() << endl;
        }
    }

    CSmallTimeAndDate ktad;
    if( HasSection("kill",ktad) == true ){
        CSmallTime qtime;
        if( HasSection("start") == true ) {
            qtime = ktad - btad;
            sout << "  and was running for " << qtime.GetSTimeAndDay() << endl;
        } else {
            if( HasSection("submit") == true ) {
                qtime = ktad - stad;
                sout << "  and was queued for " << qtime.GetSTimeAndDay() << endl;
            }
        }
        sout << "Job was killed on " << ktad.GetSDateAndTime() << endl;
        return;
    }

    if( GetJobStatus() == EJS_INCONSISTENT ){
        sout << "<red>>>> Job is in inconsistent state!"<< endl;
        sout <<         "    Comment: " << GetJobBatchComment() << "</red>" << endl;
    }

    if( GetJobStatus() == EJS_ERROR){
        sout << "<red>>>> Job is in error state!"<< endl;
        sout <<         "    Comment: " << GetJobBatchComment() << "</red>" << endl;
    }
}

//------------------------------------------------------------------------------

void CJob::PrintJobStatus(std::ostream& sout)
{
    CSmallTimeAndDate stad;

    // terminal/initial states -------------------
    if( HasSection("kill",stad) == true ){
        sout << "K";
        return;
    }

    if( HasSection("stop",stad) == true ){
        sout << "F";
        return;
    }

    if( HasSection("submit",stad) == false ){
        sout << "P";
        return;
    }

    // intermediate states -----------------------
    if( HasSection("start",stad) == false ){
        if( GetJobBatchStatus() == EJS_SUBMITTED ){
            sout << "Q";
            return;
        }
        if( GetJobBatchStatus() == EJS_RUNNING ){
            sout << "R";
            return;
        }
    } else {
        if( GetJobBatchStatus() == EJS_RUNNING ){
            sout << "R";
            return;
        }
    }

    // the rest is in inconsistent state
    sout << "IN";
    return;
}

//------------------------------------------------------------------------------

void CJob::PrintJobInfoV1(std::ostream& sout)
{
    CSmallString tmp,col;

    sout << "Job name         : " << GetItem("basic/jobinput","INF_JOB_NAME") << endl;
    if( GetItem("submit/job","INF_JOB_ID",true) != NULL ){
    sout << "Job ID           : " << GetItem("submit/job","INF_JOB_ID") << endl;
    }
    sout << "Job title        : " << GetItem("basic/jobinput","INF_JOB_TITLE") << " (Job type: ";
    sout << GetItem("basic/jobinput","INF_JOB_TYPE") << ")" << endl;

    sout << "Job directory    : " << GetItem("basic/jobinput","INF_JOB_MACHINE");
    sout << ":" << GetItem("basic/jobinput","INF_JOB_PATH") << endl;

    col = GetItem("basic/collection","INF_COLLECTION_NAME",true);
    if( col == NULL ) col = "-none-";
    tmp = GetItem("basic/jobinput","INF_JOB_PROJECT");
    if( tmp == NULL ) tmp = "-none-";
    sout << "Job project      : " << tmp << " (Collection: " << col << ")" << endl;

    sout << "Site ID          : " << GetSiteID() << endl;

    sout << "========================================================" << endl;

    tmp = GetItem("specific/resources","INF_ALIAS");
    if( tmp == NULL ){
    sout << "Alias            : -none-" << endl;
    } else {
    sout << "Alias            : " << tmp << endl;
    }

    tmp = GetItem("specific/resources","INF_QUEUE");
    sout << "Queue            : " << tmp << endl;

    sout << "----------------------------------------" << endl;

    tmp = GetItem("specific/resources","INF_NCPU");
    sout << "Number of CPUs   : " << tmp << endl;

    tmp = GetItem("specific/resources","INF_MAX_CPUS_PER_NODE");
    sout << "Max CPUs / node  : " << tmp << endl;

    tmp = GetItem("specific/resources","INF_NUM_OF_NODES");
    if( tmp != NULL ){
    sout << "Number of nodes  : " << tmp << endl;
    }
    tmp = GetItem("specific/resources","INF_PROPERTIES");
    sout << "Properties       : " << tmp << endl;

    tmp = GetItem("specific/resources","INF_RESOURCES");
    sout << "Resources        : " << tmp << endl;

    tmp = GetItem("specific/resources","INF_SYNCMODE");
    sout << "Sync mode        : " << tmp << endl;
    sout << "----------------------------------------" << endl;

    tmp = GetItem("basic/external","INF_EXTERNAL_START_AFTER");
    if( tmp == NULL ){
    sout << "Start after      : -not defined-" << endl;
    }
    else{
    sout << "Start after      : " << tmp << endl;
    }

    tmp = GetItem("basic/modules","INF_EXPORTED_MODULES");
    if( tmp == NULL ){
    sout << "Exported modules : -none-" << endl;
    }
    else{
    sout << "Exported modules : " << tmp << endl;
    }

    tmp = GetItem("basic/jobinput","INF_EXCLUDED_FILES");
    if( tmp == NULL ){
    sout << "Excluded files   : -none-" << endl;
    }
    else{
    sout << "Excluded files   : " << tmp << endl;
    }

    sout << "========================================================" << endl;

    if( HasSection("start") == true ){
        CSmallString tmp;

        tmp = GetItem("start/workdir","INF_MAIN_NODE");
        sout << "Main node        : " << tmp << endl;
        tmp = GetItem("start/workdir","INF_WORK_DIR");
        sout << "Working directory: " << tmp << endl;
        sout << "----------------------------------------" << endl;

        ListNodes(sout);

        sout << "========================================================" << endl;
    }

    CSmallTimeAndDate ctad;
    ctad.GetActualTimeAndDate();

    CSmallTimeAndDate stad;
    if( HasSection("submit",stad) == true ){
        sout << "Job was submitted on " << stad.GetSDateAndTime() << endl;
    } else {
        HasSection("basic",stad);
        sout << "Job was prepared for submission on " << stad.GetSDateAndTime() << endl;
        return;
    }

    CSmallTimeAndDate btad;
    if( HasSection("start",btad) == true ){
        CSmallTime qtime;
        qtime = btad - stad;
        sout << "  and was queued for " << qtime.GetSTimeAndDay() << endl;
        sout << "Job was started on " << btad.GetSDateAndTime() << endl;
    }

    CSmallTimeAndDate etad;
    if( HasSection("stop",etad) == true ){
        CSmallTime qtime;
        qtime = etad - btad;
        sout << "  and was running for " << qtime.GetSTimeAndDay() << endl;
        sout << "Job was finished on " << etad.GetSDateAndTime() << endl;
        return;
    }

    sout << "<b><blue>  >>> This is an old style info file." << endl;
    sout <<          "      The current job status from the batch system was not determined.</blue></b>"<< endl;
}

//------------------------------------------------------------------------------

void CJob::PrintBasicV3(std::ostream& sout)
{
    CSmallString tmp,col;

    sout << "Job name         : " << GetItem("basic/jobinput","INF_JOB_NAME") << endl;
    if( GetItem("submit/job","INF_JOB_ID",true) != NULL ){
    sout << "Job ID           : " << GetItem("submit/job","INF_JOB_ID") << endl;
    }
    sout << "Job title        : " << GetItem("basic/jobinput","INF_JOB_TITLE") << " (Job type: ";
    sout << GetItem("basic/jobinput","INF_JOB_TYPE") << ")" << endl;

    sout << "Job directory    : " << GetItem("basic/jobinput","INF_JOB_MACHINE");
    sout << ":" << GetItem("basic/jobinput","INF_JOB_PATH") << endl;

    sout << "Job key          : " << GetItem("basic/jobinput","INF_JOB_KEY") << endl;

    tmp = GetItem("basic/arguments","INF_OUTPUT_SUFFIX",true);
    if( tmp != NULL ) {
    sout << "Parametric job   : " << tmp << endl;
    }

    col = GetItem("basic/collection","INF_COLLECTION_NAME",true);
    if( col == NULL ) col = "-none-";
    tmp = GetItem("basic/jobinput","INF_JOB_PROJECT");
    if( tmp == NULL ) tmp = "-none-";
    sout << "Job project      : " << tmp << " (Collection: " << col << ")" << endl;

    sout << "========================================================" << endl;
}

//------------------------------------------------------------------------------

void CJob::PrintBasicV2(std::ostream& sout)
{
    CSmallString tmp,col;

    sout << "Job name         : " << GetItem("basic/jobinput","INF_JOB_NAME") << endl;
    if( GetItem("submit/job","INF_JOB_ID",true) != NULL ){
    sout << "Job ID           : " << GetItem("submit/job","INF_JOB_ID") << endl;
    }
    sout << "Job title        : " << GetItem("basic/jobinput","INF_JOB_TITLE") << " (Job type: ";
    sout << GetItem("basic/jobinput","INF_JOB_TYPE") << ")" << endl;

    sout << "Job directory    : " << GetItem("basic/jobinput","INF_JOB_MACHINE");
    sout << ":" << GetItem("basic/jobinput","INF_JOB_PATH") << endl;

    col = GetItem("basic/collection","INF_COLLECTION_NAME",true);
    if( col == NULL ) col = "-none-";
    tmp = GetItem("basic/jobinput","INF_JOB_PROJECT");
    if( tmp == NULL ) tmp = "-none-";
    sout << "Job project      : " << tmp << " (Collection: " << col << ")" << endl;

    sout << "Site name        : " << GetSiteName() << " (Torque server: " << GetServerName() << ")" << endl;
    sout << "Job key          : " << GetItem("basic/jobinput","INF_JOB_KEY") << endl;

    tmp = GetItem("basic/arguments","INF_OUTPUT_SUFFIX",true);
    if( tmp != NULL ) {
    sout << "Parametric job   : " << tmp << endl;
    }

    sout << "========================================================" << endl;
}

//------------------------------------------------------------------------------

void CJob::PrintResourcesV3(std::ostream& sout)
{
    CSmallString tmp,tmp1,tmp2;

    tmp = GetItem("basic/arguments","INF_ARG_DESTINATION");
    sout << "Req destination  : " << tmp << endl;

    tmp = GetItem("basic/arguments","INF_ARG_RESOURCES");
    sout << "Req resources    : " << tmp << endl;

    sout << "--------------------------------------------" << endl;

    sout << "Site name        : " << GetSiteName() << " (Batch server: " << GetServerName() << ")" << endl;

    tmp = GetItem("specific/resources","INF_DEFAULT_RESOURCES");
    if( tmp ){
        PrintResourceTokens(sout,"Default resources: ",tmp);
    }

    tmp = GetItem("specific/resources","INF_ALIAS");
    if( tmp == NULL ){
    sout << "Alias            : -none-" << endl;
    }
    else{
    sout << "Alias            : " << tmp << endl;    
    tmp = GetItem("specific/resources","INF_ALIAS_RESOURCES");
    PrintResourceTokens(sout,"Alias resources  : ",tmp);
    }

    tmp = GetItem("specific/resources","INF_RESOURCES");
    PrintResourceTokens(sout,"All resources    : ",tmp);

    tmp = GetItem("specific/resources","INF_QUEUE");
    sout << "Queue            : " << tmp << endl;

    sout << "--------------------------------------------" << endl;
    sout << "NCPUs NGPUs NNodes Memory WorkSize  WallTime" << endl;
    tmp = GetItem("specific/resources","INF_NCPU");
    sout << setw(5) << tmp;
    sout << " ";
    tmp = GetItem("specific/resources","INF_NGPU");
    sout << setw(5) << tmp;
    sout << " ";
    tmp = GetItem("specific/resources","INF_NNODE");
    sout << setw(6) << tmp;
    sout << " ";
    tmp = GetItem("specific/resources","INF_MEMORY");
    sout << setw(6) << tmp;
    sout << " ";
    tmp = GetItem("specific/resources","INF_WORK_SIZE");
    sout << setw(8) << tmp;
    sout << " ";
    tmp = GetItem("specific/resources","INF_WALLTIME");
    sout << setw(9) << tmp;
    sout << endl;

    sout << "--------------------------------------------" << endl;
    tmp = GetItem("basic/jobinput","INF_JOB_PATH");
    sout << "Input directory  : " << tmp << endl;

    tmp = GetItem("specific/resources","INF_INPUT_PATH_FSTYPE");
    sout << "File system type : " << tmp << endl;

    tmp = GetItem("specific/resources","INF_INPUT_MACHINE_GROUPNS");
    sout << "Input data storag: " << GetItem("specific/resources","INF_STORAGE_MACHINE") << ":" << GetItem("specific/resources","INF_STORAGE_PATH") << endl;

    sout << "--------------------------------------------" << endl;

    tmp = GetItem("specific/resources","INF_WORK_DIR_TYPE");
    sout << "Work directory   : " << tmp << endl;

    tmp = GetItem("specific/resources","INF_WORK_SIZE");
    sout << "Work dir size    : " << tmp << endl;

    tmp1 = GetItem("specific/resources","INF_DATAIN");
    tmp2 = GetItem("specific/resources","INF_DATAIN");
    sout << "Data IN/OUT      : " << tmp1 << "/" << tmp2 << endl;

    sout << "--------------------------------------------" << endl;


    CSmallString fs_type = GetItem("specific/resources","INF_FS_TYPE",true);
    if( fs_type == NULL ) fs_type = "consistent";
    sout << "Input FS type    : " << fs_type << endl;

// ----------------------

    if( fs_type == "inconsistent" ) sout << "<blue>";

    tmp = GetItem("specific/resources","INF_UGROUP");
    sout << "User group       : " <<  tmp << endl;

    tmp = GetItem("specific/resources","INF_UMASK");
    sout << "User file mask   : " << tmp << " [" << CUser::GetUMaskPermissions(CUser::GetUMaskMode(tmp)) << "]" <<  endl;

    if( fs_type == "inconsistent" ) sout << "</blue>";

// ----------------------

    tmp = GetItem("basic/jobinput","INF_EXCLUDED_FILES");
    if( tmp == NULL ){
    sout << "Excluded files   : -none-" << endl;
    }
    else{
    sout << "Excluded files   : " << tmp << endl;
    }

    sout << "-------------------------------------------" << endl;

    tmp = GetItem("basic/external","INF_EXTERNAL_START_AFTER");
    if( tmp == NULL ){
    sout << "Start after      : -not defined-" << endl;
    }
    else{
    sout << "Start after      : " << tmp << endl;
    }

    tmp = GetItem("basic/modules","INF_EXPORTED_MODULES");
    if( tmp == NULL ){
    sout << "Exported modules : -none-" << endl;
    }
    else{
    sout << "Exported modules : " << tmp << endl;
    }

    sout << "========================================================" << endl;
}

// -----------------------------------------------------------------------------

void CJob::PrintResourcesV2(std::ostream& sout)
{
    CSmallString tmp,tmp1,tmp2;

    tmp = GetItem("basic/arguments","INF_ARG_DESTINATION");
    sout << "Req destination  : " << tmp << endl;

    tmp = GetItem("basic/arguments","INF_ARG_RESOURCES");
    PrintResourceTokens(sout,"Req resources    :",tmp);

    tmp = GetItem("basic/arguments","INF_ARG_SYNC_MODE");
    if( tmp == NULL ){
    sout << "Req sync mode    : -none-" << endl;
    }
    else{
    sout << "Req sync mode    : " << tmp << endl;
    }

    sout << "----------------------------------------" << endl;
    tmp = GetItem("specific/resources","INF_DEFAULT_RESOURCES");
    if( tmp != NULL ) {
        PrintResourceTokens(sout,"Default resources: ",tmp);
    }

    tmp = GetItem("specific/resources","INF_ALIAS");
    if( tmp == NULL ){
    sout << "Alias            : -none-" << endl;
    }
    else{
    sout << "Alias            : " << tmp << endl;
    tmp = GetItem("specific/resources","INF_ALIAS_RESOURCES");
    PrintResourceTokens(sout,"Alias resources  : ",tmp);

    tmp = GetItem("specific/resources","INF_ALIAS_SYNC_MODE");
    sout << "Alias sync mode  : " << tmp << endl;
    }

    tmp = GetItem("specific/resources","INF_QUEUE");
    sout << "Queue            : " << tmp << endl;

    tmp = GetItem("specific/resources","INF_RESOURCES");
    PrintResourceTokens(sout,"All resources    : ",tmp);

    sout << "----------------------------------------" << endl;

    tmp = GetItem("specific/resources","INF_NCPU");
    sout << "Number of CPUs   : " << tmp << endl;

    tmp = GetItem("specific/resources","INF_NGPU");
    sout << "Number of GPUs   : " << tmp << endl;

    tmp = GetItem("specific/resources","INF_MAX_CPUS_PER_NODE");
    sout << "Max CPUs / node  : " << tmp << endl;

    tmp = GetItem("specific/resources","INF_NNODE");
    if( tmp != NULL ){
    sout << "Number of nodes  : " << tmp << endl;
    }

    sout << "----------------------------------------" << endl;

    tmp = GetItem("specific/resources","INF_SURROGATE_MACHINE",true);
    if( tmp == NULL ) tmp = GetItem("basic/jobinput","INF_JOB_MACHINE");
    if( tmp == GetItem("basic/jobinput","INF_JOB_MACHINE") ){
    sout << "Surrogate machine: " << " -- none -- " << endl;
    } else {
    sout << "Surrogate machine: " << tmp << endl;
    }

    tmp = GetItem("specific/resources","INF_SYNC_MODE");
    sout << "Sync mode        : " << tmp << endl;

    tmp = GetItem("specific/resources","INF_SCRATCH_TYPE",true);
    if( tmp == NULL ) tmp = "scratch";
    sout << "Scratch type     : " << tmp << endl;

    CSmallString fs_type = GetItem("specific/resources","INF_FS_TYPE",true);
    if( fs_type == NULL ) fs_type = "consistent";
    sout << "Input FS type    : " << fs_type << endl;

    if( fs_type == "inconsistent" ) sout << "<blue>";

    tmp = GetItem("specific/resources","INF_UGROUP",true);
    tmp1 = GetItem("specific/resources","INF_UGROUP_REALM",true);
    tmp2 = GetItem("specific/resources","INF_UGROUP_ORIG",true);
    if( tmp == NULL ) tmp = "-not specified-";
    sout << "User group       : ";
    if( tmp2 != NULL ){
        sout << tmp2 << " -> ";
    }
    sout << tmp;
    if( tmp1 != NULL ){
        sout << "@" << tmp1 << endl;
    } else {
        sout << endl;
    }

    tmp = GetItem("specific/resources","INF_UMASK",true);
    if( tmp == NULL ){
        tmp = "-not specified-";
    sout << "User file mask   : " << tmp << endl;
    } else {
    sout << "User file mask   : " << tmp << " [" << CUser::GetUMaskPermissions(CUser::GetUMaskMode(tmp)) << "]" << endl;
    }

    if( fs_type == "inconsistent" ) sout << "</blue>";

    tmp = GetItem("basic/jobinput","INF_EXCLUDED_FILES");
    if( tmp == NULL ){
    sout << "Excluded files   : -none-" << endl;
    }
    else{
    sout << "Excluded files   : " << tmp << endl;
    }

    sout << "----------------------------------------" << endl;

    tmp = GetItem("basic/external","INF_EXTERNAL_START_AFTER");
    if( tmp == NULL ){
    sout << "Start after      : -not defined-" << endl;
    }
    else{
    sout << "Start after      : " << tmp << endl;
    }

    tmp = GetItem("basic/modules","INF_EXPORTED_MODULES");
    if( tmp == NULL ){
    sout << "Exported modules : -none-" << endl;
    }
    else{
    sout << "Exported modules : " << tmp << endl;
    }

    sout << "========================================================" << endl;
}

//------------------------------------------------------------------------------

void CJob::PrintResourceTokens(std::ostream& sout,const CSmallString& title, const CSmallString& res_list)
{
    string          svalue = string(res_list);
    vector<string>  items;
    int             nrow, ncolumns = 80;
    CTerminal::GetSize(nrow,ncolumns);

    // split to items
    split(items,svalue,is_any_of(","));

    vector<string>::iterator it = items.begin();
    vector<string>::iterator ie = items.end();

    sout << title;

    if(res_list == NULL ){
        sout << "-none-" << endl;
        return;
    }

    int len = title.GetLength();

    while( it != ie ){
        string sres = *it;
        sout << sres;
        len += sres.size();
        len++;
        it++;
        if( it != ie ){
            string sres = *it;
            int tlen = len;
            tlen += sres.size();
            tlen++;
            if( tlen > ncolumns ){
                sout << "," << endl;
                for(unsigned int i=0; i < title.GetLength(); i++){
                    sout << " ";
                }
                len = title.GetLength();
            } else {
                sout << ", ";
                len += 2;
            }
        }
    }
    sout << endl;
}

//------------------------------------------------------------------------------

bool CJob::PrintExec(std::ostream& sout)
{
    CSmallString tmp;

    tmp = GetItem("start/workdir","INF_MAIN_NODE");
    sout << "Main node        : " << tmp << endl;
    tmp = GetItem("start/workdir","INF_WORK_DIR");
    sout << "Working directory: " << tmp << endl;
    tmp = GetItem("stop/result","INF_JOB_EXIT_CODE",true);
    if( tmp != NULL ){
    sout << "Job exit code    : " << tmp << endl;
    }
    sout << "----------------------------------------" << endl;

    ListNodes(sout);

    if( (GetItem("specific/resources","INF_NGPU").ToInt() > 0) &&
        (GetElementByPath("start/gpus",false) != NULL) ){
        sout << "----------------------------------------" << endl;

        ListGPUNodes(sout);
    }

    sout << "========================================================" << endl;

    return(true);
}

//------------------------------------------------------------------------------

struct SNode {
    std::string     name;
    int             id;
    CXMLElement*    ijob;
};

//----------------------------------------------

struct SNodeGroup {
    std::string     name;
    int             from;
    int             to;
    CXMLElement*    ijob;
};

//------------------------------------------------------------------------------
bool CJob::ListNodes(std::ostream& sout)
{
    CXMLElement* p_rele = GetElementByPath("start/nodes",false);
    if( p_rele == NULL ){
        ES_ERROR("start/nodes element was not found");
        return(false);
    }

    CXMLElement* p_iele = GetElementByPath("specific/ijobs",false);
    CXMLIterator    I(p_rele);
    CXMLElement*    p_sele;
    int             id = 1;

    list<SNode> nodes;

    // generate list of nodes
    if( p_iele == NULL ){
        while( (p_sele = I.GetNextChildElement("node")) != NULL ){
            SNode node;
            string name;
            p_sele->GetAttribute("name",name);
            node.name = name;
            node.id = id;
            node.ijob = NULL;
            nodes.push_back(node);
            id++;
        }
    } else {
        CXMLIterator    J(p_iele);
        CXMLElement*    p_jele;

        while( (p_jele = J.GetNextChildElement("ijob")) != NULL ){
            int ncpus = 0;
            p_jele->GetAttribute("ncpus",ncpus);
            for(int i=0; i < ncpus; i++){
                p_sele = I.GetNextChildElement("node");
                if( p_sele == NULL ){
                    ES_ERROR("inconsistency between nodes and ijobs");
                    return(false);
                }
                SNode node;
                string name;
                p_sele->GetAttribute("name",name);
                node.name = name;
                node.id = id;
                node.ijob = p_jele;
                nodes.push_back(node);
                id++;
            }
        }
    }

    // generate node groups
    list<SNode>::iterator it = nodes.begin();
    list<SNode>::iterator ie = nodes.end();

    string          prev_node;
    CXMLElement*    prev_ijob = NULL;
    int             from = 0;
    int             to = 0;

    list<SNodeGroup> groups;

    while( it != ie ){
        SNode node = *it;
        if( it == nodes.begin() ){
            prev_node = node.name;
            prev_ijob = node.ijob;
            from = node.id;
        }
        it++;
        if( (node.name != prev_node) || ((node.ijob != prev_ijob)) || (it == nodes.end()) ){
            if( it == nodes.end() ){
                to = node.id;
            }
            SNodeGroup group;
            group.name = prev_node;
            group.from = from;
            group.to = to;
            group.ijob = prev_ijob;
            groups.push_back(group);

            prev_node = node.name;
            prev_ijob = node.ijob;
            from = node.id;
        }
        to = node.id;
    }

    list<SNodeGroup>::iterator git = groups.begin();
    list<SNodeGroup>::iterator gie = groups.end();

    // group ijobs
    prev_ijob = NULL;
    while( git != gie ){
        if( (*git).ijob == prev_ijob ){
            (*git).ijob = NULL;
        } else {
            prev_ijob = (*git).ijob;
        }
        git++;
    }

    // print
    git = groups.begin();

    while( git != gie ){
        SNodeGroup group = *git;
        string ipath;
        string workdir;
        string mainnode;

        if( group.ijob != NULL ){
            group.ijob->GetAttribute("path",ipath);
            group.ijob->GetAttribute("workdir",workdir);
            group.ijob->GetAttribute("mainnode",mainnode);
        }
        if( ! ipath.empty() ){
            sout << left << "<blue>>>> ";
            sout << setw(12) << ipath << " ";
            if( ! mainnode.empty() ){
                sout << mainnode << ":" << workdir;
            }
            sout << "</blue>" << right << endl;
        }

        sout << "CPU " << setfill('0') << setw(4) << group.from;
        if( group.from == group.to ) {
            sout << setfill(' ') << "         : ";
        } else {
            sout << "-" << setw(4) << group.to << setfill(' ') << "    : ";
        }
        stringstream str;
        str << group.name;
        if( (group.to - group.from) > 0 ){
            str << " (" << group.to - group.from + 1 << ")";
        }
        sout << setw(25) << left << str.str() << right << endl;
        git++;
    }

    return(true);
}

//------------------------------------------------------------------------------

bool CJob::ListGPUNodes(std::ostream& sout)
{
    CXMLElement* p_rele = GetElementByPath("start/gpus",false);
    if( p_rele == NULL ){
        ES_ERROR("start/gpus element was not found");
        return(false);
    }

    CXMLIterator    I(p_rele);
    CXMLElement*    p_sele;

    sout << setfill('0');

    int index = 1;
    while( (p_sele = I.GetNextChildElement("gpu")) != NULL ){
        CSmallString nodename;
        p_sele->GetAttribute("name",nodename);
        sout << "GPU " << setw(3) << index << "          : " << nodename << endl;
        index++;
    }

    sout << setfill(' ');

    return(true);
}

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

const CFileName CJob::GetFullJobName(void)
{
    CSmallString name = GetItem("basic/jobinput","INF_JOB_NAME");
    CSmallString suffix = GetItem("basic/external","INF_EXTERNAL_NAME_SUFFIX");

    if( suffix != NULL ){
        return(name + suffix);
    } else {
        return(name);
    }
}

//------------------------------------------------------------------------------

const CFileName CJob::GetMainScriptName(void)
{
    CFileName path = GetItem("basic/jobinput","INF_JOB_PATH");
    CFileName job_script;
    job_script =  path / GetFullJobName() + ".infex";
    return(job_script);
}

//------------------------------------------------------------------------------

const CSmallString CJob::GetExternalVariables(void)
{
    CSmallString rv = GetItem("basic/external","INF_EXTERNAL_VARIABLES");
    return(rv);
}

//------------------------------------------------------------------------------

const CSmallString CJob::GetJobTitle(void)
{
    CSmallString rv = GetItem("basic/jobinput","INF_JOB_TITLE",true);
    if( rv == NULL ){
        rv = GetItem("batch/job","INF_JOB_TITLE");
    }
    return(rv);
}

//------------------------------------------------------------------------------

const CSmallString CJob::GetJobName(void)
{
    CSmallString rv = GetItem("basic/jobinput","INF_JOB_NAME");
    return(rv);
}

//------------------------------------------------------------------------------

const CSmallString CJob::GetJobNameSuffix(void)
{
    CSmallString rv = GetItem("basic/external","INF_EXTERNAL_NAME_SUFFIX");
    return(rv);
}

//------------------------------------------------------------------------------

const CSmallString CJob::GetJobPath(void)
{
    CSmallString rv = GetItem("basic/jobinput","INF_JOB_PATH");
    return(rv);
}

//------------------------------------------------------------------------------

const CSmallString CJob::GetJobMachine(void)
{
    CSmallString rv = GetItem("basic/jobinput","INF_JOB_MACHINE");
    return(rv);
}

//------------------------------------------------------------------------------

const CSmallString CJob::GetStoragePath(void)
{
    CSmallString rv = GetItem("basic/jobinput","INF_STORAGE_MACHINE");
    return(rv);
}

//------------------------------------------------------------------------------

const CSmallString CJob::GetStorageMachine(void)
{
    CSmallString rv = GetItem("basic/jobinput","INF_STORAGE_MACHINE");
    return(rv);
}

//------------------------------------------------------------------------------

const CSmallString CJob::GetJobID(void)
{
    CSmallString rv = GetItem("submit/job","INF_JOB_ID",true);
    if( rv == NULL ){
        rv = GetItem("batch/job","INF_JOB_ID",true);
    }
    return(rv);
}

//------------------------------------------------------------------------------

const CSmallString CJob::GetJobKey(void)
{
    CSmallString rv = GetItem("basic/jobinput","INF_JOB_KEY");
    return(rv);
}

//------------------------------------------------------------------------------

const CSmallString CJob::GetWorkDir(void)
{
    CSmallString rv = GetItem("basic/jobinput","INF_WORK_DIR_TYPE");
    return(rv);
}

//------------------------------------------------------------------------------

const CSmallString CJob::GetDataOut(void)
{
    CSmallString rv = GetItem("basic/jobinput","INF_DATAOUT");
    return(rv);
}

//------------------------------------------------------------------------------

const CFileName CJob::GetInfoutName(void)
{
    CFileName host;
    host = GetItem("specific/resources","INF_SURROGATE_MACHINE",true);
    if( host == NULL ) {
        host = GetItem("basic/jobinput","INF_JOB_MACHINE");
    }
    CFileName path = GetItem("basic/jobinput","INF_JOB_PATH");
    CFileName infout;
    infout = host + ":" + path / GetFullJobName() + ".infout";
    return(infout);
}

//------------------------------------------------------------------------------

const CSmallString CJob::GetQueue(void)
{
    CSmallString queue = GetItem("specific/resources","INF_QUEUE");
    return(queue);
}

//------------------------------------------------------------------------------

int CJob::GetNumOfCPUs(void)
{
    CSmallString tmp;
    tmp = GetItem("specific/resources","INF_NCPU");
    return(tmp.ToInt());
}

//------------------------------------------------------------------------------

int CJob::GetNumOfGPUs(void)
{
    CSmallString tmp;
    tmp = GetItem("specific/resources","INF_NGPU");
    return(tmp.ToInt());
}

//------------------------------------------------------------------------------

int CJob::GetNumOfNodes(void)
{
    CSmallString tmp;
    tmp = GetItem("specific/resources","INF_NNODE");
    return(tmp.ToInt());
}

//------------------------------------------------------------------------------

EJobStatus CJob::GetJobInfoStatus(void)
{
    // order is important!!!

    if( HasSection("kill") == true ){
        return(EJS_KILLED);
    }
    if( HasSection("error") == true ){
        return(EJS_ERROR);
    }
    if( HasSection("stop") == true ){
        return(EJS_FINISHED);
    }
    if( HasSection("start") == true ){
        return(EJS_RUNNING);
    }
    if( HasSection("submit") == true ){
        return(EJS_SUBMITTED);
    }
    if( HasSection("basic") == true ){
        return(EJS_PREPARED);
    }
    return(EJS_NONE);
}

//------------------------------------------------------------------------------

EJobStatus CJob::GetJobBatchStatus(void)
{
    return(BatchJobStatus);
}

//------------------------------------------------------------------------------

const CSmallString CJob::GetJobBatchStatusString(void)
{
    return(GetItem("batch/job","INF_JOB_STATE",true));
}

//------------------------------------------------------------------------------

EJobStatus CJob::GetJobStatus(void)
{
    if( InfoFileLoaded == false ){
        BatchJobComment = "the job info file was not found in the job directory";
        return(EJS_INCONSISTENT);
    }

    if( GetSiteID() != AMSGlobalConfig.GetActiveSiteID() ){
        switch( GetJobInfoStatus() ){
            case EJS_SUBMITTED:
                BatchJobComment = "the job was submitted to batch system of different site";
                return(EJS_INCONSISTENT);
            case EJS_RUNNING:
                BatchJobComment = "the job was submitted to batch system of different site";
                return(EJS_INCONSISTENT);
            default:
                return( GetJobInfoStatus() );
        }
    }

    if( GetInfoFileVersion() == "INFINITY_INFO_v_1_0" ){
        return(GetJobInfoStatus());
    }

    switch( GetJobInfoStatus() ){
        case EJS_SUBMITTED:
            if( BatchJobStatus == EJS_SUBMITTED ){
                return(EJS_SUBMITTED);
            }
            if( BatchJobStatus == EJS_RUNNING ){
                BatchJobComment = "the job is in submitted mode but the batch system shows it in running mode";
                return(EJS_INCONSISTENT);
            }
            if( BatchJobStatus == EJS_FINISHED ){
                BatchJobComment = "the job is in submitted mode but the batch system shows it finished";
                return(EJS_ERROR);
            }
            BatchJobComment = "the job was not found in the batch system";
            return(EJS_ERROR);
        case EJS_RUNNING:
            if( BatchJobStatus == EJS_RUNNING ){
                return(EJS_RUNNING);
            }
            if( BatchJobStatus == EJS_FINISHED ){
                BatchJobComment = "the job is in running mode but the batch system shows it finished (walltime or resources exceeded?)";
                return(EJS_ERROR);
            }
            BatchJobComment = "the job was not found in the batch system";
            return(EJS_ERROR);
        default:
            return( GetJobInfoStatus() );
    }
}

//------------------------------------------------------------------------------

const CSmallString CJob::GetJobBatchComment(void)
{
    if( BatchJobComment != NULL ){
        return(BatchJobComment);
    } else {
        return("no information is currently available, please try again later ...");
    }
}

//------------------------------------------------------------------------------

const CSmallString CJob::GetBatchUserOwner(void)
{
    return( GetItem("batch/job","INF_JOB_OWNER") );
}

//------------------------------------------------------------------------------

bool CJob::HasSection(const CSmallString& section)
{
    CSmallTimeAndDate tad;
    return( HasSection(section,tad) );
}

//------------------------------------------------------------------------------

bool CJob::HasSection(const CSmallString& section,CSmallTimeAndDate& tad)
{
    CXMLElement* p_ele = GetElementByPath(section,false);
    if( p_ele == NULL ) return(false);
    p_ele->GetAttribute("timedate",tad);
    return(true);
}

//------------------------------------------------------------------------------

bool CJob::SaveJobKey(void)
{
    ofstream  ofs;
    CFileName keyname = GetFullJobName() + ".infkey";

    ofs.open(keyname);

    if( ! ofs ){
        CSmallString error;
        error << "unable to open key file '" << keyname << "'";
        ES_ERROR(error);
        return(false);
    }

    ofs << GetItem("basic/jobinput","INF_JOB_KEY") << endl;

    if( ! ofs ){
        CSmallString error;
        error << "unable to save key to file '" << keyname << "'";
        ES_ERROR(error);
        return(false);
    }

    ofs.close();

// setup correct permissions
    // FIXME
    CSmallString sumask = GetItem("specific/resources","INF_UMASK");
    mode_t umask = CUser::GetUMaskMode(sumask);

    int mode = 0666;
    int fmode = (mode & (~ umask)) & 0777;
    CFileSystem::SetPosixMode(keyname,fmode);

    CSmallString sgroup = GetItem("specific/resources","INF_UGROUP");
    gid_t group = User.GetGroupID(sgroup);

    int ret = chown(keyname,-1,group);
    if( ret != 0 ){
        CSmallString warning;
        warning << "unable to set owner and group of file '" << keyname << "' (" << ret << ")";
        ES_WARNING(warning);
    }
    return(true);
}

//------------------------------------------------------------------------------

void CJob::PrintJobQStatInfo(std::ostream& sout,bool includepath,bool includecomment)
{
//    sout << "# ST    Job ID        User        Job Title         Queue      NCPUs NGPUs NNods Last change         " << endl;
//    sout << "# -- ------------ ------------ --------------- --------------- ----- ----- ----- --------------------" << endl;

    sout << "  " << right;
        switch( BatchJobStatus ){
            case EJS_PREPARED:
                sout << "<yellow>" << setw(2) << GetItem("batch/job","INF_JOB_STATE",true) << "</yellow> ";
                break;
            case EJS_NONE:
            case EJS_FINISHED:
                sout << setw(2) << GetItem("batch/job","INF_JOB_STATE",true) << " ";
                break;
            case EJS_ERROR:
            case EJS_INCONSISTENT:
            case EJS_KILLED:
                sout << "<red>" << setw(2) << GetItem("batch/job","INF_JOB_STATE",true) << "</red> ";
                break;

            case EJS_SUBMITTED:
                sout << "<purple>" << setw(2) << GetItem("batch/job","INF_JOB_STATE",true) << "</purple> ";
                break;
            case EJS_RUNNING:
                sout << "<green>" << setw(2) << GetItem("batch/job","INF_JOB_STATE",true) << "</green> ";
                break;
        }

    CSmallString id = GetItem("batch/job","INF_JOB_ID");
    id << ShortServerName;
    sout << right << setw(12) << id << " ";

    CSmallString user = GetItem("batch/job","INF_JOB_OWNER");
    if( user.GetLength() > 12 ){
        user = user.GetSubStringFromTo(0,11);
    }
    sout << left << setw(12) << user << " ";

    CSmallString title = GetItem("batch/job","INF_JOB_TITLE");
    if( title.GetLength() > 15 ){
        title = title.GetSubStringFromTo(0,14);
    }
    sout << left << setw(15) << title << " ";

    CSmallString queue = GetItem("batch/job","INF_JOB_QUEUE");
    if( queue.GetLength() > 15 ){
        queue = queue.GetSubStringFromTo(0,14);
    }
    sout << left << setw(15) << queue;

    CSmallString tmp;
    tmp = GetItem("specific/resources","INF_NCPU");
    sout << " " << right << setw(5) << tmp;
    tmp = GetItem("specific/resources","INF_NGPU");
    sout << " " << right << setw(5) << tmp;
    tmp = GetItem("specific/resources","INF_NNODE");
    sout << " " << right << setw(5) << tmp;

    CSmallTimeAndDate current_time;
    current_time.GetActualTimeAndDate();

    sout << " " << setw(25);
    switch( BatchJobStatus ){
        case EJS_NONE:
        case EJS_PREPARED:{
            CSmallTimeAndDate ptime(GetItem("batch/job","INF_CREATE_TIME").ToLInt());
            CSmallTime diff = current_time - ptime;
            sout << diff.GetSTimeAndDay();
            }
            break;
        case EJS_SUBMITTED:{
            CSmallTimeAndDate ptime(GetItem("batch/job","INF_SUBMIT_TIME").ToLInt());
            CSmallTime diff = current_time - ptime;
            sout << diff.GetSTimeAndDay();
            }
            break;
        case EJS_RUNNING:{
            CSmallTimeAndDate ptime(GetItem("batch/job","INF_START_TIME").ToLInt());
            CSmallTime diff = current_time - ptime;
            sout << setw(12) << right;
            sout << diff.GetSTimeAndDay();
            sout << "/";
            CSmallTime wtime(GetItem("batch/job","INF_WALL_TIME").ToLInt());
            sout << setw(12) << right;
            sout << wtime.GetSTimeAndDay();
            }
            break;
        case EJS_FINISHED:
        case EJS_KILLED:{
            CSmallTimeAndDate ptime(GetItem("batch/job","INF_FINISH_TIME").ToLInt());
            sout << ptime.GetSDateAndTime();
            }
            break;
        case EJS_ERROR:
        case EJS_INCONSISTENT:
            // nothing to be here
            break;
    }

    sout << endl;

    if( includepath ){
        CSmallString title = GetItem("batch/job","INF_JOB_TITLE");
        if( title == "STDIN" ){
                sout << "     <blue>> Interactive job </blue>" << endl;
        } else {
            if( IsJobDirLocal(true) ){
                sout << "     <blue>> " << GetItem("basic/jobinput","INF_JOB_PATH") << "</blue>" << endl;
            } else {
                sout << "     <blue>> " << GetItem("basic/jobinput","INF_JOB_MACHINE") << ":" << GetItem("basic/jobinput","INF_JOB_PATH") << "</blue>" << endl;
            }
        }
    }

    if( includecomment ){
        switch( BatchJobStatus ){
            case EJS_NONE:
            case EJS_PREPARED:
            case EJS_FINISHED:
                // nothing to be here
                break;

            case EJS_ERROR:
            case EJS_INCONSISTENT:
            case EJS_KILLED:
                sout << "     <red>" << GetJobBatchComment() << "</red>" << endl;
                break;
            case EJS_SUBMITTED:
                sout << "     <purple>" << GetJobBatchComment() << "</purple>" << endl;
                break;
            case EJS_RUNNING:
                sout << "     <green>" << GetItem("start/workdir","INF_MAIN_NODE");
                if( GetItem("specific/resources","INF_NNODE").ToInt() > 1 ){
                    sout << ",+";
                }
                sout << "</green>" << endl;
                break;
        }
    }

}

//------------------------------------------------------------------------------

void CJob::RegisterIJob(const CSmallString& dir, const CSmallString& name, int ncpus)
{
    CXMLElement* p_mele = GetElementByPath("specific/ijobs",true);
    CXMLElement* p_iele = p_mele->CreateChildElement("ijob");
    p_iele->SetAttribute("path",dir);
    p_iele->SetAttribute("name",name);
    p_iele->SetAttribute("ncpus",ncpus);
}

//------------------------------------------------------------------------------

int CJob::GetNumberOfIJobs(void)
{
    CXMLElement* p_mele = GetElementByPath("specific/ijobs",true);
    CXMLIterator I(p_mele);
    return( I.GetNumberOfChildElements("ijob") );
}

//------------------------------------------------------------------------------

bool CJob::GetIJobProperty(int id,const CSmallString& name, CSmallString& value)
{
    CXMLElement* p_mele = GetElementByPath("specific/ijobs",true);
    CXMLIterator I(p_mele);
    CXMLElement* p_iele = I.SetToChildElement("ijob",id-1);

    value = NULL;
    if( p_iele == NULL ){
        CSmallString error;
        error << "unable to find ijob with ID="<<id;
        ES_ERROR(error);
        return(false);
    }
    if( p_iele->GetAttribute(name,value) == false ){
        CSmallString error;
        error << "unable to get ijob attribute name="<<name;
        ES_ERROR(error);
        return(false);
    }
    return(true);
}

//------------------------------------------------------------------------------

void CJob::SetIJobProperty(int id,const CSmallString& name,const CSmallString& value)
{
    CXMLElement* p_mele = GetElementByPath("specific/ijobs",true);
    CXMLIterator I(p_mele);
    CXMLElement* p_iele = I.SetToChildElement("ijob",id-1);

    if( p_iele == NULL ){
        CSmallString error;
        error << "unable to find ijob with ID="<<id;
        ES_ERROR(error);
        return;
    }
    p_iele->SetAttribute(name,value);
}

//------------------------------------------------------------------------------

bool CJob::IsInteractiveJob(void)
{
    CFileName job_type;
    job_type = GetItem("basic/jobinput","INF_JOB_TYPE",true);
    return( job_type == "interactive" );
}

//------------------------------------------------------------------------------

bool CJob::IsTerminalReady(void)
{
    return( IsInteractiveJob() && HasSection("terminal") );
}

//------------------------------------------------------------------------------

CFileName CJob::GetJobInputPath(void)
{
    CFileName pwd = CShell::GetSystemVariable("PWD");
    CFileName cwd;
    CFileSystem::GetCurrentDir(cwd);

    if( pwd == NULL ){
        return(cwd);
    }

    struct stat pwd_stat;
    if( stat(pwd,&pwd_stat) != 0 ) return(cwd);

    struct stat cwd_stat;
    stat(cwd,&cwd_stat);

    if( (pwd_stat.st_dev == cwd_stat.st_dev) &&
        (pwd_stat.st_ino == cwd_stat.st_ino) ) {
        return(pwd);
    }

    return(cwd);
}

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================



