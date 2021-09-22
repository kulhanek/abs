// =============================================================================
//  ABS - Advanced Batch System
// -----------------------------------------------------------------------------
//    Copyright (C) 2014 Petr Kulhanek (kulhanek@chemi.muni.cz)
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

#include "LoopJobType.hpp"
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
#include <boost/algorithm/string/split.hpp>
#include <boost/algorithm/string/classification.hpp>
#include <sys/stat.h>

//------------------------------------------------------------------------------

using namespace std;
using namespace boost;
using namespace boost::algorithm;

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

CComObject* LoopJobTypeCB(void* p_data);

CExtUUID        LoopJobTypeID(
                    "{LOOP_JOB_TYPE:184000a2-3573-4a3a-a1fd-dcdf61641a83}",
                    "Loop Job Type",
                    "Submit jobs in a cycle");

CPluginObject   LoopJobTypeObject(&BasicJobTypesPlugin,
                    LoopJobTypeID,JOB_TYPE_CAT,
                    LoopJobTypeCB);

// -----------------------------------------------------------------------------

CComObject* LoopJobTypeCB(void* p_data)
{
    CComObject* p_object = new CLoopJobType();
    return(p_object);
}

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

CLoopJobType::CLoopJobType(void)
    : CJobType(&LoopJobTypeObject)
{
}

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

ERetStatus CLoopJobType::DetectJobType(CJob& job,bool& detected,std::ostream& sout)
{
    detected = false;

    CFileName job_file = job.GetItem("basic/arguments","INF_ARG_JOB");

    CSmallString job_type;
    if( PluginDatabase.FindObjectConfigValue(LoopJobTypeID,"_type",job_type) == false ){
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

    sout << endl;
    sout << "# ------------------------------------------------" << endl;
    sout << "# loop job summary" << endl;
    sout << "# ------------------------------------------------" << endl;

    CFileName storage;
    storage = "storage";  // hardcoded

    // check individual items
    //--------------------------------------------------------------------------
    CSmallString beg_index = GetKeyValue("start");
    if( beg_index == NULL) {
        sout << endl;
        sout << "<b><red> ERROR: Loop beginning (start) is not provided!</red></b>" << endl;
        return(ERS_FAILED);
    }
    int start = beg_index.ToInt();
    sout << "# Loop from (start)            : " << start << endl;

    //--------------------------------------------------------------------------
    CSmallString end_index = GetKeyValue("stop");
    if( end_index == NULL) {
        sout << endl;
        sout << "<b><red> ERROR: Loop end (stop) is not provided!</red></b>" << endl;
        return(ERS_FAILED);
    }
    int stop = end_index.ToInt();
    sout << "# Loop to (stop)               : " << stop << endl;

    if( stop < start ){
        sout << endl;
        sout << "<b><red> ERROR: Loop start must be greater or equal than stop value!</red></b>" << endl;
        return(ERS_FAILED);
    }

    //--------------------------------------------------------------------------
    CSmallString name_format =  GetKeyValue("format");
    if( name_format == NULL) {
        sout << endl;
        sout << "<b><red> ERROR: Data name format is not provided!</red></b>" << endl;
        return(ERS_FAILED);
    }
    sout << "# Data name format (format)    : " << name_format << endl;

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
        sout << "<b><red> ERROR: The restart name format is not valid!</red></b>" << endl;
        return(ERS_FAILED);
    }

    //--------------------------------------------------------------------------
    CSmallString restart_afix =  GetKeyValue("afix");
    if( restart_afix == NULL) {
        sout << endl;
        sout << "<b><red> ERROR: Restart name afix (afix) is not provided!</red></b>" << endl;
        return(ERS_FAILED);
    }
    sout << "# Restart file afix (afix)     : " << restart_afix << endl;

    //--------------------------------------------------------------------------
    CSmallString ustorage =  GetKeyValue("archive");
    if( ustorage != NULL ){
        storage = ustorage;
    }
    sout << "# Archive directory (archive)  : " << storage << endl;

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
        test_file_form = storage / name_format + restart_afix;
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
    sout << "# Current stage                : " << setw(4) << right << current << " (found restart: " << last_found << ")" << endl;
    } else {
    sout << "# Current stage                : " << setw(4) << right << current << endl;
    }

    if( current > stop ){
        sout << endl;
        sout << "<b><blue> INFO: The loop job processed all stages!</blue></b>" << endl;
        return(ERS_TERMINATE);
    }

    //--------------------------------------------------------------------------

    // complete setup
    detected = true;

    stringstream ssufix;
    ssufix << format("+%03d") % current;

    job.SetItem("basic/external","INF_EXTERNAL_NAME_SUFFIX",ssufix.str().c_str());
    job.SetItem("basic/jobinput","INF_JOB_NAME",job_file);
    job.SetItem("basic/jobinput","INF_JOB_TYPE",job_type);
    job.SetItem("basic/jobinput","INF_EXCLUDED_FILES",storage);

    job.SetItem("basic/recycle","INF_RECYCLE_START",start);
    job.SetItem("basic/recycle","INF_RECYCLE_STOP",stop);
    job.SetItem("basic/recycle","INF_RECYCLE_CURRENT",current);

    job.SetItem("basic/recycle","INF_LOOP_START",start);
    job.SetItem("basic/recycle","INF_LOOP_STOP",stop);
    job.SetItem("basic/recycle","INF_LOOP_CURRENT",current);
    job.SetItem("basic/recycle","INF_LOOP_FORMAT",name_format);
    job.SetItem("basic/recycle","INF_LOOP_RESTART_AFIX",restart_afix);
    job.SetItem("basic/recycle","INF_LOOP_ARCHIVE",storage);

    job.SetItem("basic/recycle","INF_ARCHIVE_DIR",storage);

    job.SetItem("basic/jobinput","INF_JOB_CHECK",LoopJobTypeID.GetFullStringForm());

    return(ERS_OK);
}

