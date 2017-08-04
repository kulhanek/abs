// =============================================================================
//  ABS - Advanced Batch System
// -----------------------------------------------------------------------------
//    Copyright (C) 2012 Petr Kulhanek (kulhanek@chemi.muni.cz)
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

#include "PRecycleJobType.hpp"
#include "BasicJobTypesModule.hpp"
#include <CategoryUUID.hpp>
#include <FileName.hpp>
#include <Job.hpp>
#include <FileSystem.hpp>
#include <PluginDatabase.hpp>
#include <ErrorSystem.hpp>
#include <boost/format.hpp>
#include <FileName.hpp>
#include <iomanip>
#include <Cache.hpp>
#include <Utils.hpp>
#include <AMSGlobalConfig.hpp>
#include <sys/stat.h>

//------------------------------------------------------------------------------

using namespace std;
using namespace boost;

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

CComObject* PRecycleJobTypeCB(void* p_data);

CExtUUID        PRecycleJobTypeID(
                    "{PRECYCLE_JOB_TYPE:2b87fa99-9fbd-4ddb-9994-7c22ac9fd6d9}",
                    "PRecycle Job Type",
                    "checking of input file for recycle jobs using AMBER");

CPluginObject   PRecycleJobTypeObject(&BasicJobTypesPlugin,
                    PRecycleJobTypeID,JOB_TYPE_CAT,
                    PRecycleJobTypeCB);

// -----------------------------------------------------------------------------

CComObject* PRecycleJobTypeCB(void* p_data)
{
    CComObject* p_object = new CPRecycleJobType();
    return(p_object);
}

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

CPRecycleJobType::CPRecycleJobType(void)
    : CJobType(&PRecycleJobTypeObject)
{
}

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

