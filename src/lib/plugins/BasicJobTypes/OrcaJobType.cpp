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
#include <Cache.hpp>

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

    // get module name
    CSmallString omodule;
    if( PluginDatabase.FindObjectConfigValue(OrcaJobTypeID,"_module",omodule) == false ){
        ES_ERROR("_module key is not provide");
        return(ERS_FAILED);
    }

    CSmallString omodver;

    // is gaussian module loaded?
    if( AMSGlobalConfig.IsModuleActive(omodule) == true ){
        // get active module version
        AMSGlobalConfig.GetActiveModuleVersion(omodule,omodver);
    } else {
        // get default version of module
        if( Cache.LoadCache(false) == false) {
            ES_ERROR("unable to load AMS cache");
            return(ERS_FAILED);
        }
        CSmallString drch, dmode;
        CXMLElement* p_ele = Cache.GetModule(omodule);
        if( p_ele ){
            Cache.GetModuleDefaults(p_ele,omodver,drch,dmode);
        }
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
        ofs << "# ------------------------------------------------" << endl;
        ofs << endl;
        ofs << "# activate orca module ---------------------------" << endl;
        ofs << "module add " << omodule << ":" << omodver  << endl;
        ofs << endl;
        ofs << "# start job --------------------------------------" << endl;
        ofs << "orca " << arg_job << endl;
        ofs << endl;

        if( ! ofs ) {
            sout << endl;
            sout << "<b><red> ERROR: The orca job was detected but unable to write data to the job script '" << job_file << "'!</red></b>" << endl;
            sout << "<b><red>        Check if you have the write permission to the job input directory.</red></b>" << endl;
            return(ERS_FAILED);
        }
    }

    // include module version into job_type
    job_type << ":" << omodver;

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

    CSmallString smpislots = job.GetItem("specific/resources","INF_MPI_SLOTS_PER_NODE");
    int mpislots = smpislots.ToInt();

    if( mpislots > 0 ){
        // update number of CPUs according to mpislotspernode setup
        ncpus = nnodes*mpislots;
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

    // what amount of memory should be dedicated to the orca job
    int             perc = 80; // in %
    long long       mem = 0;
    stringstream    minfo;

    CSmallString appmem = job.GetItem("specific/resources","INF_APPMEM");
    long long    amem = CResourceValue::GetSize(appmem); // in kB
    if( (appmem != NULL) && (amem > 0) ){
        // provided as memory value
        mem = amem / ncpus / 1024; // in MB
        minfo << "appmem("<< appmem << ")/ncpus(" << ncpus <<") = " << mem << " MB";
    } else {
        // provided as fraction of memory request
        if( appmem != NULL ){
            perc = appmem.ToDouble()*100.0;
        }
        CSmallString smem = job.GetItem("specific/resources","INF_MEMORY");
        mem = CResourceValue::GetSize(smem); // in kb
        if( mem > 0 ){
            mem = mem * perc / ncpus / 1024 / 100; // in MB
            minfo << "appmem("<< perc << "%)*mem("<< smem << ")/ncpus(" << ncpus <<") = " << mem << " MB";
        } else {
            mem = 0;
        }
    }

    bool memory_changed = false;

    // check memory keyword
    if( ! minfo.str().empty() ){
        long long umem = GetMemory(job,job_name); // in MB
        if( abs(umem - mem) > 2 ){
            sout << endl;
            sout << "<b><blue> WARNING: Inconsistency in the amount of requested memory was detected</blue></b>" << endl;
            sout << "<b><blue>          in the orca input file!</blue></b>" << endl;
            sout << endl;
            sout << "<b><blue>          The ammount of memory requested via psubmit command    : " << setw(7) <<  mem << " MB (" << minfo.str() << ")</blue></b>" << endl;
            sout << "<b><blue>          The ammount of memory requested in the orca input file : " << setw(7) << umem << " MB (via %memcore)</blue></b>" << endl;

            if( UpdateMemory(job,job_name,mem) == false ){
                sout << endl;
                sout << "<b><red> ERROR: Unable to save updated orca input file (%memcore)!</red></b>" << endl;
                return(false);
            }
            memory_changed = true;
        }
    }

    if( memory_changed || (uncpus != ncpus) ){
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

    if( mem > 0 ){
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
        if( iline.find("%pal") != string::npos ){
            vector<string>  items;
            split(items,iline,is_any_of(" "));
            if( items.size() == 3 ){
                int ncpu = 0;
                stringstream sstr(items[2]);
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

