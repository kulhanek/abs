// =============================================================================
//  ABS - Advanced Batch System
// -----------------------------------------------------------------------------
//    Copyright (C) 2017 Petr Kulhanek (kulhanek@chemi.muni.cz)
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

#include "OrcaJobType.hpp"
#include "BasicJobTypesModule.hpp"
#include <CategoryUUID.hpp>
#include <FileName.hpp>
#include <Job.hpp>
#include <FileSystem.hpp>
#include <PluginDatabase.hpp>
#include <ErrorSystem.hpp>
#include <AMSGlobalConfig.hpp>
#include <ResourceList.hpp>
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

CComObject* OrcaJobTypeCB(void* p_data);

CExtUUID        OrcaJobTypeID(
                    "{ORCA_JOB_TYPE:37b32314-0500-417b-a5b8-0dedf32a1fde}",
                    "Orca Job Type",
                    "autodetection of orca jobs, update resource specification in the input file");

CPluginObject   OrcaJobTypeObject(&BasicJobTypesPlugin,
                    OrcaJobTypeID,JOB_TYPE_CAT,
                    OrcaJobTypeCB);

// -----------------------------------------------------------------------------

CComObject* OrcaJobTypeCB(void* p_data)
{
    CComObject* p_object = new COrcaJobType();
    return(p_object);
}

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

COrcaJobType::COrcaJobType(void)
    : CJobType(&OrcaJobTypeObject)
{
}

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

ERetStatus COrcaJobType::DetectJobType(CJob& job,bool& detected,std::ostream& sout)
{
    detected = false;

    CSmallString job_type;
    if( PluginDatabase.FindObjectConfigValue(OrcaJobTypeID,"_type",job_type) == false ){
        ES_ERROR("_type key is not provided");
        return(ERS_FAILED);
    }

    CFileName arg_job = job.GetItem("basic/arguments","INF_ARG_JOB");

    // is the job name a file with .orca extension?
    if( arg_job.GetFileNameExt() == ".orca" ){
        job.SetItem("basic/jobinput","INF_JOB_INPUT_EXT",".orca");
        detected = true;
    }
    if( detected == false ) return(ERS_OK);

    if( CFileSystem::IsFile(arg_job) == false ){
        sout << endl;
        sout << "<b><red> ERROR: The orca job was detected but the input file '" << arg_job << "' does not exist!</red></b>" << endl;
        return(ERS_FAILED);
    }

    CFileName job_file = arg_job.GetFileNameWithoutExt();

    // do not write the file if psanitize is used
    if( job.GetItem("basic/arguments","INF_SANITIZE_JOB",true) != "YES" ){
        ofstream ofs(job_file);
        if( ! ofs ) {
            sout << endl;
            sout << "<b><red> ERROR: The orca job was detected but unable to open the job script '" << job_file << "' for writing!</red></b>" << endl;
            sout << "<b><red>        Remove the file and check if you have the write permission in the job directory.</red></b>" << endl;
            return(ERS_FAILED);
        }

        ofs << "#!/usr/bin/env infinity-env" << endl;
        ofs << "# ----------------------------------------------------------" << endl;
        ofs << endl;
        ofs << "module add orca" << endl;
        ofs << endl;
        ofs << "orca " << job_file << endl;
        ofs << endl;

        if( ! ofs ) {
            sout << endl;
            sout << "<b><red> ERROR: The orca job was detected but unable to write data to the job script '" << job_file << "'!</red></b>" << endl;
            sout << "<b><red>        Check if you have the write permission to the job input directory.</red></b>" << endl;
            return(ERS_FAILED);
        }
    }

    job.SetItem("basic/jobinput","INF_JOB_NAME",job_file);
    job.SetItem("basic/jobinput","INF_JOB_TYPE",job_type);
    job.SetItem("basic/jobinput","INF_JOB_CHECK",OrcaJobTypeID.GetFullStringForm());

    return(ERS_OK);
}

//------------------------------------------------------------------------------