ERetStatus CPRecycleJobType::DetectJobType(CJob& job,bool& detected,std::ostream& sout)
{
    detected = false;

    CFileName job_file = job.GetItem("basic/arguments","INF_ARG_JOB");

    CSmallString job_type;
    if( PluginDatabase.FindObjectConfigValue(PRecycleJobTypeID,"_type",job_type) == false ){
        ES_ERROR("_type key is not provided");
        return(ERS_FAILED);
    }

    // job file should be real file
    if( CFileSystem::IsFile(job_file) == false ){
        ES_ERROR("the job file is not a file");
        return(ERS_OK);
    }

    // cache file
    if( CacheJobFile(job_file) == false ){
        ES_ERROR("unable to cache job input file");
        return(ERS_OK);
    }

    CFileName storage;
    storage = "storage";  // hardcoded

    sout << endl;
    sout << "# ------------------------------------------------" << endl;
    sout << "# precycle job summary" << endl;
    sout << "# ------------------------------------------------" << endl;
    sout << "# Job script name     : " << job_file << endl;

    // check individual items
    //--------------------------------------------------------------------------
    CSmallString top_name = GetInputFileKeyValue("PRECYCLE_TOP");
    if( top_name == NULL ){
        sout << endl;
        sout << "<b><red> ERROR: The name of topology file (PRECYCLE_TOP) is not provided in the job script!</red></b>" << endl;
        return(ERS_FAILED);
    }
    sout << "# System topology     : " << top_name << endl;
    if( CFileSystem::IsFile(top_name) == false ) {
        sout << endl;
        sout << "<b><red> ERROR: The specified topology (PRECYCLE_TOP) is not a file or does not exist!</red></b>" << endl;
        return(ERS_FAILED);
    }

    //--------------------------------------------------------------------------
    CSmallString crd_name = GetInputFileKeyValue("PRECYCLE_CRD");
    if( crd_name == NULL ){
        sout << endl;
        sout << "<b><red> ERROR: The name of file with the initial coordinates (PRECYCLE_CRD) is not provided!</red></b>" << endl;
        return(ERS_FAILED);
    }
    sout << "# Initial coordinates : " << crd_name << endl;
    if( CFileSystem::IsFile(crd_name) == false ) {
        sout << endl;
        sout << "<b><red> ERROR: The specified initial coordinates (PRECYCLE_CRD) are not a file or do not exist!</red></b>" << endl;
        return(ERS_FAILED);
    }

    //--------------------------------------------------------------------------
    CSmallString ctr_name = GetInputFileKeyValue("PRECYCLE_CONTROL");
    if( ctr_name == NULL ){
        sout << endl;
        sout << "<b><red> ERROR: The name of control file (PRECYCLE_CONTROL) is not provided!</red></b>" << endl;
        return(ERS_FAILED);
    }
    sout << "# Control file        : " << ctr_name << endl;
    if( CFileSystem::IsFile(ctr_name) == false ) {
        sout << endl;
        sout << "<b><red> ERROR: The specified control file (PRECYCLE_CONTROL) is not a file or does not exist!</red></b>" << endl;
        return(ERS_FAILED);
    }



    //--------------------------------------------------------------------------
    CSmallString beg_index = GetInputFileKeyValue("PRECYCLE_START");
    if( beg_index == NULL ){
        sout << endl;
        sout << "<b><red> ERROR: The initial stage (PRECYCLE_START) is not provided!</red></b>" << endl;
        return(ERS_FAILED);
    }
    int start = beg_index.ToInt();
    sout << "# Starting stage      : " << setw(4) << right << start << endl;
    if( start < 0 ){
        sout << endl;
        sout << "<b><red> ERROR: The initial stage (PRECYCLE_START) must be larger than zero!</red></b>" << endl;
        return(ERS_FAILED);
    }

    //--------------------------------------------------------------------------
    CSmallString end_index = GetInputFileKeyValue("PRECYCLE_STOP");
    if( end_index == NULL ){
        sout << endl;
        sout << "<b><red> ERROR: The final stage (PRECYCLE_STOP) is not provided!</red></b>" << endl;
        return(ERS_FAILED);
    }
    int stop = end_index.ToInt();
    sout << "# Final stage         : " << setw(4) << right << stop << endl;
    if( stop <= start ){
        sout << endl;
        sout << "<b><red> ERROR: The final stage (PRECYCLE_STOP) must be larger than the initial stage!</red></b>" << endl;
        return(ERS_FAILED);
    }

    //--------------------------------------------------------------------------
    CSmallString sicycles = GetInputFileKeyValue("PRECYCLE_ICYCLES");
    if( sicycles == NULL ){
        sout << endl;
        sout << "<b><red> ERROR: The number of internal cycles (PRECYCLE_ICYCLES) is not provided!</red></b>" << endl;
        return(ERS_FAILED);
    }
    int icycles = sicycles.ToInt();
    sout << "# Internal cycles     : " << setw(4) << right << icycles << endl;
    if( icycles <= 0 ){
        sout << endl;
        sout << "<b><red> ERROR: The number of internal cycles (PRECYCLE_ICYCLES) must be larger zero!</red></b>" << endl;
        return(ERS_FAILED);
    }

    //--------------------------------------------------------------------------
    CSmallString comp_traj = GetInputFileKeyValue("PRECYCLE_COMPRESS_TRAJ");
    if( comp_traj != NULL ){
        sout << "# Compress trajectory : " << comp_traj << endl;
        if( (comp_traj != "gzip") && (comp_traj != "bzip2") ){
        sout << endl;
        sout << "<b><red> ERROR: Unsupported compression format (PRECYCLE_COMPRESS_TRAJ)!</red></b>" << endl;
        sout << "<b><red>        Supported formats: gzip, bzip2</red></b>" << endl;
        return(ERS_FAILED);
        }
    } else {
        sout << "# Compress trajectory : -no-" << endl;
    }

    //--------------------------------------------------------------------------
    CSmallString name_format = GetInputFileKeyValue("PRECYCLE_NAME_FORMAT");
    if( name_format == NULL ){
        sout << endl;
        sout << "<b><red> ERROR: The name format (PRECYCLE_NAME_FORMAT) is not provided!</red></b>" << endl;
        return(ERS_FAILED);
    }
    sout << "# Name format         : " << name_format << endl;

    bool format_ok = false;
    try{
        stringstream s1;
        int          n1 = 1;
        s1 << format(name_format) % n1;
        stringstream s2;
        int          n2 = 2;
        s2 << format(name_format) % n2;
        if( s1.str() != s2.str() ){
            format_ok = true;
        }
    } catch(...){
        format_ok = false;
    }
    if( format_ok == false ){
        sout << endl;
        sout << "<b><red> ERROR: The name format (PRECYCLE_NAME_FORMAT) is not valid!</red></b>" << endl;
        return(ERS_FAILED);
    }

    //--------------------------------------------------------------------------
    sout << "# Storage directory   : " << storage << endl;

    if( CFileSystem::IsFile(storage) ){
        sout << endl;
        sout << "<b><red> ERROR: '" << storage << "' must be a directory but it is a file!</red></b>" << endl;
        return(ERS_FAILED);
    }

    if( CFileSystem::IsDirectory(storage) == false ){
        CFileSystem::CreateDir(storage);
    }

    if( CFileSystem::IsDirectory(storage) == false ){
        sout << endl;
        sout << "<b><red> ERROR: '" << storage << "' must be a directory but it appears that it does not exist!</red></b>" << endl;
        return(ERS_FAILED);
    }

    //--------------------------------------------------------------------------
    // determine current stage
    int current = 0;
    CFileName last_found;
    for(int i=stop+1; i >= start; i--){
        CFileName test_file_form;
        test_file_form = storage / name_format + ".crd";
        stringstream test_file_str;
        test_file_str << format(test_file_form) % i;
        CFileName test_file = test_file_str.str().c_str();
        if( CFileSystem::IsFile(test_file) == true ){
            current = i;
            last_found = test_file;
            break;
        }
    }
    if( current == 0 ){
        current = 1;
    }

    if( last_found  != NULL ){
    sout << "# Current stage       : " << setw(4) << right << current << " (found restart: " << last_found << ")" << endl;
    } else {
    sout << "# Current stage       : " << setw(4) << right << current << endl;
    }

    if( current > stop ){
        sout << endl;
        sout << "<b><blue> INFO: The precycle job processed all stages!</blue></b>" << endl;
        return(ERS_TERMINATE);
    }

    // determine possible collision between input file and production files
    for(int i=stop+1; i >= start; i--){
        CFileName test_file_form;
        test_file_form = name_format + ".crd";
        stringstream test_file_str;
        test_file_str << format(test_file_form) % i;
        CFileName test_file = test_file_str.str().c_str();
        if( test_file == crd_name ){
            sout << endl;
            sout << "<b><red> ERROR: The name of initial coordinates (" << crd_name;
            sout << ") collides with the name of precycle run restart file (" << test_file << ")!</red></b>" << endl;
            return(ERS_FAILED);
        }
    }

    // -------------------------------------------------------------------------
    if( Cache.LoadCache() == false ){
        ES_ERROR("unable to load module cache");
        return(ERS_FAILED);
    }

    //--------------------------------------------------------------------------
    CSmallString md_module = GetInputFileKeyValue("MD_MODULE");
    sout << "# MD engine module    : " << md_module << endl;

    CSmallString mname,mver,march,mmode;
    CUtils::ParseModuleName(md_module,mname,mver,march,mmode);

    CXMLElement* p_mod = Cache.GetModule(mname);
    if( p_mod == NULL ){
        sout << endl;
        sout << "<b><red> ERROR: The specified MD engine module (" << mname;
        sout << ") is not available for the active site (" << AMSGlobalConfig.GetActiveSiteName() << ")!</red></b>" << endl;
        return(ERS_FAILED);
    }
    if( mver != NULL ){
        if( Cache.CheckModuleVersion(p_mod,mver) == false ){
            sout << endl;
            sout << "<b><red> ERROR: The specified MD engine module (" << mname << ":" << mver;
            sout << ") is not available for the active site (" << AMSGlobalConfig.GetActiveSiteName() << ")!</red></b>" << endl;
            return(ERS_FAILED);
        }
    }

    if( march != NULL ){
        sout << endl;
        sout << "<b><blue> WARNING: It is not wise to specify module architecture (" << md_module << ").</blue></b>" << endl;
        if( mmode != NULL ){
            sout << endl;
            sout << "<b><blue> WARNING: It is not wise to specify module mode (" << md_module << ").</blue></b>" << endl;
        }
    }

    //--------------------------------------------------------------------------
    CSmallString md_core = GetInputFileKeyValue("MD_CORE");
    sout << "# MD engine program   : " << md_core << endl;

    if( CFileSystem::IsFile("EXIT") == true ){
        sout << endl;
        sout << "<b><red> ERROR: The EXIT file found in the job directory!</red></b>" << endl;
        return(ERS_FAILED);
    }

    // complete setup
    detected = true;

    stringstream ssufix;
    ssufix << format("#%03d") % current;

    job.SetItem("basic/external","INF_EXTERNAL_NAME_SUFFIX",ssufix.str().c_str());
    job.SetItem("basic/jobinput","INF_JOB_NAME",job_file);
    job.SetItem("basic/jobinput","INF_JOB_TYPE",job_type);
    job.SetItem("basic/jobinput","INF_EXCLUDED_FILES",storage);

    job.SetItem("basic/recycle","INF_RECYCLE_START",start);
    job.SetItem("basic/recycle","INF_RECYCLE_STOP",stop);
    job.SetItem("basic/recycle","INF_RECYCLE_CURRENT",current);
    job.SetItem("basic/recycle","INF_ARCHIVE_DIR",storage);

    job.SetItem("basic/jobinput","INF_JOB_CHECK",PRecycleJobTypeID.GetFullStringForm());
    job.SetItem("basic/jobinput","INF_MD_MODULE",mname);

    return(ERS_OK);
}

