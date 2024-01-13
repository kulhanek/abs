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
#include <boost/algorithm/string/trim.hpp>
#include <boost/format.hpp>
#include <JobList.hpp>
#include <PluginDatabase.hpp>
#include <PluginObject.hpp>
#include <CategoryUUID.hpp>
#include <JobType.hpp>
#include <iostream>
#include <vector>
#include <sys/sysmacros.h>
#include <sys/stat.h>
#include <unistd.h>
#include <BatchServers.hpp>
#include <ResourceList.hpp>
#include <sys/types.h>
#include <grp.h>
#include <sstream>
#include <Host.hpp>
#include <NodeList.hpp>
#include <UserUtils.hpp>
#include <SiteController.hpp>
#include <ModuleController.hpp>
#include <HostGroup.hpp>
#include <AMSRegistry.hpp>

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
    ResourceList.SetJob(this);
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
    CFileName name = GetInputDir() / GetFullJobName() + ".info";
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
    CFileName name = GetInputDir() / GetFullJobName() + ".info";
    return( SaveInfoFile(name) );
}

//------------------------------------------------------------------------------

bool CJob::SaveInfoFileWithPerms(void)
{
    if( DoNotSave == true ) return(true);
    CFileName name = GetInputDir() / GetFullJobName() + ".info";
    bool result = SaveInfoFile(name);
    if( result == false ) return(false);

// permissions
    CSmallString sumask = GetItem("specific/resources","INF_UMASK");
    mode_t umask = CUserUtils::GetUMaskMode(sumask);

    int mode = 0666;
    int fmode = (mode & (~ umask)) & 0777;
    chmod(name,fmode);

    CSmallString sgroup = GetItem("specific/resources","INF_USTORAGEGROUP");
    if( (sgroup != NULL) && (sgroup != "-disabled-") ){
        if( GetItem("specific/resources","INF_STORAGE_MACHINE_REALM_FOR_INPUT_MACHINE") != NULL ){
            sgroup << "@" << GetItem("specific/resources","INF_STORAGE_MACHINE_REALM_FOR_INPUT_MACHINE");
        }
        gid_t group = CUserUtils::GetGroupID(sgroup,false);
        int ret = chown(name,-1,group);
        if( ret != 0 ){
            CSmallString warning;
            warning << "unable to set owner and group of file '" << name << "' (" << ret << ")";
            ES_WARNING(warning);
        }
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

//------------------------------------------------------------------------------

bool CJob::SaveAMSRegistryWithPerms(void)
{
    if( DoNotSave == true ) return(true);
    CFileName name = GetInputDir() / GetFullJobName() + ".amsreg";
    bool result = AMSRegistry.SaveRegistry(name);
    if( result == false ) return(false);

// permissions
    CSmallString sumask = GetItem("specific/resources","INF_UMASK");
    mode_t umask = CUserUtils::GetUMaskMode(sumask);

    int mode = 0666;
    int fmode = (mode & (~ umask)) & 0777;
    chmod(name,fmode);

    CSmallString sgroup = GetItem("specific/resources","INF_USTORAGEGROUP");
    if( (sgroup != NULL) && (sgroup != "-disabled-") ){
        if( GetItem("specific/resources","INF_STORAGE_MACHINE_REALM_FOR_INPUT_MACHINE") != NULL ){
            sgroup << "@" << GetItem("specific/resources","INF_STORAGE_MACHINE_REALM_FOR_INPUT_MACHINE");
        }
        gid_t group = CUserUtils::GetGroupID(sgroup,false);
        int ret = chown(name,-1,group);
        if( ret != 0 ){
            CSmallString warning;
            warning << "unable to set owner and group of file '" << name << "' (" << ret << ")";
            ES_WARNING(warning);
        }
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

    version = "INFINITY_INFO_v_4_0";

    p_header->SetAttribute("version",version);
    p_header->SetAttribute("site",SiteController.GetActiveSite());

    CSmallString absmod,absver;
    absmod = "abs";
    ModuleController.GetActiveModuleVersion(absmod,absver);
    absmod << ":" << absver;
    p_header->SetAttribute("abs",absmod);
}

//------------------------------------------------------------------------------

void CJob::CreateHeaderFromBatchJob(const CSmallString& siteid, const CSmallString& absvers)
{
    CXMLElement* p_header = GetChildElementByPath("job_info/infinity",true);

    CSmallString version;

    version = "INFINITY_INFO_v_4_0";

    p_header->SetAttribute("version",version);
    p_header->SetAttribute("site",siteid);

    CSmallString absmod;
    absmod = "abs";
    absmod << ":" << absvers;
    p_header->SetAttribute("abs",absmod);
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
    WriteInfo("basic/external","INF_EXTERNAL_FLAGS",false);
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
    bool runtime_files = AreRuntimeFiles(".");
    if( runtime_files == false ) return(true);

    CSmallString var;
    var = CShell::GetSystemVariable("INF_IGNORE_RUNTIME_FILES");
    if( var == NULL ){
        ABSConfig.GetUserConfigItem("INF_IGNORE_RUNTIME_FILES",var);
    }

    if( (var == "YES") || (ignore == true) ){
        sout << endl;
        sout << "<b><blue> WARNING: The input directory contains runtime files!</blue></b>" << endl;
        return(true);
    }

    return(false);
}

//------------------------------------------------------------------------------

bool CJob::AreRuntimeFiles(const CFileName& dir)
{
    bool            runtime_files = false;
    CSmallString    filters = "*.info;*.stdout;*.infex;*.infout;*.nodes;*.gpus;*.mpinodes;*.infkey;*.vncid;*.vncpsw;*.kill;*.hwtopo;*.amsreg";
    char*           p_filter;
    char*           p_buffer = NULL;

    p_filter = strtok_r(filters.GetBuffer(),";",&p_buffer);
    while( p_filter ){
        // detect run-time file
        CDirectoryEnum direnum;
        CFileName      file;
        direnum.SetDirName(dir);
        direnum.StartFindFile(p_filter);
        runtime_files = direnum.FindFile(file);
        if( runtime_files ) break;
        p_filter = strtok_r(NULL,";",&p_buffer);
    }

    return(runtime_files);
}

//------------------------------------------------------------------------------

ERetStatus CJob::JobInput(std::ostream& sout,bool allowallpaths,bool inputfrominfo)
{
    // test arg job name
    if( CheckJobName(sout) == false ){
        ES_TRACE_ERROR("illegal job name");
        return(ERS_FAILED);
    }

    CSmallString tmp;

    // get job directory -------------------------

    if( inputfrominfo ) {
        // already set in by collection and checked
        tmp = GetInputDir();
    } else {
        // this does not follow symlinks
        //    CFileSystem::GetCurrentDir(tmp);
        tmp = GetJobInputPath();

        tmp = JobPathCheck(tmp,sout,allowallpaths);
        if( tmp == NULL ){
            ES_TRACE_ERROR("illegal job dir");
            return(ERS_FAILED);
        }
        SetItem("basic/jobinput","INF_INPUT_DIR",tmp);
    }

    tmp = CShell::GetSystemVariable("HOSTNAME");
    SetItem("basic/jobinput","INF_INPUT_MACHINE",tmp);

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
    SetItem("basic/modules","INF_EXPORTED_MODULES",ModuleController.GetExportedModules());

    // unique job key
    SetItem("basic/jobinput","INF_JOB_KEY",CUtils::GenerateUUID());

    return(ERS_OK);
}

//------------------------------------------------------------------------------

bool CJob::DecodeResources(std::ostream& sout,bool expertmode)
{
    // input directory
    if( InputDirectory(sout) == false ){
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

// this cannot be here because of ncpuspernode resource token
// default value of ncpus and nnodes is resolved at CResourceList::GetNumOfCPUs() ...
// add required default resources
//    ResourceList.AddRawResource("ncpus","1");
//    ResourceList.AddRawResource("nnodes","1");

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

    // add project resources
    CSmallString prjres = GetItem("basic/jobinput","INF_JOB_PROJECT_RESOURCES");
    ResourceList.AddResources(prjres,sout,result,expertmode);
    if( result == false ){
        ES_TRACE_ERROR("project resources are invalid");
        return(false);
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
    SetItem("specific/resources","INF_SERVER_SHORT",ShortServerName);
    SetItem("specific/resources","INF_JOB_OWNER",User.GetName());

    CSmallString batch_server_groupns = HostGroup.GetGroupNS(BatchServerName);
    SetItem("specific/resources","INF_BATCH_SERVER_GROUPNS",batch_server_groupns);

    CSmallString input_machine_groupns          = GetItem("specific/resources","INF_INPUT_MACHINE_GROUPNS");
    CSmallString storage_machine_groupns        = GetItem("specific/resources","INF_STORAGE_MACHINE_GROUPNS");

// default batch group name - derived from the input directory group
    if( ResourceList.FindResource("batchgroup") == NULL ){
        if( storage_machine_groupns == batch_server_groupns ){
            // if the file system is compatible with the batch server
            ResourceList.AddRawResource("batchgroup",ResourceList.GetResourceValue("storagegroup"));
        } else {
            // is not then try to use the current primary group name
            if( batch_server_groupns == input_machine_groupns ){
                // take the effective group
                ResourceList.AddRawResource("batchgroup",User.GetEGroup());
            }
        }
    }

// decode user resources
    ResourceList.AddResources(sres,sout,result,expertmode);
    if( result == false ){
        ES_TRACE_ERROR("user resources are invalid");
        return(false);
    }

// resolve conflicts
    ResourceList.ResolveConflicts(ShortServerName);

// fix default values for umask and storagegroup
// do we have umask? - if not add the default one
    if( ResourceList.FindResource("umask") == NULL ){
        ResourceList.AddRawResource("umask",GetItem("specific/resources","INF_BACKUP_UMASK"));
    }
// do we have storagegroup? - if not add the default one
    if( ResourceList.FindResource("storagegroup") == NULL ){
        ResourceList.AddRawResource("storagegroup",GetItem("specific/resources","INF_BACKUP_USTORAGEGROUP"));
    }
// fix default values for ncpus and nnodes
    if( ResourceList.FindResource("ncpus") == NULL ){
        ResourceList.AddRawResource("ncpus","1");
    }
    if( ResourceList.FindResource("nnodes") == NULL ){
        ResourceList.AddRawResource("nnodes","1");
    }

// test resources
    ResourceList.PreTestResourceValues(sout,result);
    if( result == false ){
        ES_TRACE_ERROR("some resource value is invalid");
        return(false);
    }

// calculate dynamic resources
    ResourceList.ResolveDynamicResources(sout,result);
    if( result == false ){
        ES_TRACE_ERROR("some resource value is invalid");
        return(false);
    }

    ResourceList.PostTestResourceValues(sout,result);
    if( result == false ){
        ES_TRACE_ERROR("some resource value is invalid");
        return(false);
    }

// determine batch resources
    CBatchServerPtr srv_ptr = BatchServers.FindBatchServer(BatchServerName,true);
    if( srv_ptr == NULL ){
        ES_TRACE_ERROR("unable to init batch server");
        return(false);
    }

// find queue and determine walltime if not specified by the user
    if( ResourceList.FindResource("walltime") == NULL ){
        // init queues
        srv_ptr->GetQueues(QueueList);
        // get queue default resource
        CQueuePtr que_ptr = QueueList.FindQueue(queue);
        if( que_ptr == NULL ){
            if( result == true ) sout << endl;
            sout << "<b><red> ERROR: Unable to find the specified queue '" << queue << "' at the batch server!</red></b>" << endl;
            return(false);
        }
        ResourceList.AddRawResource("walltime",que_ptr->GetDefaultWallTime().GetSTimeFull());
    }

// init batch specific resources
    if( srv_ptr->InitBatchResources(&ResourceList) == false ){
        ES_TRACE_ERROR("unable to init batch resources");
        return(false);
    }

// consistency check
    if( batch_server_groupns != "personal" ){
        if( storage_machine_groupns == "personal" ){
            sout << "<b><red> ERROR: When submitting job outside of the personal computer the storage machine cannot be on the personal computer!</red></b>" << endl;
            return(false);
        }
    }


// setup specific items derived from resources
    SetItem("specific/resources","INF_NCPUS",ResourceList.GetNumOfCPUs());
    SetItem("specific/resources","INF_NGPUS",ResourceList.GetNumOfGPUs());
    SetItem("specific/resources","INF_NNODES",ResourceList.GetNumOfNodes());
    SetItem("specific/resources","INF_MEMORY",ResourceList.GetMemoryString());
    SetItem("specific/resources","INF_WALLTIME",ResourceList.GetWallTimeString());
    SetItem("specific/resources","INF_RESOURCES",ResourceList.ToString(false));
    SetItem("specific/resources","INF_MPI_SLOTS_PER_NODE",ResourceList.GetResourceValue("mpislotspernode"));
    SetItem("specific/resources","INF_APPMEM",ResourceList.GetResourceValue("appmem"));

    if( ResourceList.GetResourceValue("inputdata") != NULL ){
        SetItem("specific/resources","INF_INPUT_DATA",ResourceList.GetResourceValue("inputdata"));
    } else {
        SetItem("specific/resources","INF_INPUT_DATA","default");
    }

// umask and group
    SetItem("specific/resources","INF_USTORAGEGROUP",ResourceList.GetResourceValue("storagegroup"));
    SetItem("specific/resources","INF_UBATCHGROUP",ResourceList.GetResourceValue("batchgroup"));
    SetItem("specific/resources","INF_UMASK",ResourceList.GetResourceValue("umask"));
    SetItem("specific/resources","INF_FIXPERMS",ResourceList.GetResourceValue("fixperms"));

// setup specific items for working directory
    SetItem("specific/resources","INF_DATAIN",ResourceList.GetResourceValue("datain"));
    SetItem("specific/resources","INF_DATAOUT",ResourceList.GetResourceValue("dataout"));
    SetItem("specific/resources","INF_WORK_DIR_TYPE",ResourceList.GetResourceValue("workdir"));
    SetItem("specific/resources","INF_WORK_SIZE",ResourceList.GetWorkSizeString());

    return(true);
}

//------------------------------------------------------------------------------

bool CJob::InputDirectory(std::ostream& sout)
{
    CSmallString input_machine = GetItem("basic/jobinput","INF_INPUT_MACHINE");
    CSmallString input_dir = GetItem("basic/jobinput","INF_INPUT_DIR");
    CSmallString cwd;
    CFileSystem::GetCurrentDir(cwd);
    string       input_dir_raw(cwd);     // use cwd instead of PWD

// determine FS type of input directory and group namespace,
// storage name and storage directory

    struct stat job_dir_stat;
    if( stat(input_dir_raw.c_str(),&job_dir_stat) != 0 ){
        ES_ERROR("unable to stat CWD");
        return(false);
    }

    mode_t input_dir_umask = (job_dir_stat.st_mode ^ 0777) & 0777;
    gid_t  input_dir_gid = job_dir_stat.st_gid;

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
        string n1,n2,s1,n3,p1;
        smntpoint >> n1 >> n2 >> s1 >> n3 >> p1;
        // RT#274876
        // nfs4 mounts from one server have the same sdev, it is necessary to test also the path
        if( (s1 == sdev.str()) && (input_dir_raw.find(p1) == 0) ){
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
        if( p1 == 5 ) dest = *it;
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
    CSmallString input_machine_groupns          = HostGroup.GetGroupNS();
    CSmallString storage_machine_groupns        = HostGroup.GetGroupNS();
    CSmallString storage_machine_group_realm    = HostGroup.GetRealm();

// determine storage machine and storage path
    CSmallString storage_machine = input_machine;
    CSmallString storage_dir = input_dir;

    if( (fstype == "nfs4:krb5") || (fstype == "nfs4:sys") ){
        // determine server name and data directory
        string smach = src.substr(0,src.find(":"));
        string spath = src.substr(src.find(":")+1,string::npos);
        if( input_dir_raw.find(dest) == string::npos ){
            CSmallString error;
            error << "mnt dest (" << dest << ") is not root of cwd (" << input_dir_raw << ")";
            ES_ERROR(error);
            return(false);
        }
        if( spath == "/" ) spath = ""; // remove root '/' character
        spath = spath + input_dir_raw.substr(dest.length(),string::npos);
        storage_machine_groupns     = HostGroup.GetGroupNS(smach);
        storage_machine_group_realm = HostGroup.GetRealm(smach);

        if( ABSConfig.GetSystemConfigItem("INF_USE_NFS4_STORAGES") == "YES" ){
            storage_machine = smach;
            storage_dir = spath;
        }
    }

// default umask is derived from the input directory permission
    ResourceList.AddRawResource("umask",CUserUtils::GetUMask(input_dir_umask));

// determine storage group name - derived from the input directory group
    string gname;
    struct group* p_grp = getgrgid(input_dir_gid);
    if( p_grp != NULL ){
        gname = string(p_grp->gr_name);
    }

    if( gname.empty() ){
        ES_WARNING("unable to determine the jobdir group");
        sout << "<b><blue> WARNING: Unable to determine group name of the job input directory!" << endl;
        sout <<          "          To continue, various file system checks will be disabled, which can lead" << endl;
        sout <<          "          to data ownership inconsistency!</blue></b>" << endl;
        gname = "-disabled-";
    }

    if( gname.find("@") != string::npos ){
        SetItem("specific/resources","INF_STORAGE_MACHINE_REALM_FOR_INPUT_MACHINE",HostGroup.GetRealm(storage_machine));
        string realm = gname.substr(gname.find("@")+1,string::npos);
        if( CSmallString(realm) == HostGroup.GetRealm(storage_machine) ) {
            ResourceList.AddRawResource("storagegroup",gname.substr(0,gname.find("@")));
        } else {
            sout << "<b><red> ERROR: Consistency check: Input directory group realm '" << realm
                 << "' is not the same as the storage machine realm '" << HostGroup.GetRealm(storage_machine) << "'!</red></b>" << endl;
            return(false);
        }
    } else {
        SetItem("specific/resources","INF_STORAGE_MACHINE_REALM_FOR_INPUT_MACHINE","");
        ResourceList.AddRawResource("storagegroup",gname);
    }

// input storage
    SetItem("specific/resources","INF_INPUT_PATH_FSTYPE",fstype);
    SetItem("specific/resources","INF_INPUT_MACHINE_GROUPNS",input_machine_groupns);

    SetItem("specific/resources","INF_STORAGE_MACHINE",storage_machine);
    SetItem("specific/resources","INF_STORAGE_DIR",storage_dir);
    SetItem("specific/resources","INF_STORAGE_MACHINE_GROUPNS",storage_machine_groupns);
    SetItem("specific/resources","INF_STORAGE_MACHINE_REALM",storage_machine_group_realm);

// set backup values
    SetItem("specific/resources","INF_BACKUP_USTORAGEGROUP",ResourceList.GetResourceValue("storagegroup"));
    SetItem("specific/resources","INF_BACKUP_UMASK",ResourceList.GetResourceValue("umask"));
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
    sout << "Do you want to submit the job to the batch server (YES/NO)?" << endl;
    sout << "> ";

    string answer;
    cin >> answer;

    CSmallString sanswer(answer.c_str());
    sanswer.ToUpperCase();

    if( sanswer == "YES" ) return(true);

    sout << "Job was NOT submited to the batch server!" << endl;

    return(false);
}

//------------------------------------------------------------------------------

bool CJob::SubmitJob(std::ostream& sout,bool siblings,bool verbose,bool nocollection)
{   
// ------------------------
    if( PrepareJobScript() == false ){
         ES_TRACE_ERROR("unable to preare job script");
         return(false);
    }

    CFileName job_script;
    job_script = GetFullJobName() + ".infex";

// ------------------------
    CFileName user_script   = GetItem("basic/jobinput","INF_JOB_NAME",true);

    if( ! siblings ) {
        CSmallString sumask = GetItem("specific/resources","INF_UMASK");
        mode_t umask = CUserUtils::GetUMaskMode(sumask);

        int mode;
        int fmode;
        int ret;

        FixJobPerms();

        // job script
        mode = 0766;
        fmode = (mode & (~ umask)) & 0777;
        ret = chmod(job_script,fmode);
        if( ret != 0 ){
            CSmallString warning;
            warning << "unable to set group for file '" << job_script << "' (" << ret << ")";
            ES_WARNING(warning);
        }

        if( IsInteractiveJob() == false ){
            chmod(user_script,fmode);
        }

        // determine FS user group
        // if the FS uses composed group add a storage machine realm to the group
        CSmallString sgroup = GetItem("specific/resources","INF_USTORAGEGROUP");
        if( (sgroup != NULL) && (sgroup != "-disabled-") ){
            if( GetItem("specific/resources","INF_STORAGE_MACHINE_REALM_FOR_INPUT_MACHINE") != NULL ){
                sgroup << "@" << GetItem("specific/resources","INF_STORAGE_MACHINE_REALM_FOR_INPUT_MACHINE");
            }
            gid_t sgrid = CUserUtils::GetGroupID(sgroup,false);

            ret = chown(job_script,-1,sgrid);
            if( ret != 0 ){
                CSmallString warning;
                warning << "unable to set group for file '" << job_script << "' (" << ret << ")";
                ES_WARNING(warning);
            }

            // user script
            if( IsInteractiveJob() == false ){
                ret = chown(user_script,-1,sgrid);
                if( ret != 0 ){
                    CSmallString warning;
                    warning << "unable to set group for file '" << user_script << "' (" << ret << ")";
                    ES_WARNING(warning);
                }
            }
        }
    }

    if( (GetItem("basic/collection","INF_COLLECTION_NAME",true) == NULL) || (nocollection == true) ) {
        // submit job to batch system
        if( BatchServers.SubmitJob(*this,verbose) == false ){
            if( ! siblings ){
                sout << "<b><red>Job was NOT submited to the Batch server!</red></b>" << endl;
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
            sout << "Job was sucessfully submited to the batch server!" << endl;
            sout << "  > Job ID: " << low << GetJobID() << endl << medium;

            if( user_script == "cli" ){
                sout << endl;
                sout << "<b><blue>NOTE: This is an interactive job employing a command line interface (CLI).</blue></b>" << endl;
                sout << "<b><blue>      Once the job is running, use the pgo command to establish an interactive connection.</blue></b>" << endl;
            }

            if( user_script == "gui" ){
                sout << endl;
                sout << "<b><blue>NOTE: This is an interactive job employing a graphics user interface (GUI).</blue></b>" << endl;
                sout << "<b><blue>      Once the job is running, use the pgo command to get information how to connect to the VNC server.</blue></b>" << endl;
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

        if( collection.LoadCollection(coll_path,coll_name,coll_id,site_name,true) == false ){
            ES_ERROR("unable to load collection");
            return(false);
        }

        // add this job into collection
        CJobPtr job_ptr = JobList.FindJob(this);
        collection.AddJob(job_ptr);

        // save collection file
        if( collection.SaveCollection() == false ){
            ES_ERROR("unable to save collection");
            return(false);
        }
        if( ! siblings ){
            sout << endl;
            sout << "Job was sucessfully added into the collection '" << coll_name << "'!" << endl;
        } else {
            CSmallString tmp = GetItem("basic/arguments","INF_OUTPUT_SUFFIX");
            sout << "  > " << tmp << " -> added into the collection" << endl;
        }
    }

    return(true);
}

//------------------------------------------------------------------------------

// https://stackoverflow.com/questions/180947/base64-decode-snippet-in-c

typedef unsigned char uchar;

std::string base64_encode(const std::string &in) {

    std::string out;

    int val = 0, valb = -6;
    for (uchar c : in) {
        val = (val << 8) + c;
        valb += 8;
        while (valb >= 0) {
            out.push_back("ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/"[(val>>valb)&0x3F]);
            valb -= 6;
        }
    }
    if (valb>-6) out.push_back("ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/"[((val<<8)>>(valb+8))&0x3F]);
    while (out.size()%4) out.push_back('=');
    return out;
}

//------------------------------------------------------------------------------

bool CJob::PrepareJobScript(void)
{
// infex template
    CFileName infex_template_file;
    infex_template_file = ABSConfig.GetABSRootDir() / "share" / "scripts" / "abs-execution-script-L0";

    std::ifstream infex_template_fin;
    infex_template_fin.open(infex_template_file);
    if( ! infex_template_fin ){
        CSmallString error;
        error << "unable to open infex template '" << infex_template_file << "'";
        ES_ERROR(error);
        return(false);
    }

    std::ostringstream infex_template_sstr;
    infex_template_sstr << infex_template_fin.rdbuf();

    if( ! infex_template_fin ){
        CSmallString error;
        error << "unable to read infex template '" << infex_template_file << "'";
        ES_ERROR(error);
        return(false);
    }
    infex_template_fin.close();

    std::string infex_template = infex_template_sstr.str();

// AMS registry
    CFileName ams_registry_file;
    ams_registry_file = GetFullJobName() + ".amsreg";

    std::ifstream ams_registry_fin;
    ams_registry_fin.open(ams_registry_file);

    if( ! ams_registry_fin ){
        CSmallString error;
        error << "unable to open AMS registry '" << ams_registry_file << "'";
        ES_ERROR(error);
        return(false);
    }

    std::ostringstream ams_registry_sstr;
    ams_registry_sstr << ams_registry_fin.rdbuf();

    if( ! ams_registry_fin ){
        CSmallString error;
        error << "unable to read AMS registry '" << ams_registry_file << "'";
        ES_ERROR(error);
        return(false);
    }
    ams_registry_fin.close();

    std::string ams_registry = base64_encode(ams_registry_sstr.str());
    std::ostringstream ams_registry_wrapped_str;

    int len = 0;
    for(char c : ams_registry){
        ams_registry_wrapped_str << c;
        len++;
        if( len % 80 == 0 ) ams_registry_wrapped_str << endl;
    }

    std::string ams_registry_wrapped = ams_registry_wrapped_str.str();

    // decode base64

// substitute
    std::string key;
    key = "__AMSREGISTRY__";
    infex_template.replace(infex_template.find(key),key.size(),ams_registry_wrapped);

// save
    CFileName job_script_file;
    job_script_file = GetFullJobName() + ".infex";

    std::ofstream infex_script_fout;
    infex_script_fout.open(job_script_file);

    if( ! infex_script_fout ){
        CSmallString error;
        error << "unable to open the infex script '" << job_script_file << "'";
        ES_ERROR(error);
        return(false);
    }

    infex_script_fout << infex_template;
    if( ! infex_script_fout ){
        CSmallString error;
        error << "unable to write the infex script '" << job_script_file << "'";
        ES_ERROR(error);
        return(false);
    }
    infex_script_fout.close();

    return(true);
}

//------------------------------------------------------------------------------

void CJob::FixJobPerms(void)
{
    CSmallString fixperms = ResourceList.GetResourceValue("fixperms");

    if( (fixperms == NULL) || (fixperms == "none") ) return;

    gid_t groupid = -1;

    CSmallString sgroup = GetItem("specific/resources","INF_USTORAGEGROUP");
    if( (sgroup != NULL) && (sgroup != "-disabled-") ) {
        if( GetItem("specific/resources","INF_STORAGE_MACHINE_REALM_FOR_INPUT_MACHINE") != NULL ){
            sgroup << "@" << GetItem("specific/resources","INF_STORAGE_MACHINE_REALM_FOR_INPUT_MACHINE");
        }
        groupid = CUserUtils::GetGroupID(sgroup,false);
    }

    CSmallString sumask = GetItem("specific/resources","INF_UMASK");
    mode_t umask = CUserUtils::GetUMaskMode(sumask);

    string sfixperms(fixperms);
    vector<string> fixmodes;
    split(fixmodes,sfixperms,is_any_of("+"),token_compress_on);

    vector<string>::iterator it = fixmodes.begin();
    vector<string>::iterator ie = fixmodes.end();

    while( it != ie ){
        string fixmode = *it;
        if( fixmode == "jobdir" ){
            FixJobPermsJobDir(groupid,umask);
        } else if( fixmode == "jobdata" ){
            FixJobPermsJobData(groupid,umask);
        } else if ( fixmode == "parent" ) {
            FixJobPermsParent(groupid,umask);
        }
        it++;
    }
}

//------------------------------------------------------------------------------

void CJob::FixJobPermsJobDir(gid_t groupid,mode_t umask)
{
    CSmallString input_dir  = GetItem("basic/jobinput","INF_INPUT_DIR");

    int ret;

    struct stat my_stat;
    memset(&my_stat,0,sizeof(my_stat));
    stat(input_dir,&my_stat);

    mode_t mode = 0777;
    // preserve sticky-bits
    mode_t fmode = (my_stat.st_mode & 07000) | ((mode & (~ umask)) & 0777);
    ret = chmod(input_dir,fmode);
    if( ret != 0 ){
        CSmallString warning;
        warning << "unable to set permissions for directory '" << input_dir << "' (" << ret << ")";
        ES_WARNING(warning);
    }

    ret = chown(input_dir,-1,groupid);
    if( ret != 0 ){
        CSmallString warning;
        warning << "unable to set user group for directory '" << input_dir << "' (" << ret << ")";
        ES_WARNING(warning);
    }
}

//------------------------------------------------------------------------------

void CJob::FixJobPermsJobData(gid_t groupid,mode_t umask)
{
    CFileName input_dir  = GetItem("basic/jobinput","INF_INPUT_DIR");
    CSmallString excluded_files  = GetItem("basic/jobinput","INF_EXCLUDED_FILES");

    std::set<std::string> exclusions;
    std::string sexlusions(excluded_files);
    split(exclusions,sexlusions,is_any_of(" "),token_compress_on);

    FixJobPermsJobDataDir(input_dir,exclusions,groupid,umask,0);
}

//------------------------------------------------------------------------------

void CJob::FixJobPermsJobDataDir(CFileName& dir,const std::set<std::string>& exclusions,const gid_t groupid,
                                const mode_t umask,const int level)
{
    DIR* p_dir = opendir(dir);
    if( p_dir == NULL ) return;

    struct dirent* p_dirent;
    while( (p_dirent = readdir(p_dir)) != NULL ){
        if( strcmp(p_dirent->d_name,".") == 0 ) continue;
        if( strcmp(p_dirent->d_name,"..") == 0 ) continue;

        CFileName full_name = dir / CFileName(p_dirent->d_name);
        struct stat my_stat;
        if( stat(full_name,&my_stat) == 0 ){
            if( S_ISREG(my_stat.st_mode) ||  S_ISDIR(my_stat.st_mode) ){
                mode_t mode = 0600;
                if( S_ISREG(my_stat.st_mode) ){
                    if( S_IXUSR & my_stat.st_mode ){
                        mode = 0766;
                    } else {
                        mode = 0666;
                    }
                }
                if( S_ISDIR(my_stat.st_mode) ){
                    mode = 0777;
                }
                // preserve sticky-bits
                mode_t fmode = (my_stat.st_mode & 07000) | ((mode & (~ umask)) & 0777);
                chmod(full_name,fmode);
                chown(full_name,-1,groupid);
            }
            if( S_ISDIR(my_stat.st_mode) ){
                // skip directories from exclusion list on the first level only
                if( (level == 0) && ( exclusions.count(string(p_dirent->d_name)) != 0) ) continue;
                // recursion
                FixJobPermsJobDataDir(full_name,exclusions,groupid,umask,level+1);
            }
        }
    }

    closedir(p_dir);
}

//------------------------------------------------------------------------------

void CJob::FixJobPermsParent(gid_t groupid,mode_t umask)
{
    CFileName input_dir  = GetItem("basic/jobinput","INF_INPUT_DIR");
    bool setumask = true;
    bool setgroup = true;
    CFileName updir = input_dir.GetFileDirectory();  // skip job dir
    FixJobPermsParent(updir,groupid,umask,setgroup,setumask);
}

//------------------------------------------------------------------------------

void CJob::FixJobPermsParent(const CFileName& dir,gid_t groupid,mode_t umask,bool& setgroup,bool& setumask)
{
    // read project file if exists
    CFileName pfile = dir / ".project";
    ifstream ifs(pfile);
    if( ifs ){
        string line;
        getline(ifs,line); // skip project name
        // read project resources - one per line
        while( getline(ifs,line) ){
            if( line.find("storagegroup") != string::npos ) setgroup = false;
            if( line.find("umask") != string::npos ) setumask = false;
            if( line.find("fixperms") != string::npos ){
                setgroup = false;
                setumask = false;
            }
        }
    }

    if( (setumask == false) && (setgroup == false) ) return; // nothing to set

    // set permissions
    int ret;

    if( setumask ){

        struct stat my_stat;
        memset(&my_stat,0,sizeof(my_stat));
        stat(dir,&my_stat);

        mode_t mode = 0777;
        // preserve sticky-bits
        mode_t fmode = (my_stat.st_mode & 07000) | ((mode & (~ umask)) & 0777);

        ret = chmod(dir,fmode);
        if( ret != 0 ){
            CSmallString warning;
            warning << "unable to set permissions for directory '" << dir << "' (" << ret << ")";
            ES_WARNING(warning);
        }
    }

    if( setgroup ){
        ret = chown(dir,-1,groupid);
        if( ret != 0 ){
            CSmallString warning;
            warning << "unable to set user group for directory '" << dir << "' (" << ret << ")";
            ES_WARNING(warning);
        }
    }

    // go one level up
    CFileName updir = dir.GetFileDirectory();
    if( updir != NULL ){
        FixJobPermsParent(updir,groupid,umask,setgroup,setumask);
    }
}

//------------------------------------------------------------------------------

ERetStatus CJob::ResubmitJob(bool verbose)
{

// FIXME
    // it would be safer if the status of INF_EXTERNAL_START_AFTER would be checked before this job is submited
    // here we assume that the job was already terminated
    SetItem("basic/external","INF_EXTERNAL_START_AFTER","");

    // go to job directory
    CFileSystem::SetCurrentDir(GetInputDir());

    // four /dev/null output
    stringstream tmpout;

    tmpout.str("");
    ERetStatus retstat = JobInput(tmpout,false,true);
    if( retstat == ERS_FAILED ){
        ES_TRACE_ERROR(tmpout.str());
        ES_TRACE_ERROR("unable to set job input");
        return(ERS_FAILED);
    }
    if( retstat == ERS_TERMINATE ){
        // save job info - update infor file with proper CYCLE
        if( SaveInfoFileWithPerms() == false ){
            ES_ERROR("unable to save job info file");
            return(ERS_FAILED);
        }
        return(ERS_TERMINATE);
    }

    // destroy start/stop/kill sections
    DestroySection("submit");
    DestroySection("start");
    DestroySection("stop");
    DestroySection("kill");

    // resources
    tmpout.str("");
    if( DecodeResources(tmpout,true) == false ){
        ES_TRACE_ERROR(tmpout.str());
        ES_TRACE_ERROR("unable to decode resources");
        return(ERS_FAILED);
    }

    // last job check
    tmpout.str("");
    if( LastJobCheck(tmpout) == false ){
        ES_TRACE_ERROR(tmpout.str());
        ES_TRACE_ERROR("job submission was canceled by last check procedure");
        return(ERS_FAILED);
    }

    // we need registry for the job submission
    // save AMS registry
    if( SaveAMSRegistryWithPerms() == false ){
        ES_ERROR("unable to save ams registry");
        return(ERS_FAILED);
    }

    // submit job
    tmpout.str("");
    if( SubmitJob(tmpout,false,verbose,true) == false ){
        ES_TRACE_ERROR(tmpout.str());
        ES_TRACE_ERROR("unable to submit job");
        return(ERS_FAILED);
    }

    // save job info
    if( SaveInfoFileWithPerms() == false ){
        ES_ERROR("unable to save job info file");
        return(ERS_FAILED);
    }

    if( SaveJobKey() == false ){
        ES_ERROR("unable to save job key file");
        return(ERS_FAILED);
    }

    return(ERS_OK);
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

    // update job status from batch system, set following items
    //    job.SetItem("specific/resources","INF_REAL_QUEUE",tmp);
    //    job.SetItem("specific/resources","INF_REAL_SERVER",tmp);

    // init all subsystems
    if( ABSConfig.LoadSystemConfig() == false ){
        ES_ERROR("unable to load ABSConfig config");
        return(false);
    }

    // get job status and reason
    if( BatchServers.GetJobStatus(*this) == false ){
        ES_ERROR("unable to update job status");
        return(false);
    }

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
        string name,gpus;
        try{
            stringstream str(node);
            string tmp;
            str >> name >> tmp;
            gpus = tmp.substr(gpus.find("=")+1,string::npos);
        } catch(...){
            // nothing to be here
        }
        vector<string> gpuids;
        split(gpuids,gpus,is_any_of(","),boost::token_compress_on);
        vector<string>::iterator it = gpuids.begin();
        vector<string>::iterator ie = gpuids.end();
        while( it != ie ){
            string id = *it;
            if( ! id.empty() ){
                CXMLElement* p_sele = p_ele->CreateChildElement("gpu");
                p_sele->SetAttribute("name",name);
                p_sele->SetAttribute("id",id);
            }
            it++;
        }
    }

    snodefile.close();

    return(true);
}

//------------------------------------------------------------------------------

void CJob::WriteCLITerminalReady(const CSmallString& agent)
{
    CreateSection("terminal");
    SetItem("terminal","INF_AGENT_MODULE",agent);
}

//------------------------------------------------------------------------------

void CJob::WriteGUITerminalReady(const CSmallString& agent,const CSmallString& vncid)
{
    CreateSection("terminal");
    SetItem("terminal","INF_AGENT_MODULE",agent);
    SetItem("terminal","INF_VNC_ID",vncid);
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

// HW utilization


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
    if( ABSConfig.IsServerConfigured(GetServerName()) == false ){
        // job was run under different server
        ES_TRACE_ERROR("job batch server is not available under the current site");
        return(false);
    }

    switch( GetJobStatus() ){
        case EJS_SUBMITTED:
        case EJS_BOOTING:
        case EJS_RUNNING:{
            // try to kill job
            bool result = BatchServers.KillJob(*this);
            if( result == true ) CreateSection("kill");
            return(result);
            }
        default:
            ES_ERROR("unable to kill job that is not queued/booting/running");
            return(false);
    }
}

//------------------------------------------------------------------------------

bool CJob::UpdateJobStatus(void)
{
    switch( GetJobInfoStatus() ){
        case EJS_SUBMITTED:
        case EJS_RUNNING:
            break;
        default:
            // job was not submitted neither is running
            BatchJobStatus = GetJobInfoStatus();
            return(true);
    }

    if( ABSConfig.IsServerConfigured(GetServerName()) == false ){
        // job was run under different server
        ES_TRACE_ERROR("job batch server is not available under the current site");
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

    string legall_characters = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ01234567890_+-.";
    string legall_char_short = "a-z A-Z 0-9 _+-.";

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

const CSmallString CJob::JobPathCheck(const CSmallString& inpath,std::ostream& sout,bool allowallpaths)
{
    CSmallString outpath = inpath;

    string legall_characters = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ01234567890_+-./";
    string legall_char_short = "a-z A-Z 0-9 _+-./";

    string soutpath(outpath);
    if( soutpath.find_first_not_of(legall_characters) != string::npos ){
        sout << endl;
        sout << "<b><red> ERROR: The job input directory path '" << soutpath << "' contains illegal characters!</red></b>" << endl;
        sout <<         "        Legal characters are : " << legall_char_short << endl;
        return("");
    }

    // check if job dir is really directory
    if( ! CFileSystem::IsDirectory(outpath) ){
        sout << endl;
        sout << "<b><red> ERROR: The job input directory '" << outpath << "' is not a directory!</red></b>" << endl;
        return("");
    }

    // is it writable?
    CFileName test_file;
    test_file = CFileName(outpath) / ".________INFINITY_______";
    ofstream ofs(test_file);
    ofs << "________INFINITY_______" << endl;
    if( ! ofs ){
        sout << endl;
        sout << "<b><red> ERROR: The job input directory '" << outpath << "' is not writable!</red></b>" << endl;
        return("");
    } else {
        CFileSystem::RemoveFile(test_file);
    }

    if( ! allowallpaths ){
        // check if job input directory path is allowed
        if( ABSConfig.IsInputJobPathAllowed(inpath) == false ){
            sout << endl;
            sout << "<b><red> ERROR: The job input directory '" << outpath << "' is not allowed!</red></b>" << endl;
            ABSConfig.PrintAllowedJobInputPaths(sout);
            return("");
        }
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
    CSmallString curdir = GetJobInputPath();
    CSmallString resources;
    CSmallString project = GetJobProject(curdir,resources);
    SetItem("basic/jobinput","INF_JOB_PROJECT",project);
    SetItem("basic/jobinput","INF_JOB_PROJECT_RESOURCES",resources);
}

//------------------------------------------------------------------------------

CSmallString CJob::GetJobProject(const CFileName& dir,CSmallString& resources)
{
    CSmallString project;

    // first one level up
    CFileName updir = dir.GetFileDirectory();
    if( updir != NULL ){
        CSmallString ppath = GetJobProject(updir,resources);
        project << ppath;
    }

    CFileName pfile = dir / ".project";
    if( ! CFileSystem::IsFile(pfile) ) {
        return(project);
    }

    // read project file
    ifstream ifs(pfile);
    string line;
    // read project name
    if( getline(ifs,line) ){
        if( project != NULL ) project << "/";
        project << line.c_str();
    }
    // read project resources - one per line
    while( getline(ifs,line) ){
        if( resources != NULL ) resources << ",";
        resources << line;
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
        // no open collection, try .collinfo file
        if( CFileSystem::IsFile(".collinfo") == false ) return;

        // try to read .collinfo file
        ifstream ifs(".collinfo");
        if( ! ifs ) return;

        string id,path,name;
        if( ! getline(ifs,id) ) return;
        if( ! getline(ifs,path) ) return;
        if( ! getline(ifs,name) ) return;

        coll_id = id;
        coll_path = path;
        coll_name = name;

        // test if collection file exists
        CFileName cofi = CFileName(coll_path) / CFileName(coll_name) + ".cofi";
        if( CFileSystem::IsFile(cofi) == false ) return;

        // This was commented becuase it was a bad idea, it slowed the processeing significantly
        // so DO NOT re-read the collection contents
        // CJobList coll;
        // if( coll.LoadCollection(cofi,true) == false ) return;
        // coll_id = coll.CollectionID;
        // coll_path = coll.CollectionPath;
        // coll_name = coll.CollectionName;
    }

    // setup collection
    SetItem("basic/collection","INF_COLLECTION_PATH",coll_path);
    SetItem("basic/collection","INF_COLLECTION_NAME",coll_name);
    SetItem("basic/collection","INF_COLLECTION_ID",coll_id);

    // save .collinfo file
    ofstream ofs(".collinfo");
    if( ofs ){
        ofs << coll_id << endl;
        ofs << coll_path << endl;
        ofs << coll_name << endl;
    }
}

//------------------------------------------------------------------------------

bool CJob::IsJobInCollection(void)
{
   return( GetItem("basic/collection","INF_COLLECTION_ID",true) != NULL );
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

    return(p_iele->GetAttribute("value",value));
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

    return(p_iele->GetAttribute("value",value));
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

    CSmallString name;
    p_rele->GetAttribute("site",name);
    return(name);
}

//------------------------------------------------------------------------------

const CSmallString CJob::GetABSModule(void)
{
    CXMLElement* p_rele = GetElementByPath("infinity",false);
    if( p_rele == NULL ){
        ES_ERROR("infinity element was not found");
        return("");
    }

    CSmallString absmod;
    p_rele->GetAttribute("abs",absmod);

    return(absmod);
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
    CSmallString tmp;
    tmp = GetItem("specific/resources","INF_SERVER",true);
    // RT#222440
    if( tmp == NULL ){
        // try the old location
        tmp = GetServerNameV2();
    }
    if( tmp == NULL ){
        tmp = "unknown";
    }
    return(tmp);
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
        case EJS_BOOTING:
            HasSection("submit",btad);
            break;
        case EJS_MOVED:
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

bool CJob::GetVNodes(void)
{
    bool result = true;

    std::set<std::string>::iterator it = ExecVNodes.begin();
    std::set<std::string>::iterator ie = ExecVNodes.end();
    while( it != ie ){
        CSmallString name(*it);
        // append the server name
        name << "@" << ShortServerName;
        // get node
        result &= BatchServers.GetNode(NodeList,name);
        it++;
    }

    return(result);
}

//------------------------------------------------------------------------------

bool CJob::IsJobDirLocal(bool no_deep)
{
    if( GetItem("basic/jobinput","INF_INPUT_MACHINE",false) == ABSConfig.GetHostName() ){
        return(true);
    }
    if( no_deep ) return(false); // no deep checking

    // try to read key file and compare its contents
    ifstream  ifs;
    CFileName keyname = GetFullJobName() + ".infkey";

    ifs.open(keyname);
    if( ! ifs ) return(false);
    string key;
    ifs >> key;

    return( GetItem("basic/jobinput","INF_JOB_KEY") == CSmallString(key) );
}

//------------------------------------------------------------------------------

void CJob::SetSimpleJobIdentification(const CSmallString& name, const CSmallString& machine,
                                      const CSmallString& path)
{
    SetItem("basic/jobinput","INF_JOB_TITLE",name);
    SetItem("basic/jobinput","INF_JOB_NAME",name);
    SetItem("basic/jobinput","INF_INPUT_MACHINE",machine);
    SetItem("basic/jobinput","INF_INPUT_DIR",path);
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

int CJob::GetCurrentRecycleJob(void)
{
    CSmallString start;
    GetItem("basic/recycle","INF_RECYCLE_START",start,true);
    if( start == NULL ) return(1);

    CSmallString current;
    GetItem("basic/recycle","INF_RECYCLE_CURRENT",current,true);

    return( current.ToInt() - start.ToInt() + 1);
}

//------------------------------------------------------------------------------

void CJob::IncrementRecycleStage(void)
{
    CSmallString current;
    GetItem("basic/recycle","INF_RECYCLE_CURRENT",current,true);
    if( current != NULL ){
        int inum = current.ToInt();
        inum++;
        SetItem("basic/recycle","INF_RECYCLE_CURRENT",inum);
    }
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

    // get surrogate machines if any
    CSmallString surrogates = HostGroup.GetSurrogateMachines();
    ShellProcessor.SetVariable("INF_SURROGATE_MACHINES",surrogates);
}

//------------------------------------------------------------------------------

bool CJob::PrepareGoWorkingDirEnv(bool noterm)
{
    CSmallString job_key = CShell::GetSystemVariable("INF_JOB_KEY");
    if( IsInteractiveJob() && job_key == GetJobKey() ){
        ES_ERROR("already in CLI/GUI");
        return(false);
    }

    bool result = true;

    CSmallString bscript;
    if( ABSConfig.GetSystemConfigItem("INF_BOOT_SCRIPT",bscript) == false ){
        // FIXME
        // hardcoded backup
        bscript = "/software/ncbr/softmods/9.0/etc/boot/bashrc.ncbr";
    }

    ShellProcessor.SetVariable("INF_GO_BOOT_SCRIPT",bscript);
    ShellProcessor.SetVariable("INF_GO_SITE",GetSiteName());

    CSmallString tmp;
    tmp = NULL;
    result &= GetItem("specific/resources","INF_UMASK",tmp);
    ShellProcessor.SetVariable("INF_GO_UMASK",tmp);
    tmp = NULL;
    result &= GetItem("start/workdir","INF_MAIN_NODE",tmp);
    ShellProcessor.SetVariable("INF_GO_MAIN_NODE",tmp);
    tmp = NULL;
    result &= GetItem("start/workdir","INF_WORK_DIR",tmp);
    ShellProcessor.SetVariable("INF_GO_WORK_DIR",tmp);
    tmp = NULL;
    result &= GetItem("basic/jobinput","INF_JOB_KEY",tmp);
    ShellProcessor.SetVariable("INF_GO_JOB_KEY",tmp);

    CSmallString jobname;
    result &= GetItem("basic/jobinput","INF_JOB_NAME",jobname);

    if( noterm == true ){
        ShellProcessor.SetVariable("INF_GO_JOB_NAME","noterm");
    } else {
        ShellProcessor.SetVariable("INF_GO_JOB_NAME",jobname);
        // terminal items are optional as the terminal session might not be started yet
        if( jobname == "gui" ){
            CSmallString wn;
            tmp = NULL;
            result &= GetItem("basic/external","INF_EXTERNAL_NAME_SUFFIX",tmp);
            wn << jobname << tmp;
            ShellProcessor.SetVariable("INF_GO_WHOLE_NAME",wn);

            tmp = NULL;
            result &= GetItem("terminal","INF_VNC_ID",tmp);
            ShellProcessor.SetVariable("INF_GO_VNC_ID",tmp);

            tmp = NULL;
            result &= GetItem("terminal","INF_AGENT_MODULE",tmp);
            ShellProcessor.SetVariable("INF_GO_AGENT_MODULE",tmp);

            tmp = NULL;
            if( GetItem("terminal","INF_GUI_PROXY",tmp,true) == true ){
                ShellProcessor.SetVariable("INF_GO_GUI_PROXY",tmp);
            }
        }
        if( jobname == "cli" ){
            tmp = NULL;
            result &= GetItem("terminal","INF_AGENT_MODULE",tmp);
            ShellProcessor.SetVariable("INF_GO_AGENT_MODULE",tmp);
        }
    }
    return(result);
}

//------------------------------------------------------------------------------

bool CJob::PrepareGoInputDirEnv(void)
{
    bool result = true;

    CSmallString bscript;
    if( ABSConfig.GetSystemConfigItem("INF_BOOT_SCRIPT",bscript) == false ){
        // FIXME
        // hardcoded backup
        bscript = "/software/ncbr/softmods/9.0/etc/boot/bashrc.ncbr";
    }

    ShellProcessor.SetVariable("INF_GO_BOOT_SCRIPT",bscript);
    ShellProcessor.SetVariable("INF_GO_SITE",GetSiteName());

    CSmallString tmp;
    tmp = NULL;
    result &= GetItem("basic/jobinput","INF_INPUT_MACHINE",tmp);
    ShellProcessor.SetVariable("INF_GO_INPUT_MACHINE",tmp);
    tmp = NULL;
    result &= GetItem("basic/jobinput","INF_INPUT_DIR",tmp);
    ShellProcessor.SetVariable("INF_GO_INPUT_DIR",tmp);

    return(result);
}

//------------------------------------------------------------------------------

bool CJob::PrepareSyncWorkingDirEnv(void)
{
    bool result = true;

    CSmallString tmp;
    tmp = NULL;
    result &= GetItem("start/workdir","INF_MAIN_NODE",tmp);
    ShellProcessor.SetVariable("INF_SYNC_MAIN_NODE",tmp);
    tmp = NULL;
    result &= GetItem("start/workdir","INF_WORK_DIR",tmp);
    ShellProcessor.SetVariable("INF_SYNC_WORK_DIR",tmp);
    tmp = NULL;
    result &= GetItem("basic/jobinput","INF_JOB_KEY",tmp);
    ShellProcessor.SetVariable("INF_SYNC_JOB_KEY",tmp);
    tmp = NULL;
    result &= GetItem("basic/jobinput","INF_INPUT_MACHINE",tmp);
    ShellProcessor.SetVariable("INF_SYNC_INPUT_MACHINE",tmp);
    tmp = NULL;
    result &= GetItem("basic/jobinput","INF_INPUT_DIR",tmp);
    ShellProcessor.SetVariable("INF_SYNC_INPUT_DIR",tmp);

    CSmallString INF_WI_RSYNCOPTS;
    CSmallString sgroup = GetItem("specific/resources","INF_USTORAGEGROUP");
    if( sgroup != "-disabled-" ){
        if( GetItem("specific/resources","INF_INPUT_MACHINE_GROUPNS") != GetItem("specific/resources","INF_STORAGE_MACHINE_GROUPNS") ){
            INF_WI_RSYNCOPTS << "--chown=:" << sgroup << "@" << GetItem("specific/resources","INF_STORAGE_MACHINE_REALM");
        } else {
            INF_WI_RSYNCOPTS << "--chown=:" << sgroup;
        }
    }
    ShellProcessor.SetVariable("INF_SYNC_WI_RSYNCOPTS",INF_WI_RSYNCOPTS);

    CSmallString wn;
    tmp = NULL;
    result &= GetItem("basic/jobinput","INF_JOB_NAME",tmp);
    wn << tmp;
    tmp = NULL;
    result &= GetItem("basic/external","INF_EXTERNAL_NAME_SUFFIX",tmp);
    wn << tmp;
    ShellProcessor.SetVariable("INF_SYNC_WHOLE_NAME",wn);

    return(result);
}

//------------------------------------------------------------------------------

bool CJob::PrepareSoftKillEnv(void)
{
/*
    # INF_KILL_MAIN_NODE
    # INF_KILL_WORK_DIR
    # INF_KILL_WHOLE_NAME
    # INF_KILL_JOB_KEY
*/

    bool result = true;

    CSmallString tmp;
    tmp = NULL;
    result &= GetItem("start/workdir","INF_MAIN_NODE",tmp);
    ShellProcessor.SetVariable("INF_KILL_MAIN_NODE",tmp);
    tmp = NULL;
    result &= GetItem("start/workdir","INF_WORK_DIR",tmp);
    ShellProcessor.SetVariable("INF_KILL_WORK_DIR",tmp);
    tmp = NULL;
    result &= GetItem("basic/jobinput","INF_JOB_KEY",tmp);
    ShellProcessor.SetVariable("INF_KILL_JOB_KEY",tmp);

    CSmallString wn;
    tmp = NULL;
    result &= GetItem("basic/jobinput","INF_JOB_NAME",tmp);
    wn << tmp;
    tmp = NULL;
    result &= GetItem("basic/external","INF_EXTERNAL_NAME_SUFFIX",tmp);
    wn << tmp;
    ShellProcessor.SetVariable("INF_KILL_WHOLE_NAME",wn);

    return(true);
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
    if( ver == "INFINITY_INFO_v_4_0"){
        PrintJobInfoV4(sout);
    } else if( ver == "INFINITY_INFO_v_3_0"){
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
    if( ver == "INFINITY_INFO_v_4_0"){
        PrintJobInfoCompactV4(sout,includepath,includecomment);
    } else if( ver == "INFINITY_INFO_v_3_0"){
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
            sout << "<yellow>P</yellow> ";
            break;
        case EJS_SUBMITTED:
            sout << "<purple>Q</purple> ";
            break;
        case EJS_BOOTING:
            sout << "<green>B</green> ";
            break;
        case EJS_RUNNING:
            sout << "<green>R</green> ";
            break;
        case EJS_FINISHED:
            if( GetJobExitCode() == 0 ){
                sout << "F ";
            } else {
                sout << "<red>FE</red>";
            }
            break;
        case EJS_KILLED:
            sout << "<red>K</red> ";
            break;
        case EJS_ERROR:
            sout << "<red>ER</red>";
            break;
        case EJS_MOVED:
            sout << "<cyan>M</cyan>";
            break;
        case EJS_INCONSISTENT:
            sout << "<red>IN</red>";
            break;
    }
    CSmallString name = GetItem("basic/jobinput","INF_JOB_NAME");
    if( name.GetLength() > 15 ){
        name = name.GetSubStringFromTo(0,14);
    }
    sout << left;
    sout << " " << setw(15) << name;

    if( InfoFileLoaded ){
        CSmallString queue = GetItem("specific/resources","INF_QUEUE");
        if( queue.GetLength() > 15 ){
            queue = queue.GetSubStringFromTo(0,14);
        }
        sout << " " << setw(15) << queue;
        sout << right;
        sout << " " << setw(5) << GetItem("specific/resources","INF_NCPUS");
        sout << " " << setw(5) << GetItem("specific/resources","INF_NGPUS");
        sout << " " << setw(5) << GetItem("specific/resources","INF_NNODES");

        CSmallTimeAndDate   last_change;
        CSmallTimeAndDate   curr_time;
        CSmallTime          from_last_change;

        curr_time.GetActualTimeAndDate();
        last_change = GetTimeOfLastChange();

        sout << " " ;

        int tot = GetNumberOfRecycleJobs();
        int cur = GetCurrentRecycleJob();
        int per = 0;

        switch( GetJobStatus() ){
            case EJS_NONE:
            case EJS_INCONSISTENT:
            case EJS_ERROR:
            case EJS_MOVED:
                per = cur - 1;
                sout << "                    ";
                break;
            case EJS_PREPARED:
            case EJS_SUBMITTED:
            case EJS_BOOTING:
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
            case EJS_BOOTING:
            case EJS_KILLED:
                // nothing to be here
                break;

            case EJS_ERROR:
            case EJS_INCONSISTENT:
                sout << "       <red>" << BatchJobComment << "</red>" << endl;
                break;
            case EJS_SUBMITTED:
            case EJS_MOVED:
                sout << "       <green>" << BatchJobComment << "</green>" << endl;
                break;
            case EJS_RUNNING:
                sout << "       <green>" << GetItem("start/workdir","INF_MAIN_NODE") << "</green>" << endl;
                break;
        }
    }

    if( includepath ){
        if( IsJobDirLocal() ){
            sout << "       <blue>" << GetItem("basic/jobinput","INF_INPUT_DIR") << "</blue>" << endl;
        } else {
            sout << "       <blue>" << GetItem("basic/jobinput","INF_INPUT_MACHINE") << ":" << GetItem("basic/jobinput","INF_INPUT_DIR") << "</blue>" << endl;
        }
    }
}

//------------------------------------------------------------------------------

void CJob::PrintJobInfoV4(std::ostream& sout)
{
    PrintBasicV4(sout);
    PrintResourcesV4(sout);

    if( HasSection("start") == true ){
        PrintExecV4(sout);
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
        if( GetJobExitCode() == 0 ){
            sout << "F";
        } else {
            sout << "FE";
        }
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

void CJob::PrintBasicV4(std::ostream& sout)
{
    CSmallString tmp,col;

    sout << "Job name         : " << GetItem("basic/jobinput","INF_JOB_NAME") << endl;
    if( GetItem("submit/job","INF_JOB_ID",true) != NULL ){
    sout << "Job ID           : " << GetItem("submit/job","INF_JOB_ID") << endl;
    }
    sout << "Job title        : " << GetItem("basic/jobinput","INF_JOB_TITLE") << " (Job type: ";
    sout << GetItem("basic/jobinput","INF_JOB_TYPE");
    if( HasSection("basic/recycle") == true ){
        sout << " ";
        sout << format("%03d") % GetItem("basic/recycle","INF_RECYCLE_CURRENT",false).ToInt();
        sout << "/";
        sout << format("%03d") % GetItem("basic/recycle","INF_RECYCLE_STOP",false).ToInt();
    }
    sout << ")" << endl;

    sout << "Job input dir    : " << GetItem("basic/jobinput","INF_INPUT_MACHINE");
    sout << ":" << GetItem("basic/jobinput","INF_INPUT_DIR") << endl;

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

void CJob::PrintResourcesV4(std::ostream& sout)
{
    CSmallString tmp,tmp1,tmp2;

    tmp = GetItem("basic/arguments","INF_ARG_DESTINATION");
    sout << "Req destination  : " << tmp << endl;

    tmp = GetItem("basic/arguments","INF_ARG_RESOURCES");
    if( tmp != NULL ){
    PrintResourceTokens(sout,"Req resources    : ",tmp," ");
    } else {
    sout << "Req resources    : -none-" << endl;
    }

    sout << "-----------------------------------------------" << endl;

    tmp = GetItem("specific/resources","INF_SERVER_SHORT");
    sout << "Site spec        : " << GetSiteName() << "/" << GetABSModule() << "/" << GetServerName() << "|" << tmp << endl;
    CSmallString server = GetServerName();

    tmp = GetItem("specific/resources","INF_DEFAULT_RESOURCES");    
    if( tmp != NULL ){
    PrintResourceTokens(sout,"Default resources: ",tmp,", ");
    } else {
    sout << "Default resources: -none-" << endl;
    }

    tmp = GetItem("basic/jobinput","INF_JOB_PROJECT_RESOURCES",true);
    if( tmp != NULL ){
    PrintResourceTokens(sout,"Project resources: ",tmp,", ");
    } else {
    sout << "Project resources: -none-" << endl;
    }

    tmp = GetItem("specific/resources","INF_ALIAS");
    if( tmp == NULL ){
    sout << "Alias            : -none-" << endl;
    }
    else{
    sout << "Alias            : " << tmp << endl;    
    tmp = GetItem("specific/resources","INF_ALIAS_RESOURCES");
    PrintResourceTokens(sout,"Alias resources  : ",tmp,", ");
    }

    tmp = GetItem("specific/resources","INF_RESOURCES");
    PrintResourceTokens(sout,"All resources    : ",tmp,", ");

    tmp = GetItem("specific/resources","INF_QUEUE");
    sout << "Requested queue  : " << tmp << endl;
    CSmallString queue = tmp;

    sout << "-----------------------------------------------" << endl;
    sout << "NCPUs NGPUs NNodes Memory WorkSize     WallTime" << endl;
    tmp = GetItem("specific/resources","INF_NCPUS");
    sout << setw(5) << tmp;
    sout << " ";
    tmp = GetItem("specific/resources","INF_NGPUS");
    sout << setw(5) << tmp;
    sout << " ";
    tmp = GetItem("specific/resources","INF_NNODES");
    sout << setw(6) << tmp;
    sout << " ";
    tmp = GetItem("specific/resources","INF_MEMORY");
    sout << setw(6) << tmp;
    sout << " ";
    tmp = GetItem("specific/resources","INF_WORK_SIZE");
    sout << setw(8) << tmp;
    sout << " ";
    tmp = GetItem("specific/resources","INF_WALLTIME");
    CSmallTime wtime;
    wtime.SetFromString(tmp);
    sout << setw(12) << wtime.GetSTimeAndDay();
    sout << endl;

    sout << "-----------------------------------------------" << endl;
    tmp = GetItem("basic/jobinput","INF_INPUT_DIR");
    sout << "Input directory  : " << tmp << endl;

    tmp = GetItem("specific/resources","INF_INPUT_PATH_FSTYPE");
    sout << "File system type : " << tmp << endl;

    sout << "Input storage    : " << GetItem("specific/resources","INF_STORAGE_MACHINE") << ":" << GetItem("specific/resources","INF_STORAGE_DIR") << endl;

    sout << "-----------------------------------------------" << endl;

    tmp = GetItem("specific/resources","INF_WORK_DIR_TYPE");
    sout << "Work directory   : " << tmp << endl;

    tmp = GetItem("specific/resources","INF_WORK_SIZE");
    sout << "Work dir size    : " << tmp << endl;

    tmp1 = GetItem("specific/resources","INF_DATAIN");
    tmp2 = GetItem("specific/resources","INF_DATAOUT");
    sout << "Data IN/OUT      : " << tmp1 << "/" << tmp2 << endl;

    tmp = GetItem("basic/jobinput","INF_EXCLUDED_FILES");
    if( tmp == NULL ){
    sout << "Excluded files   : -none-" << endl;
    }
    else{
    sout << "Excluded files   : " << tmp << endl;
    }

    sout << "-----------------------------------------------" << endl;
    sout << "Group namespaces : ";
    sout << GetItem("specific/resources","INF_INPUT_MACHINE_GROUPNS") << " (input machine) | ";

    // refactorization: INF_STORAGE_GROUPNS -> INF_STORAGE_MACHINE_GROUPNS
    tmp = GetItem("specific/resources","INF_STORAGE_MACHINE_GROUPNS",true);
    if( tmp == NULL ){
        tmp = GetItem("specific/resources","INF_STORAGE_GROUPNS");
    }
    sout << tmp << " (storage machine) | ";
    sout << GetItem("specific/resources","INF_BATCH_SERVER_GROUPNS") << " (batch server)" <<  endl;

    // refactorization: INF_USTORAGEGROUP introduced
    tmp = GetItem("specific/resources","INF_USTORAGEGROUP",true);
    if( tmp != NULL ){
        tmp1 = GetItem("specific/resources","INF_STORAGE_MACHINE_REALM");
    sout << "Storage user grp : " <<  tmp << "[@" << tmp1 << "]" << endl;
    }

    // refactorization: INF_UGROUP -> INF_BATCH_UGROUP
    tmp = GetItem("specific/resources","INF_UBATCHGROUP",true);
    if( tmp == NULL ) tmp = "-default server group-";
    sout << "Batch user group : " <<  tmp << endl;

    tmp = GetItem("specific/resources","INF_UMASK");
    sout << "User file mask   : " << tmp << " [" << CUserUtils::GetUMaskPermissions(CUserUtils::GetUMaskMode(tmp)) << "]" <<  endl;

    tmp = GetItem("specific/resources","INF_FIXPERMS",true);
    if( tmp != NULL ){
    sout << "Fix permissions  : " << tmp << endl;
    } else {
    sout << "Fix permissions  : none" << endl;
    }

    sout << "-----------------------------------------------" << endl;

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

    // moved jobs - optional section
    bool result = true;
    CSmallString    rqueue;
    CSmallString    rsrv;
    result &= GetItem("specific/resources","INF_REAL_QUEUE",rqueue,true);
    result &= GetItem("specific/resources","INF_REAL_SERVER",rsrv,true);

    if( result ){
        if( ((rqueue != NULL) && (rqueue != queue)) ||
            ((rsrv != NULL) && (rsrv != server))  ) {
        sout << "-----------------------------------------------" << endl;
        }

        if( (rqueue != NULL) && (rqueue != queue) ){
        sout << "> Real queue     : " << rqueue << endl;
        }
        if( (rsrv != NULL) && (rsrv != server) ){
        sout << "> Real server    : " << rsrv << endl;
        }
    }

    sout << "========================================================" << endl;
}

//------------------------------------------------------------------------------

void CJob::PrintResourceTokens(std::ostream& sout,const CSmallString& title, const CSmallString& res_list,
                               const CSmallString& delim)
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
                sout << delim << endl;
                for(unsigned int i=0; i < title.GetLength(); i++){
                    sout << " ";
                }
                len = title.GetLength();
            } else {
                sout << delim;
                len += delim.GetLength();
            }
        }
    }
    sout << endl;
}

//------------------------------------------------------------------------------

bool CJob::PrintExecV4(std::ostream& sout)
{
    CSmallString tmp;

    tmp = GetItem("start/workdir","INF_MAIN_NODE");
    sout << "Main node        : " << tmp << endl;

    tmp = GetItem("start/workdir","INF_WORK_DIR");
    sout << "Working directory: " << tmp << endl;

    tmp = GetItem("terminal","INF_AGENT_MODULE",true);
    if( tmp != NULL ){
    sout << "Terminal agent   : " << tmp << endl;
    }

    tmp = GetItem("terminal","INF_VNC_ID",true);
    if( tmp != NULL ){
    sout << "VNC Server ID    : " << tmp << endl;
    }

    tmp = GetItem("stop/result","INF_JOB_EXIT_CODE",true);
    if( tmp != NULL ){
    sout << "Job exit code    : " << tmp << endl;
    }
    sout << "----------------------------------------" << endl;

    ListNodes(sout);

    if( (GetItem("specific/resources","INF_NGPUS").ToInt() > 0) &&
        (GetElementByPath("start/gpus",false) != NULL) ){
        sout << "----------------------------------------" << endl;

        ListGPUNodes(sout);
    }

    sout << "========================================================" << endl;

    return(true);
}

//----------------------------------------------

struct SNodeGroup {
    CSmallString    name;
    int             from;       // CPU/GPU ID
    int             to;         // CPU/GPU ID
    CXMLElement*    ijob;
};

typedef boost::shared_ptr<SNodeGroup>  SNodeGroupPtr;

//------------------------------------------------------------------------------

bool CJob::ListNodes(std::ostream& sout)
{
    CXMLElement* p_rele = GetElementByPath("start/nodes",false);
    if( p_rele == NULL ){
        ES_ERROR("start/nodes element was not found");
        return(false);
    }

    CXMLElement*    p_iele = GetElementByPath("specific/ijobs",false);
    CXMLIterator    I(p_rele);
    CXMLElement*    p_sele;
    CXMLIterator    J(p_iele);
    CXMLElement*    p_jele;

    list<SNodeGroupPtr> groups;
    SNodeGroupPtr       group_ptr;
    CSmallString        node;

    // generate groups
    int ncpus = 0;
    if( p_iele == NULL ){
        while( (p_sele = I.GetNextChildElement("node")) != NULL ){
            p_sele->GetAttribute("name",node);
            if( (group_ptr == NULL) || (group_ptr->name != node) ){
                group_ptr = SNodeGroupPtr(new(SNodeGroup));
                group_ptr->name = node;
                group_ptr->from = ncpus;
                group_ptr->to = ncpus;
                group_ptr->ijob = NULL;
                groups.push_back(group_ptr);
            } else {
                group_ptr->to = ncpus;
            }
            ncpus++;
        }
    } else {
        while( (p_jele = J.GetNextChildElement("ijob")) != NULL ){
            int ncpus = 0;
            p_jele->GetAttribute("ncpus",ncpus);
            for(int i=0; i < ncpus; i++){
                p_sele = I.GetNextChildElement("node");
                if( p_sele == NULL ){
                    ES_ERROR("inconsistency between nodes and ijobs");
                    return(false);
                }
                p_sele->GetAttribute("name",node);
                if( (group_ptr == NULL) || (group_ptr->name != node) || (group_ptr->ijob != p_jele) ){
                    group_ptr = SNodeGroupPtr(new(SNodeGroup));
                    if( group_ptr->name != node) ncpus = 0;
                    group_ptr->name = node;
                    group_ptr->from = ncpus;
                    group_ptr->to = ncpus;
                    group_ptr->ijob = p_jele;
                    groups.push_back(group_ptr);
                } else {
                    group_ptr->to = ncpus;
                }
                ncpus++;
            }
        }
    }

    list<SNodeGroupPtr>::iterator git = groups.begin();
    list<SNodeGroupPtr>::iterator gie = groups.end();

    while( git != gie ){
        group_ptr = *git;
        string ipath;
        string workdir;
        string mainnode;

        if( group_ptr->ijob != NULL ){
            group_ptr->ijob->GetAttribute("path",ipath);
            group_ptr->ijob->GetAttribute("workdir",workdir);
            group_ptr->ijob->GetAttribute("mainnode",mainnode);
        }
        if( ! ipath.empty() ){
            sout << left << "<blue>>>> ";
            sout << setw(12) << ipath << " ";
            if( ! mainnode.empty() ){
                sout << mainnode << ":" << workdir;
            }
            sout << "</blue>" << right << endl;
        }

        sout << "CPU " << setfill('0') << setw(4) << group_ptr->from;
        if( group_ptr->from == group_ptr->to ) {
            sout << setfill(' ') << "         : ";
        } else {
            sout << "-" << setw(4) << group_ptr->to << setfill(' ') << "    : ";
        }
        stringstream str;
        str << group_ptr->name;
        if( (group_ptr->to - group_ptr->from) > 0 ){
            str << " (" << group_ptr->to - group_ptr->from + 1 << ")";
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

    list<SNodeGroupPtr> groups;
    SNodeGroupPtr       group_ptr;
    CSmallString        node;

    int ngpus = 0;
    while( (p_sele = I.GetNextChildElement("gpu")) != NULL ){
        p_sele->GetAttribute("name",node);
        if( (group_ptr == NULL) || (group_ptr->name != node) ){
            group_ptr = SNodeGroupPtr(new(SNodeGroup));
            group_ptr->name = node;
            group_ptr->from = ngpus;
            group_ptr->to = ngpus;
            group_ptr->ijob = NULL;
            groups.push_back(group_ptr);
        } else {
            group_ptr->to = ngpus;
        }
        ngpus++;
    }

    list<SNodeGroupPtr>::iterator git = groups.begin();
    list<SNodeGroupPtr>::iterator gie = groups.end();

    while( git != gie ){
        group_ptr = *git;

        if( group_ptr->ijob != NULL ){
            ES_ERROR("ijobs and gpus are not supported");
            return(false);
        }

        sout << "GPU " << setfill('0') << setw(4) << group_ptr->from;
        if( group_ptr->from == group_ptr->to ) {
            sout << setfill(' ') << "         : ";
        } else {
            sout << "-" << setw(4) << group_ptr->to << setfill(' ') << "    : ";
        }
        stringstream str;
        str << group_ptr->name;
        if( (group_ptr->to - group_ptr->from) > 0 ){
            str << " (" << group_ptr->to - group_ptr->from + 1 << ")";
        }
        sout << setw(25) << left << str.str() << right << endl;
        git++;
    }

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
    CFileName path = GetItem("basic/jobinput","INF_INPUT_DIR");
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

const CSmallString CJob::GetInputDir(void)
{
    CSmallString rv = GetItem("basic/jobinput","INF_INPUT_DIR");
    return(rv);
}

//------------------------------------------------------------------------------

const CSmallString CJob::GetInputMachine(void)
{
    CSmallString rv = GetItem("basic/jobinput","INF_INPUT_MACHINE");
    return(rv);
}

//------------------------------------------------------------------------------

const CSmallString CJob::GetStorageDir(void)
{
    CSmallString rv = GetItem("specific/resources","INF_STORAGE_DIR");
    return(rv);
}

//------------------------------------------------------------------------------

const CSmallString CJob::GetStorageMachine(void)
{
    CSmallString rv = GetItem("specific/resources","INF_STORAGE_MACHINE");
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

const CSmallString CJob::GetBatchJobID(void)
{
    CSmallString id = GetItem("batch/job","INF_JOB_ID");
    CSmallString sv = GetItem("batch/job","INF_SERVER_NAME");
    return(id + "." + sv);
}

//------------------------------------------------------------------------------

const CSmallString CJob::GetJobKey(void)
{
    CSmallString rv = GetItem("basic/jobinput","INF_JOB_KEY");
    return(rv);
}

//------------------------------------------------------------------------------

const CSmallString CJob::GetWorkDirType(void)
{
    CSmallString rv = GetItem("specific/resources","INF_WORK_DIR_TYPE");
    return(rv);
}

//------------------------------------------------------------------------------

const CSmallString CJob::GetDataOut(void)
{
    CSmallString rv = GetItem("specific/resources","INF_DATAOUT");
    return(rv);
}

//------------------------------------------------------------------------------

const CFileName CJob::GetInfoutName(void)
{
    CFileName host = GetItem("specific/resources","INF_STORAGE_MACHINE");
    CFileName path = GetItem("specific/resources","INF_STORAGE_DIR");
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
    CSmallString tmp = GetItem("specific/resources","INF_NCPUS",true);
    if( tmp == NULL ){
        // be compatible wil older ABS versions ...
        // RT#216417
        tmp = GetItem("specific/resources","INF_NCPU");
    }
    return(tmp.ToInt());
}

//------------------------------------------------------------------------------

int CJob::GetNumOfGPUs(void)
{
    CSmallString tmp = GetItem("specific/resources","INF_NGPUS",true);
    if( tmp == NULL ){
        // be compatible wil older ABS versions ...
        // RT#216417
        tmp = GetItem("specific/resources","INF_NGPU");
    }
    return(tmp.ToInt());
}

//------------------------------------------------------------------------------

int CJob::GetNumOfNodes(void)
{
    CSmallString tmp = GetItem("specific/resources","INF_NNODES",true);
    if( tmp == NULL ){
        // be compatible wil older ABS versions ...
        // RT#216417
        tmp = GetItem("specific/resources","INF_NNODE");
    }
    return(tmp.ToInt());
}

//------------------------------------------------------------------------------

int CJob::GetJobExitCode(void)
{
    CSmallString tmp = GetItem("stop/result","INF_JOB_EXIT_CODE",true);
    if( tmp == NULL ){
        return(0);
    }
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

    if( ABSConfig.IsServerConfigured(GetServerName()) == false ){
        switch( GetJobInfoStatus() ){
            case EJS_SUBMITTED:
                BatchJobComment = "the job batch server is not available under the current site";
                return(EJS_INCONSISTENT);
            case EJS_RUNNING:
                BatchJobComment = "the job batch server is not available under the current site";
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
                BatchJobComment = "the job is in the submitted mode but the batch system shows it in the running mode";
                return(EJS_BOOTING);
            }
            if( BatchJobStatus == EJS_FINISHED ){
                BatchJobComment = "the job is in the submitted mode but the batch system shows it finished";
                return(EJS_ERROR);
            }
            BatchJobComment = "the job was not found in the batch system";
            return(EJS_ERROR);
        case EJS_RUNNING:
            if( BatchJobStatus == EJS_RUNNING ){
                return(EJS_RUNNING);
            }
            if( BatchJobStatus == EJS_FINISHED ){
                BatchJobComment = "the job is in the running mode but the batch system shows it finished (walltime or resources exceeded?)";
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
    CSmallString sumask = GetItem("specific/resources","INF_UMASK");
    mode_t umask = CUserUtils::GetUMaskMode(sumask);

    int mode = 0666;
    int fmode = (mode & (~ umask)) & 0777;
    chmod(keyname,fmode);

    CSmallString sgroup = GetItem("specific/resources","INF_USTORAGEGROUP");
    if( (sgroup != NULL) && (sgroup != "-disabled-") ){
        if( GetItem("specific/resources","INF_STORAGE_MACHINE_REALM_FOR_INPUT_MACHINE") != NULL ){
            sgroup << "@" << GetItem("specific/resources","INF_STORAGE_MACHINE_REALM_FOR_INPUT_MACHINE");
        }
        gid_t group = CUserUtils::GetGroupID(sgroup,false);

        int ret = chown(keyname,-1,group);
        if( ret != 0 ){
            CSmallString warning;
            warning << "unable to set owner and group of file '" << keyname << "' (" << ret << ")";
            ES_WARNING(warning);
        }
    }

    return(true);
}

//------------------------------------------------------------------------------

void CJob::PrintJobInfoCompactV4(std::ostream& sout,bool includepath,bool includecomment)
{

//    sout << "# ST    Job ID        User        Job Title         Queue      NCPUs NGPUs NNods Last change          Exit" << endl;
//    sout << "# -- ------------ ------------ --------------- --------------- ----- ----- ----- -------------------- ----" << endl;

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
        case EJS_BOOTING:
            sout << "<green>B</green> ";
            break;
        case EJS_RUNNING:
            sout << "<green>R</green> ";
            break;
        case EJS_FINISHED:
            if( GetJobExitCode() == 0 ){
                sout << "F ";
            } else {
                sout << "<red>FE</red>";
            }
            break;
        case EJS_KILLED:
            sout << "<red>KI</red>";
            break;
        case EJS_ERROR:
            sout << "<red>ER</red>";
            break;
        case EJS_MOVED:
            sout << "<cyan>M</cyan>";
            break;
        case EJS_INCONSISTENT:
            sout << "<red>IN</red>";
            break;
    }
    if( GetItem("submit/job","INF_JOB_ID",true) != NULL ){
    CSmallString id = GetItem("submit/job","INF_JOB_ID");
    CSmallString srv = GetItem("specific/resources","INF_SERVER_SHORT");
    string stmp(id);
    vector<string> items;
    split(items,stmp,is_any_of("."));
    if( items.size() >= 1 ){
        id = items[0];
    }
    id << srv;
    sout << " " << right << setw(12) << id;
    } else {
    sout << "             ";
    }

    CSmallString user = GetItem("specific/resources","INF_JOB_OWNER",true);
    if( user.GetLength() > 12 ){
        user = user.GetSubStringFromTo(0,11);
    }
    sout << " " << left << setw(12) << user;

    CSmallString title = GetItem("basic/jobinput","INF_JOB_TITLE");
    if( title.GetLength() > 15 ){
        title = title.GetSubStringFromTo(0,14);
    }
    sout << " " << setw(15) << title;
    CSmallString queue = GetItem("specific/resources","INF_QUEUE");
    if( queue.GetLength() > 15 ){
        queue = queue.GetSubStringFromTo(0,14);
    }
    sout << " " << setw(15) << left << queue;
    sout << right;
    sout << " " << setw(5) << GetItem("specific/resources","INF_NCPUS");
    sout << " " << setw(5) << GetItem("specific/resources","INF_NGPUS");
    sout << " " << setw(5) << GetItem("specific/resources","INF_NNODES");

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
        case EJS_MOVED:
            break;
        case EJS_PREPARED:
        case EJS_SUBMITTED:
        case EJS_BOOTING:
        case EJS_RUNNING:
            from_last_change = curr_time - last_change;
            sout << right << setw(25) << from_last_change.GetSTimeAndDay();
            break;
        case EJS_FINISHED:
        case EJS_KILLED:
            sout << right << setw(25) << last_change.GetSDateAndTime();
            break;
    }

    sout << endl;

    if( includepath ){
        if( IsJobDirLocal() ){
            sout << "                  <blue>> " << GetItem("basic/jobinput","INF_INPUT_DIR") << "</blue>" << endl;
        } else {
            sout << "                  <blue>> " << GetItem("basic/jobinput","INF_INPUT_MACHINE") << ":" << GetItem("basic/jobinput","INF_INPUT_DIR") << "</blue>" << endl;
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
                sout << "                  <red>" << GetJobBatchComment() << "</red>" << endl;
                break;
            case EJS_SUBMITTED:
            case EJS_BOOTING:
            case EJS_MOVED:
                sout << "                  <purple>" << GetJobBatchComment() << "</purple>" << endl;
                break;
            case EJS_RUNNING:         
                sout << "                  <green>" << GetItem("start/workdir","INF_MAIN_NODE");
                if( GetItem("specific/resources","INF_NNODES").ToInt() > 1 ){
                    sout << ",+";
                }
                sout << "</green>" << endl;
                break;
        }
    }
}

//------------------------------------------------------------------------------

void CJob::PrintJobQStatInfo(std::ostream& sout,bool includepath,bool includecomment,bool includeorigin)
{
//    sout << "#                                                                                                          Util[%]     " << endl;
//    sout << "# ST    Job ID        User        Job Title         Queue      NCPUs NGPUs NNods          Times            CPU Mem Exit" << endl;
//    sout << "# -- ------------ ------------ --------------- --------------- ----- ----- ----- ------------------------- --- --- ----" << endl;

    CSmallString id = GetItem("batch/job","INF_JOB_ID");
    CSmallString srv = GetItem("batch/job","INF_SERVER_NAME");
    CSmallString pid = id;

    if( srv == BatchServerName ){
        // normal job
        pid << ShortServerName;
        sout << "  " << right;
    } else {
        // moved job - get short server name
        CSmallString sn = BatchServers.GetShortServerName(srv);
        pid << sn;
        sout << "<cyan>></cyan> " << right;
    }

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
        case EJS_BOOTING:
        case EJS_RUNNING:
            sout << "<green>" << setw(2) << GetItem("batch/job","INF_JOB_STATE",true) << "</green> ";
            break;
        case EJS_MOVED:
            sout << "<cyan>" << setw(2) << GetItem("batch/job","INF_JOB_STATE",true) << "</cyan> ";
            break;
    }

    sout << right << setw(12) << pid << " ";

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
    tmp = GetItem("specific/resources","INF_NCPUS");
    sout << " " << right << setw(5) << tmp;
    tmp = GetItem("specific/resources","INF_NGPUS");
    sout << " " << right << setw(5) << tmp;
    tmp = GetItem("specific/resources","INF_NNODES");
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
            sout << setw(12) << right;
            string tmp1(diff.GetSTimeAndDay());
            trim(tmp1);
            sout << tmp1;
            }
            break;
        case EJS_BOOTING:
        case EJS_RUNNING:{
            CSmallTimeAndDate ptime(GetItem("batch/job","INF_START_TIME").ToLInt());
            CSmallTime diff = current_time - ptime;
            sout << setw(12) << right;
            string tmp1(diff.GetSTimeAndDay());
            trim(tmp1);
            sout << tmp1;
            sout << "/";
            CSmallTime wtime(GetItem("batch/job","INF_WALL_TIME").ToLInt());
            sout << setw(12) << right;
            string tmp2(wtime.GetSTimeAndDay());
            trim(tmp2);
            sout << tmp2;
            PrintJobQStatUtilization(sout);
            }
            break;
        case EJS_FINISHED:
        case EJS_MOVED:
        case EJS_KILLED:{
            CSmallTimeAndDate ptime(GetItem("batch/job","INF_FINISH_TIME").ToLInt());
            sout << ptime.GetSDateAndTime();
            }
            PrintJobQStatUtilization(sout);
            sout << " " << right << setw(4) << GetItem("batch/job","INF_EXIT_STATUS",true);
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
                sout << "                  <blue>> Interactive job </blue>" << endl;
        } else {
            if( IsJobDirLocal(true) ){
                sout << "                  <blue>> " << GetItem("basic/jobinput","INF_INPUT_DIR") << "</blue>" << endl;
            } else {
                sout << "                  <blue>> " << GetItem("basic/jobinput","INF_INPUT_MACHINE") << ":" << GetItem("basic/jobinput","INF_INPUT_DIR") << "</blue>" << endl;
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
                sout << "                  <red>" << GetJobBatchComment() << "</red>" << endl;
                break;
            case EJS_SUBMITTED:
            case EJS_MOVED:
                sout << "                  <purple>" << GetJobBatchComment() << "</purple>" << endl;
                break;
            case EJS_BOOTING:
            case EJS_RUNNING:
                sout << "                  <green>" << GetItem("start/workdir","INF_MAIN_NODE");
                if( GetItem("specific/resources","INF_NNODES").ToInt() > 1 ){
                    sout << ",+";
                }
                sout << "</green>" << endl;
                break;
        }
    }
    if( includeorigin ){
        if( srv == BatchServerName ){
            // normal job
            sout << "                  <cyan>" << id << "." << srv << "@" << srv << "</cyan>" << endl;
        } else {
            // moved job - get short server name
            sout << "                  <cyan>" << id << "." << srv << "@" << srv << " -> " << id << "." << srv << "@" << BatchServerName << "</cyan>" << endl;
        }
    }

}

//------------------------------------------------------------------------------

void CJob::PrintJobQStatUtilization(std::ostream& sout)
{
    CSmallString tmp;
    tmp = GetItem("batch/job","INF_UTIL_CPU",true);

    int ctil = 0;
    if( tmp != NULL ){
        ctil = tmp. ToInt();
    }
    if( (ctil > 110) || (ctil < 60) ){
        sout << "<red>";
    } else if( ctil < 80 ) {
        sout << "<yellow>";
    }
    sout << " " << right << setw(3) << tmp;
    if( (ctil > 110) || (ctil < 60) ){
        sout << "</red>";
    } else if( ctil < 80 ) {
        sout << "</yellow>";
    }

    // do not color mem utilization
    sout << " " << right << setw(3) << GetItem("batch/job","INF_UTIL_MEM",true);
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

void CJob::ArchiveRuntimeFiles(const CSmallString& sformat)
{
    // CLEANME

    CFileName job_dir = GetItem("basic/jobinput","INF_INPUT_DIR",true);
    if( job_dir == NULL ) return;
    if( CFileSystem::SetCurrentDir(job_dir) == false ) return;

    CSmallString whole_name = GetJobName();
    if( GetJobNameSuffix() != NULL ){
        whole_name += GetJobNameSuffix();
    }
    bool autorecovery = false;
    if( GetItem("basic/external","INF_EXTERNAL_FLAGS",true) == "-" ){
        // this indicates autorecovery jobs
        autorecovery = true;
    }

    // delete non-important runtime files
    // *.infex;*.nodes;*.gpus;*.mpinodes;*.infkey;*.vncid;*.vncpsw;*.kill;*.hwtopo;*.amsreg
    CSmallString cmd;
    cmd << "rm -f ";
    cmd << "'" << whole_name << ".infex' ";
    cmd << "'" << whole_name << ".nodes' ";
    cmd << "'" << whole_name << ".gpus' ";
    cmd << "'" << whole_name << ".mpinodes' ";
    cmd << "'" << whole_name << ".infkey' ";
    cmd << "'" << whole_name << ".vncid' ";
    cmd << "'" << whole_name << ".vncpsw' ";
    cmd << "'" << whole_name << ".kill' ";
    cmd << "'" << whole_name << ".hwtopo' ";
    cmd << "'" << whole_name << ".amsreg' ";
    // FUJ - FIXME
    system(cmd);

    // archive *.info *.stdout *.infout
    CFileName archive;
    archive = GetItem("basic/recycle","INF_ARCHIVE_DIR",false);
    if( archive == NULL ) return;

    CSmallString scurrent_stage;
    scurrent_stage = GetItem("basic/recycle","INF_RECYCLE_CURRENT",false);
    if( scurrent_stage == NULL ) return;

    int current_stage = scurrent_stage.ToInt();

    stringstream str;
    str << format(sformat) % current_stage;
    CFileName dest_name(str.str());

    archive = archive / dest_name;

    cmd = NULL;
    cmd << "mv ";
    cmd << "'" << whole_name << ".info' ";
    if( autorecovery ){
        cmd << "'" << archive << ".info-'";
    } else {
        cmd << "'" << archive << ".info'";
    }
    // FUJ - FIXME
    system(cmd);

    cmd = NULL;
    cmd << "mv ";
    cmd << "'" << whole_name << ".stdout' ";
    if( autorecovery ){
        cmd << "'" << archive << ".stdout-'";
    } else {
        cmd << "'" << archive << ".stdout'";
    }
    // FUJ - FIXME
    system(cmd);

    cmd = NULL;
    cmd << "mv ";
    cmd << "'" << whole_name << ".infout' ";
    if( autorecovery ){
        cmd << "'" << archive << ".infout-'";
    } else {
        cmd << "'" << archive << ".infout'";
    }
    // FUJ - FIXME
    system(cmd);
}

//------------------------------------------------------------------------------

void CJob::CleanRuntimeFiles(void)
{
    // CLEANME

    CFileName job_dir = GetItem("basic/jobinput","INF_INPUT_DIR",true);
    if( job_dir == NULL ) return;
    if( CFileSystem::SetCurrentDir(job_dir) == false ) return;

    CSmallString whole_name = GetJobName();
    if( GetJobNameSuffix() != NULL ){
        whole_name += GetJobNameSuffix();
    }

    // delete all runtime files
    // *.infex;*.nodes;*.gpus;*.mpinodes;*.infkey;*.vncid;*.vncpsw;*.kill;*info;*stdout;*infout;*.hwtopo;*.amsreg
    CSmallString cmd;
    cmd << "rm -f ";
    cmd << "'" << whole_name << ".infex' ";
    cmd << "'" << whole_name << ".nodes' ";
    cmd << "'" << whole_name << ".gpus' ";
    cmd << "'" << whole_name << ".mpinodes' ";
    cmd << "'" << whole_name << ".infkey' ";
    cmd << "'" << whole_name << ".vncid' ";
    cmd << "'" << whole_name << ".vncpsw' ";
    cmd << "'" << whole_name << ".kill' ";
    cmd << "'" << whole_name << ".info' ";
    cmd << "'" << whole_name << ".stdout' ";
    cmd << "'" << whole_name << ".infout' ";
    cmd << "'" << whole_name << ".hwtopo' ";
    cmd << "'" << whole_name << ".amsreg' ";
    // FUJ - FIXME
    system(cmd);
}

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

bool CJob::IsJobInCollectionFromBS(void)
{
    return(BatchVariables.count("INF_COLLECTION_ID") == 1);
}

//------------------------------------------------------------------------------

const std::string CJob::GetCollectionIDFromBS(void)
{
    return(BatchVariables["INF_COLLECTION_ID"]);
}

//------------------------------------------------------------------------------

const std::string CJob::GetCollectionNameFromBS(void)
{
    return(BatchVariables["INF_COLLECTION_NAME"]);
}

//------------------------------------------------------------------------------

const std::string CJob::GetCollectionPathFromBS(void)
{
    return(BatchVariables["INF_COLLECTION_PATH"]);
}


//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

bool CJob::SubmitJobFull(ostream& vout,vector<string>& args,
                         bool ignorertmf,bool allowallpath,bool expermode,
                         bool assumeyes, bool resubmitmode, bool verbose)
{
    if( args.size() < 2 ){
        RUNTIME_ERROR("not enough of arguments");
    }

    CJobPtr Job = JobList.CreateNewJob();

    // start job preparation
    Job->CreateHeader();

    // basic section
    Job->CreateBasicSection();
    Job->SetExternalOptions();

    CSmallString qa,jn,rs;
    qa = args[0];
    jn = args[1];

    // generate list of resources
    vector<string> argres;
    vector<string> allres;
    for(size_t i=2; i < args.size(); i++){
        string stm = args[i];
        split(argres,stm,is_any_of(","),boost::token_compress_on);
        allres.insert(allres.end(),argres.begin(),argres.end());
    }
    rs = join(allres,",");

    if( Job->SetArguments(qa,jn,rs) == false ) return(false);
    if( Job->CheckRuntimeFiles(vout,ignorertmf) == false ){
        CJobList jobs;
        jobs.InitByInfoFiles(".",false);
        if( jobs.GetNumberOfJobs() > 0 ){
            jobs.SortByPrepareDateAndTime();
            jobs.UpdateJobStatuses();
            vout << endl;
            vout << ">>> List of jobs from info files ..." << endl;
            jobs.PrintInfosCompact(vout,false,true);
        }

        vout << endl;
        vout << " <b><red>ERROR: Infinity runtime files were detected in the job input directory!</red></b>" << endl;
        vout << "" << endl;
        vout << "<b><red>        The presence of the runtime files indicates that another job</red></b>" << endl;
        vout << "<b><red>        has been started in this directory. Multiple job submission </red></b>" << endl;
        vout << "<b><red>        within the same directory is not permitted by the Infinity system.</red></b>" << endl;

        if( (jobs.GetNumberOfJobsFromBatchSys(EJS_SUBMITTED) > 0) || (jobs.GetNumberOfJobsFromBatchSys(EJS_RUNNING) > 0) ){
            vout << "" << endl;
            vout << "<blue>        At least one job is waiting or running in the batch system (state: Q, R),</blue>" << endl;
            vout << "<blue>        please use the pkill command to remove this job from the batch system first!</blue>" << endl;
        } else {
            if( (jobs.GetNumberOfJobs(EJS_INCONSISTENT) > 0) || (jobs.GetNumberOfJobs(EJS_ERROR) > 0) ){
                vout << "" << endl;
                vout << "<blue>        At least one job is in inconsistent or error state (state: IN, ER),</blue>" << endl;
                vout << "<blue>        thus it is potentially dangerous to blindly remove the runtime files via:</blue>" << endl;
                vout << "<blue>        premovertf</blue>" << endl;
            } else {
                vout << "" << endl;
                vout << "        If you really want to submit the job, you have to remove runtime" << endl;
                vout << "        files from the previous run. Please, be very sure that the previous job has " << endl;
                vout << "        been already terminated otherwise undefined behaviour can occur!" << endl;
                vout << "" << endl;
                vout << "        Type following to remove runtime files:" << endl;
                vout << "        <b>premovertf</b>" << endl;
            }
        }
        ES_TRACE_ERROR("runtime files detected");
        return(false);
    }

    // execute presubmit hook
    if( CJob::ExecPresubmitHook() == false ){
        vout << endl;
        vout << " <b><red>ERROR: Unable to execute the presubmit-hook (or .presubmit-hook) script!</red></b>" << endl;
        vout << " <b><red>       Aborting the job submission ...</red></b>" << endl;
        ES_TRACE_ERROR("presubmit hook failed");
        return(false);
    }

    ERetStatus retstat = Job->JobInput(vout,allowallpath,false);
    if( retstat == ERS_FAILED ){
        ES_TRACE_ERROR("unable to set job input");
        return(false);
    }
    if( retstat == ERS_TERMINATE ){
        return(true);
    }

    vout << endl;
    Job->PrintBasicV4(vout);

    // resources
    if( Job->DecodeResources(vout,expermode) == false ){
        ES_TRACE_ERROR("unable to decode resources");
        return(false);
    }
    Job->PrintResourcesV4(vout);

    // last job check
    if( Job->LastJobCheck(vout) == false ){
        ES_TRACE_ERROR("job submission was canceled by last check procedure");
        return(false);
    }

    // submit job
    if( Job->ShouldSubmitJob(vout,assumeyes) == false ){
        ES_TRACE_ERROR("job submission was canceled by an user");
        return(false);
    }

    // we need registry for the job submission
    // save AMS registry
    if( Job->SaveAMSRegistryWithPerms() == false ){
        ES_ERROR("unable to save ams registry");
        return(false);
    }


    // in resubmit mode - ignore collections
    if( Job->SubmitJob(vout,false,verbose,resubmitmode) == false ){
        ES_TRACE_ERROR("unable to submit job");
        return(false);
    }

    // save job info
    if( Job->SaveInfoFileWithPerms() == false ){
        ES_ERROR("unable to save job info file");
        return(false);
    }

    // fix me - race condition? - should it be before submit job?
    if( Job->SaveJobKey() == false ){
        ES_ERROR("unable to save job key file");
        return(false);
    }

    return(true);
}

//------------------------------------------------------------------------------

bool CJob::ExecPresubmitHook(void)
{
    CFileName presubmit_hook;

    if( CFileSystem::IsFile("presubmit-hook") ){
        presubmit_hook = "presubmit-hook";
    } else if( CFileSystem::IsFile(".presubmit-hook") ) {
        presubmit_hook = ".presubmit-hook";
    }

    if( presubmit_hook == NULL ) return(true);  // nothing to do

    // make the script executable
    mode_t mode = CFileSystem::GetPosixMode(presubmit_hook);
    mode_t fmode = (mode | 0100 ) & 0777;
    chmod(presubmit_hook,fmode);

    CFileName pwd = CJob::GetJobInputPath();
    presubmit_hook =  pwd / presubmit_hook;

    // execute script
    bool result = system(presubmit_hook) == 0;

    // dissable execution flag for the executable
    fmode = (mode & ~0111 ) & 0777;
    chmod(presubmit_hook,fmode);

    return(result);
}

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================