bool COrcaJobType::CheckInputFile(CJob& job,std::ostream& sout)
{
    // check if the input file contains correct number of CPUs
    CSmallString sncpus = job.GetItem("specific/resources","INF_NCPUS");
    int ncpus = sncpus.ToInt();

    CSmallString snnodes = job.GetItem("specific/resources","INF_NNODES");
    int nnodes = snnodes.ToInt();

    CResourceValuePtr p_rv;
    p_rv = job.ResourceList.FindResource("mpislotspernode");
    if( p_rv != NULL ){
        // update number of CPUs according to mpislotspernode setup
        ncpus = nnodes*p_rv->GetNumber();
    }

    CSmallString job_name = job.GetItem("basic/jobinput","INF_JOB_NAME");
    int uncpus = GetNProcs(job,job_name);

    if( uncpus != ncpus ){
        sout << endl;
        sout << "<b><blue> WARNING: Inconsistency in the number of requested CPUs was detected</blue></b>" << endl;
        sout << "<b><blue>          in the orca input file!</blue></b>" << endl;
        sout << endl;
        sout << "<b><blue>          The number of CPUs requested via psubmit command    : " << ncpus << "</blue></b>" << endl;
        sout << "<b><blue>          The number of CPUs requested in the orca input file : " << uncpus << " (via %pal nprocs)</blue></b>" << endl;

        if( UpdateNProcs(job,job_name,ncpus) == false ){
            sout << endl;
            sout << "<b><red> ERROR: Unable to save updated orca input file (%pal nprocs)!</red></b>" << endl;
            return(false);
        }
    }

    // what amount should be dedicated to the orca job
    int perc = 80; // in %
    p_rv = job.ResourceList.FindResource("appmem");
    if( p_rv != NULL ){
        perc = p_rv->GetFloatNumber()*100;
    }

    bool mem_changed = false;

    // check memory keyword
    CSmallString smem = job.GetItem("specific/resources","INF_MEMORY"); // in kb
    if( smem != NULL ){
        long long mem = CResourceValue::GetSize(smem)*perc/100/1024/ncpus;
        long long umem = GetMemory(job,job_name);

        if( abs(umem - mem) > 2 ){
            sout << endl;
            sout << "<b><blue> WARNING: Inconsistency in the amount of requested memory was detected</blue></b>" << endl;
            sout << "<b><blue>          in the orca input file!</blue></b>" << endl;
            sout << endl;
            sout << "<b><blue>          The ammount of memory requested via psubmit command (" << perc << "%)  : " << setw(7) << mem << " MB</blue></b>" << endl;
            sout << "<b><blue>          The ammount of memory requested in the orca input file : " << setw(7) << umem << " MB (via %memcore)</blue></b>" << endl;

            if( UpdateMemory(job,job_name,mem) == false ){
                sout << endl;
                sout << "<b><red> ERROR: Unable to save updated orca input file (%memcore)!</red></b>" << endl;
                return(false);
            }
            mem_changed = true;
        }
    }

    if( mem_changed || (uncpus != ncpus) ){
        sout << endl;
        sout << "<b><blue> WARNING: The input file was updated!</blue></b>" << endl;
    }

    return(true);
}

//------------------------------------------------------------------------------

bool COrcaJobType::UpdateNProcs(CJob& job,const CSmallString& name,int nprocs)
{
    CSmallString full_name = name + job.GetItem("basic/jobinput","INF_JOB_INPUT_EXT");
    ifstream ifs(full_name);
    if( ! ifs ) return(false);

    string          line;
    vector<string>  lines;
    bool            transformed = false;

    bool find_end = false;

    while( getline(ifs,line) ){
        string iline = line;
        if( find_end ) {
            if( iline.find("end") != string::npos ){
                find_end = false;
            }
        } else {
            to_lower(iline);
            if( iline.find("%pal") != string::npos ){
                transformed = true;
                find_end = true;
            } else {
                lines.push_back(line);
            }
        }
    }
    ifs.close();

    // no change
    if( (transformed == false) && (nprocs == 1) ) return(true);

    // save changed file
    ofstream ofs(full_name);

    if( nprocs > 1 ){
        ofs << "%pal nprocs " << nprocs << endl;
        ofs << "     end" << endl;
    }

    vector<string>::iterator    it = lines.begin();
    vector<string>::iterator    ie = lines.end();

    while( it != ie ){
        ofs << *it << endl;
        it++;
    }

    if( ! ofs ){
        return(false);
    }

    ofs.close();
    return(true);
}

//------------------------------------------------------------------------------

bool COrcaJobType::UpdateMemory(CJob& job,const CSmallString& name,long long mem)
{
    CSmallString full_name = name + job.GetItem("basic/jobinput","INF_JOB_INPUT_EXT");
    ifstream ifs(full_name);
    if( ! ifs ) return(false);

    string          line;
    vector<string>  lines;
    bool            transformed = false;

    while( getline(ifs,line) ){
        string iline = line;
        to_lower(iline);
        if( iline.find("%maxcore") != string::npos ){
            transformed = true;
        } else {
            lines.push_back(line);
        }
    }
    ifs.close();

    // no change
    if( (transformed == false) && (mem == 0) ) return(true);

    // save changed file
    ofstream ofs(full_name);

    if( mem > 1024*1024 ){
        ofs << "%maxcore " << mem << endl;
    }

    vector<string>::iterator    it = lines.begin();
    vector<string>::iterator    ie = lines.end();

    while( it != ie ){
        ofs << *it << endl;
        it++;
    }

    if( ! ofs ){
        return(false);
    }

    ofs.close();
    return(true);
}

//------------------------------------------------------------------------------

int COrcaJobType::GetNProcs(CJob& job,const CSmallString& name)
{
    CSmallString full_name = name + job.GetItem("basic/jobinput","INF_JOB_INPUT_EXT");
    ifstream ifs(full_name);
    if( ! ifs ) return(false);

    string          line;

    while( getline(ifs,line) ){
        string iline = line;
        to_lower(iline);
        if( iline.find("%nprocshared") != string::npos ){
            vector<string>  items;
            split(items,iline,is_any_of("="));
            if( items.size() == 2 ){
                int ncpu = 0;
                stringstream sstr(items[1]);
                sstr >> ncpu;
                return(ncpu);
            }
        }
    }
    ifs.close();

    return(1);
}

//------------------------------------------------------------------------------

long long COrcaJobType::GetMemory(CJob& job,const CSmallString& name)
{
    CSmallString full_name = name + job.GetItem("basic/jobinput","INF_JOB_INPUT_EXT");
    ifstream ifs(full_name);
    if( ! ifs ) return(false);

    string          line;

    while( getline(ifs,line) ){
        string iline = line;
        to_lower(iline);
        if( iline.find("%maxcore") != string::npos ){
            vector<string>  items;
            split(items,iline,is_any_of(" "));
            if( items.size() == 2 ){
                long long mem = 0;
                stringstream sstr(items[1]);
                sstr >> mem;
                return(mem);
            }
        }
    }
    ifs.close();

    return(0);
}

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