//------------------------------------------------------------------------------

bool CPRecycleJobType::CheckInputFile(CJob& job,std::ostream& sout)
{
    CSmallString mname = job.GetItem("basic/jobinput","INF_MD_MODULE");

    //note: cache is already initialized

    // check if GPU resources are required by module
    if( Cache.DoesItNeedGPU(mname) == false ) return(true);

    // the module needs GPU, did we requested GPUs?
    CSmallString sngpus = job.GetItem("specific/resources","INF_NGPUS");
    int ngpus = sngpus.ToInt();

    if( ngpus == 0 ){
        sout << endl;
        sout << "<b><red> ERROR: The specified MD engine module (" << mname;
        sout << ") requires GPU resources but none were requested!</red></b>" << endl;
        return(false);
    }

    CFileName storage;
    storage = "storage";  // hardcoded

    // fix access permissions to storage dir
    CSmallString sumask = job.GetItem("specific/resources","INF_UMASK");
    mode_t umask = CUser::GetUMaskMode(sumask);

    int mode = 0777;
    int fmode = (mode & (~ umask)) & 0777;
    chmod(storage,fmode);

    CSmallString sgroup = job.GetItem("specific/resources","INF_USTORAGEGROUP");
    if( sgroup != NULL ){
        if( job.GetItem("specific/resources","INF_STORAGE_MACHINE_REALM_FOR_INPUT_MACHINE") != NULL ){
            sgroup << "@" << job.GetItem("specific/resources","INF_STORAGE_MACHINE_REALM_FOR_INPUT_MACHINE");
        }
        gid_t group = CUser::GetGroupID(sgroup,false);

        int ret = chown(storage,-1,group);
        if( ret != 0 ){
            CSmallString warning;
            warning << "unable to set owner and group of file '" << storage << "' (" << ret << ")";
            ES_WARNING(warning);
        }
    }

    return(true);
}

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

bool CPRecycleJobType::CacheJobFile(const CSmallString& file)
{
    ifstream ifs;
    ifs.open(file);
    if( ! ifs ) {
        ES_ERROR("unable to open the job input file");
        return(false);
    }

    string line;
    while( getline(ifs,line) ){
        if( line.find("#") == 0 ) continue;
        if( line.empty() == true ) continue;
        JobFileCached.push_back(line);
    }

    return(true);
}

//------------------------------------------------------------------------------

const CSmallString CPRecycleJobType::GetInputFileKeyValue(const CSmallString& key)
{
    std::vector<std::string>::iterator  it = JobFileCached.begin();
    std::vector<std::string>::iterator  ie = JobFileCached.end();

    while( it != ie ){
        string line = *it;
        if( line.find(key) != string::npos ){
            // extract key value
            unsigned int p1 = line.find_first_of("\"");
            if( p1 == string::npos ) return("");
            unsigned int p2 = line.find_last_of("\"");
            if( p2 == string::npos ) return("");
            if( p2 == p1 ) return("");
            if( p1+1 >= line.size() ) return("");
            return( line.substr(p1+1,p2-p1-1) );
        }

        it++;
    }

    return("");
}

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

