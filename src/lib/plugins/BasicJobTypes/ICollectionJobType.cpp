// =============================================================================
//  ABS - Advanced Batch System
// -----------------------------------------------------------------------------
//    Copyright (C) 2013 Petr Kulhanek (kulhanek@chemi.muni.cz)
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

#include "ICollectionJobType.hpp"
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
#include "ICollectionJobTypeOptions.hpp"
#include <boost/algorithm/string/split.hpp>
#include <boost/algorithm/string/classification.hpp>

//------------------------------------------------------------------------------

using namespace std;
using namespace boost;
using namespace boost::algorithm;

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

CComObject* ICollectionJobTypeCB(void* p_data);

CExtUUID        ICollectionJobTypeID(
                    "{ICOLLECTION_JOB_TYPE:a8aa8420-1b82-46f9-a5eb-26b4f8fa27b9}",
                    "Internal Collection Job Type",
                    "collection of small jobs that are submitted as one big job");

CPluginObject   ICollectionJobTypeObject(&BasicJobTypesPlugin,
                    ICollectionJobTypeID,JOB_TYPE_CAT,
                    ICollectionJobTypeCB);

// -----------------------------------------------------------------------------

CComObject* ICollectionJobTypeCB(void* p_data)
{
    CComObject* p_object = new CICollectionJobType();
    return(p_object);
}

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

CICollectionJobType::CICollectionJobType(void)
    : CJobType(&ICollectionJobTypeObject)
{
}

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