//------------------------------------------------------------------------------

bool CLoopJobType::CheckInputFile(CJob& job,std::ostream& sout)
{
    CFileName storage;
    storage = "storage";  // hardcoded

    CSmallString ustorage =  GetKeyValue("archive");
    if( ustorage != NULL ){
        storage = ustorage;
    }

    // fix access permissions
    CSmallString sumask = job.GetItem("specific/resources","INF_UMASK");
    mode_t umask = CUser::GetUMaskMode(sumask);

    int mode = 0777;
    int fmode = (mode & (~ umask)) & 0777;
    chmod(storage,fmode);

    CSmallString sgroup = job.GetItem("specific/resources","INF_USTORAGEGROUP");
    if( (sgroup != NULL) && (sgroup != "-disabled-") ){
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

bool CLoopJobType::CacheJobFile(const CSmallString& file)
{
    ifstream ifs;
    ifs.open(file);
    if( ! ifs ) {
        ES_ERROR("unable to open the job input file");
        return(false);
    }

    string line;
    while( getline(ifs,line) ){
        if( line.empty() == true ) continue;
        JobFileCached.push_back(line);
    }

    return(true);
}

//------------------------------------------------------------------------------

const std::string CLoopJobType::GetKeyValue(const std::string& key)
{
    std::vector<std::string>::iterator  it = JobFileCached.begin();
    std::vector<std::string>::iterator  ie = JobFileCached.end();

    while( it != ie ){
        string line = *it;

        // split line into words
        vector<string> words;
        split(words,line,is_any_of("\t ="),boost::token_compress_on);
        if( words.size() == 4 ){
            if( (words[0] == "#") && (words[1] == "INFINITY") && (words[2] == key) ){
                return(words[3]);
            }
        }

        it++;
    }

    return(string());
}

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

