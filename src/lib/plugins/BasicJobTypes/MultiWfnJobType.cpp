// =============================================================================
//  ABS - Advanced Batch System
// -----------------------------------------------------------------------------
//    Copyright (C) 2015 Petr Kulhanek (kulhanek@chemi.muni.cz)
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

#include "MultiWfnJobType.hpp"
#include "BasicJobTypesModule.hpp"
#include <CategoryUUID.hpp>
#include <FileName.hpp>
#include <Job.hpp>
#include <FileSystem.hpp>
#include <PluginDatabase.hpp>
#include <ErrorSystem.hpp>
#include <boost/algorithm/string.hpp>
#include <boost/algorithm/string/split.hpp>
#include <fstream>
#include <string>
#include <iomanip>

//------------------------------------------------------------------------------

using namespace std;
using namespace boost;
using namespace boost::algorithm;

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

CComObject* MultiWfnJobTypeCB(void* p_data);

CExtUUID        MultiWfnJobTypeID(
                    "{MULTIWFN_JOB_TYPE:c1779cd7-4a82-4f91-a4ac-fac8e2f815a7}",
                    "MultiWfn Job Type",
                    "MultiWfn job - update resource specification (nthreads) in the default settings.ini file");

CPluginObject   MultiWfnJobTypeObject(&BasicJobTypesPlugin,
                    MultiWfnJobTypeID,JOB_TYPE_CAT,
                    MultiWfnJobTypeCB);

// -----------------------------------------------------------------------------

CComObject* MultiWfnJobTypeCB(void* p_data)
{
    CComObject* p_object = new CMultiWfnJobType();
    return(p_object);
}

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

CMultiWfnJobType::CMultiWfnJobType(void)
    : CJobType(&MultiWfnJobTypeObject)
{
}

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

ERetStatus CMultiWfnJobType::DetectJobType(CJob& job,bool& detected,std::ostream& sout)
{
    detected = false;

    CFileName job_file = job.GetItem("basic/arguments","INF_ARG_JOB");

    CSmallString job_type;
    if( PluginDatabase.FindObjectConfigValue(MultiWfnJobTypeID,"_type",job_type) == false ){
        ES_ERROR("_type key is not provided");
        return(ERS_FAILED);
    }

    // job file should be real file
    if( CFileSystem::IsFile(job_file) == false ){
        ES_ERROR("the job file is not a file");
        return(ERS_OK);
    }



    detected = true;
    job.SetItem("basic/jobinput","INF_JOB_NAME",job_file);
    job.SetItem("basic/jobinput","INF_JOB_TYPE",job_type);
    job.SetItem("basic/jobinput","INF_JOB_CHECK",MultiWfnJobTypeID.GetFullStringForm());

    return(ERS_OK);
}

//------------------------------------------------------------------------------

bool CMultiWfnJobType::CheckInputFile(CJob& job,std::ostream& sout)
{
    CFileName job_file = job.GetItem("basic/arguments","INF_ARG_JOB");

    // cache file
    if( CacheJobFile(job_file) == false ){
        ES_ERROR("unable to cache job input file");
        return(true);
    }

    // check if the input file contains correct number of CPUs
    CSmallString sncpus = job.GetItem("specific/resources","INF_NCPUS");
    int ncpus = sncpus.ToInt();
    int uncpus = GetNThreads();

    bool updated = false;

    if( uncpus != ncpus ){
        sout << endl;
        sout << "<b><blue> WARNING: Inconsistency in the number of requested CPUs was detected</blue></b>" << endl;
        sout << "<b><blue>          in the multiwfn configuration file!</blue></b>" << endl;
        sout << endl;
        sout << "<b><blue>          The number of CPUs requested via psubmit command         : " << ncpus << "</blue></b>" << endl;
        sout << "<b><blue>          The number of CPUs requested in the multiwfn config file : " << uncpus << " (via nthreads)</blue></b>" << endl;

        UpdateNThreads(ncpus);
        updated = true;
    }

    if( updated ){
        if( WriteJobFile(job_file) == false ){
            sout << endl;
            sout << "<b><red> ERROR: Unable to save updated multiwfn config file!</red></b>" << endl;
            return(false);
        }
    }

    return(true);
}

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

bool CMultiWfnJobType::CacheJobFile(const CSmallString& file)
{
    ifstream ifs;
    ifs.open(file);
    if( ! ifs ) {
        ES_ERROR("unable to open the job input file");
        return(false);
    }

    string line;
    while( getline(ifs,line) ){
        JobFileCached.push_back(line);
    }

    return(true);
}

//------------------------------------------------------------------------------

bool CMultiWfnJobType::WriteJobFile(const CSmallString& file)
{
    ofstream ofs;
    ofs.open(file);
    if( ! ofs ) {
        ES_ERROR("unable to open the job input file");
        return(false);
    }

    std::vector<std::string>::iterator  it = JobFileCached.begin();
    std::vector<std::string>::iterator  ie = JobFileCached.end();

    while( it != ie ){
        ofs << *it << endl;
        it++;
    }
    bool result = static_cast<bool>(ofs);
    ofs.close();

    return( result );
}

//------------------------------------------------------------------------------

void CMultiWfnJobType::UpdateNThreads(int nprocshared)
{
    std::vector<std::string>::iterator  it = JobFileCached.begin();
    std::vector<std::string>::iterator  ie = JobFileCached.end();

    while( it != ie ){
        string iline = *it;
        to_lower(iline);
        if( iline.find("nthreads") != string::npos ){
            stringstream str;
            str << "  nthreads= " << nprocshared << " // updated by INFINITY";
            *it = str.str();
        }
        it++;
    }
}

//------------------------------------------------------------------------------

int CMultiWfnJobType::GetNThreads(void)
{
    std::vector<std::string>::iterator  it = JobFileCached.begin();
    std::vector<std::string>::iterator  ie = JobFileCached.end();

    while( it != ie ){
        string iline = *it;
        to_lower(iline);
        if( iline.find("nthreads") != string::npos ){
            vector<string>  items;
            split(items,iline,is_any_of("="));
            if( items.size() == 2 ){
                int ncpu = 0;
                stringstream sstr(items[1]);
                sstr >> ncpu;
                return(ncpu);
            }
        }
        it++;
    }

    return(1);
}

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