ERetStatus CICollectionJobType::DetectJobType(CJob& job,bool& detected,std::ostream& sout)
{
    detected = false;

    CFileName job_file = job.GetItem("basic/arguments","INF_ARG_JOB");

    CSmallString job_type;
    if( PluginDatabase.FindObjectConfigValue(ICollectionJobTypeID,"_type",job_type) == false ){
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
    sout << "# icollection job summary" << endl;
    sout << "# ------------------------------------------------" << endl;
    sout << "# Master job script name : " << job_file << endl;
    sout << "#" << endl;

    int nkeys = GetNumberOfKeys("ijob");

    sout << "# Collection jobs specified by " << nkeys << " ijob entries" << endl;
    sout << "#" << endl;
    sout << "# ID Directory            JobName         NCPUs" << endl;
    sout << "# -- -------------------- --------------- -----" << endl;

    // enumerate via all ijobs commands
    int id = 1;
    for(int i=0; i < nkeys; i++){
        string iargs = GetKeyArguments("ijob",i);
        // parse arguments
        CIJobOptions ijobsopts;
        if( ijobsopts.ParseCmdLine(iargs) != SO_CONTINUE ){
            sout << endl;
            sout << "<b><red> ERROR: Unable to parse arguments for " << iargs << "!</red></b>" << endl;
            return(ERS_FAILED);
        }
        // expand
        if( ijobsopts.IsOptFromSet() ){
            string pdir = string(ijobsopts.GetArgDirectory());
            for(int j=ijobsopts.GetOptFrom(); j<=ijobsopts.GetOptTo(); j++ ){
                CIJob isubcmd;
                stringstream ssufix;
                try{
                    ssufix << format(pdir) % j;
                } catch(...){
                    sout << endl;
                    sout << "<b><red> ERROR: Unable to expand directory name " << pdir << "!</red></b>" << endl;
                    return(ERS_FAILED);
                }

                isubcmd.dir = ssufix.str();
                isubcmd.name = ijobsopts.GetArgJobName();
                isubcmd.ncpus = ijobsopts.GetArgNCPUs().ToInt();
                IJobs.push_back(isubcmd);
                sout << right << "# " << setw(2) << id << " ";
                sout << left << setw(20) << isubcmd.dir << " ";
                sout << left << setw(15) << isubcmd.name << " ";
                sout << right << setw(5) << isubcmd.ncpus << " ";
                sout << endl;
                id++;
            }
        } else {
            CIJob isubcmd;
            isubcmd.dir = ijobsopts.GetArgDirectory();
            isubcmd.name = ijobsopts.GetArgJobName();
            isubcmd.ncpus = ijobsopts.GetArgNCPUs().ToInt();
            IJobs.push_back(isubcmd);
            sout << right << "# " << setw(2) << id << " ";
            sout << left << setw(20) << isubcmd.dir << " ";
            sout << left << setw(15) << isubcmd.name << " ";
            sout << right << setw(5) << isubcmd.ncpus << " ";
            sout << endl;
        }
    }

    sout << "# ---------------------------------------------" << endl;

    if( IJobs.size() <= 0 ){
        sout << endl;
        sout << "<b><red> ERROR: No ijobs in the collection!</red></b>" << endl;
        return(ERS_FAILED);
    }

    int tcpus = 0;
    CFileName excfiles;
    excfiles = CFileName(IJobs[0].dir).GetFileDirectory();

    for(size_t i=0; i < IJobs.size(); i++){
        if( CFileName(IJobs[i].dir).GetFileDirectory() != excfiles ){
            sout << endl;
            sout << "<b><red> ERROR: The root directory for all jobs must be the same!</red></b>" << endl;
            sout << "<b><red>        The job " << i+1 << " has " << CFileName(IJobs[i].dir).GetFileDirectory();
            sout << " but it should be " << excfiles << "</red></b>" << endl;
            return(ERS_FAILED);
        }
        if( CFileSystem::IsDirectory(CFileName(IJobs[i].dir)) == false ){
            sout << endl;
            sout << "<b><red> ERROR: The job directory " << IJobs[i].dir << " not found for job " << i+1 << "!</red></b>" << endl;
            return(ERS_FAILED);
        }
        if( CFileSystem::IsFile(CFileName(IJobs[i].dir) / CFileName(IJobs[i].name)) == false ){
            sout << endl;
            sout << "<b><red> ERROR: The job script " << IJobs[i].name << " not found in directory of job " << i+1 << "!</red></b>" << endl;
            return(ERS_FAILED);
        }
        tcpus += IJobs[i].ncpus;
    }
    sout << "# Excluded directory   : " << excfiles << endl;
    sout << "# Total number of CPUs : " << tcpus << endl;

    // register ijobs
    for(size_t i=0; i < IJobs.size(); i++){
        job.RegisterIJob(IJobs[i].dir,IJobs[i].name,IJobs[i].ncpus);
    }

    // complete setup
    detected = true;

    job.SetItem("basic/jobinput","INF_JOB_NAME",job_file);
    job.SetItem("basic/jobinput","INF_JOB_TYPE",job_type);
    job.SetItem("basic/jobinput","INF_EXCLUDED_FILES",excfiles);

    job.SetItem("basic/jobinput","INF_JOB_CHECK",ICollectionJobTypeID.GetFullStringForm());
    job.SetItem("basic/jobinput","INF_ISUBMIT_TOT_NCPUS",tcpus);

    return(ERS_OK);
}

//------------------------------------------------------------------------------

bool CICollectionJobType::CheckInputFile(CJob& job,std::ostream& sout)
{
    CSmallString stncpus = job.GetItem("basic/jobinput","INF_ISUBMIT_TOT_NCPUS");
    int tncpus = stncpus.ToInt();

    CSmallString srcpus = job.GetItem("specific/resources","INF_NCPUS");
    int rncpus = srcpus.ToInt();

    if( tncpus != rncpus ) {
        sout << endl;
        sout << "<b><red> ERROR: Number of requested CPUs (" << rncpus << ") is not equal ";
        sout << "to total number of CPUs via ijobs commands (" << tncpus << ")!</red></b>" << endl;
        return(false);
    }

    // we do not support GPU
    CSmallString srgpus = job.GetItem("specific/resources","INF_NGPUS");
    int rgpus = srgpus.ToInt();

    if( rgpus > 0 ){
        sout << endl;
        sout << "<b><red> ERROR: You cannot request GPUs in the icollection job type!</red></b>" << endl;
        return(false);
    }

    return(true);
}

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

bool CICollectionJobType::CacheJobFile(const CSmallString& file)
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

int CICollectionJobType::GetNumberOfKeys(const std::string& key)
{
    std::vector<std::string>::iterator  it = JobFileCached.begin();
    std::vector<std::string>::iterator  ie = JobFileCached.end();

    int count = 0;

    while( it != ie ){
        string line = *it;
        // split line into words
        vector<string> words;
        split(words,line,is_any_of("\t "),boost::token_compress_on);
        if( words.size() >= 3 ){
            if( (words[0] == "#") && (words[1] == "INFINITY") && (words[2] == key) ){
                count++;
            }
        }

        it++;
    }

    return(count);
}

//------------------------------------------------------------------------------

const std::string CICollectionJobType::GetKeyArguments(const std::string& key,int id)
{
    std::vector<std::string>::iterator  it = JobFileCached.begin();
    std::vector<std::string>::iterator  ie = JobFileCached.end();

    int count = 0;

    stringstream args;

    args << key;

    while( it != ie ){
        string line = *it;
        // split line into words
        vector<string> words;
        split(words,line,is_any_of("\t "),boost::token_compress_on);
        if( words.size() >= 3 ){
            if( (words[0] == "#") && (words[1] == "INFINITY") && (words[2] == key) ){
                if( count == id ){
                    for(size_t i=3; i < words.size(); i++ ){
                        args << " " << words[i];
                    }
                    return(args.str());
                }
                count++;
            }
        }

        it++;
    }

    return(args.str());
}

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

