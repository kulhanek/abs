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

#include "GaussianJobType.hpp"
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

CComObject* GaussianJobTypeCB(void* p_data);

CExtUUID        GaussianJobTypeID(
                    "{GAUSSIAN_JOB_TYPE:f8e7aa03-4c20-4d38-88d8-3e6dd9171430}",
                    "Gaussian Job Type",
                    "autodetection of Gaussian jobs, update resource specification in the input file");

CPluginObject   GaussianJobTypeObject(&BasicJobTypesPlugin,
                    GaussianJobTypeID,JOB_TYPE_CAT,
                    GaussianJobTypeCB);

// -----------------------------------------------------------------------------

CComObject* GaussianJobTypeCB(void* p_data)
{
    CComObject* p_object = new CGaussianJobType();
    return(p_object);
}

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

CGaussianJobType::CGaussianJobType(void)
    : CJobType(&GaussianJobTypeObject)
{
}

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

ERetStatus CGaussianJobType::DetectJobType(CJob& job,bool& detected,std::ostream& sout)
{
    detected = false;

    CSmallString job_type;
    if( PluginDatabase.FindObjectConfigValue(GaussianJobTypeID,"_type",job_type) == false ){
        ES_ERROR("_type key is not provided");
        return(ERS_FAILED);
    }

    CFileName arg_job = job.GetItem("basic/arguments","INF_ARG_JOB");

    // is the job name a file with .com or .gjf extension?
    if( arg_job.GetFileNameExt() == ".com" ){
        job.SetItem("basic/jobinput","INF_JOB_INPUT_EXT",".com");
        detected = true;
    } else if ( arg_job.GetFileNameExt() == ".gjf" ){
        job.SetItem("basic/jobinput","INF_JOB_INPUT_EXT",".gjf");
        detected = true;
    }
    if( detected == false ) return(ERS_OK);

    if( CFileSystem::IsFile(arg_job) == false ){
        sout << endl;
        sout << "<b><red> ERROR: The gaussian job was detected but the input file '" << arg_job << "' does not exist!</red></b>" << endl;
        return(ERS_FAILED);
    }

    // get module name
    CSmallString gmodule;
    if( PluginDatabase.FindObjectConfigValue(GaussianJobTypeID,"_module",gmodule) == false ){
        ES_ERROR("_module key is not provide");
        return(ERS_FAILED);
    }

    CSmallString gmodver;

    // is gaussian module loaded?
    if( AMSGlobalConfig.IsModuleActive(gmodule) == true ){
        // get active module version
        AMSGlobalConfig.GetActiveModuleVersion(gmodule,gmodver);
    } else {
        // get default version of module
        if( Cache.LoadCache(false) == false) {
            ES_ERROR("unable to load AMS cache");
            return(ERS_FAILED);
        }
        CSmallString drch, dmode;
        CXMLElement* p_ele = Cache.GetModule(gmodule);
        if( p_ele ){
            Cache.GetModuleDefaults(p_ele,gmodver,drch,dmode);
        }
    }

    CSmallString gexec;
    if( PluginDatabase.FindObjectConfigValue(GaussianJobTypeID,gmodver,gexec) == false ){
        sout << endl;
        sout << "<b><red> ERROR: The detected version of gaussian (" << gmodule << ":" << gmodver <<  ") is not supported!</red></b>" << endl;
        sout << "<b><red>        Application name was not found in the configuration database (contact support).</red></b>" << endl;
        return(ERS_FAILED);
    }

    CFileName job_file = arg_job.GetFileNameWithoutExt();

    // do not write the file if psanitize is used
    if( job.GetItem("basic/arguments","INF_SANITIZE_JOB",true) != "YES" ){
        ofstream ofs(job_file);
        if( ! ofs ) {
            sout << endl;
            sout << "<b><red> ERROR: The gaussian job was detected but unable to open the job script '" << job_file << "' for writing!</red></b>" << endl;
            sout << "<b><red>        Remove the file and check if you have the write permission in the job directory.</red></b>" << endl;
            return(ERS_FAILED);
        }

        ofs << "#!/usr/bin/env infinity-env" << endl;
        ofs << "# ------------------------------------------------" << endl;
        ofs << endl;
        ofs << "# activate gaussian module -----------------------" << endl;
        ofs << "module add " << gmodule << ":" << gmodver << endl;
        ofs << endl;
        ofs << "# start job --------------------------------------" << endl;
        ofs << gexec << " " << job_file << endl;
        ofs << endl;
        ofs << "# clean ------------------------------------------" << endl;
        ofs << "rm -f core" << endl;
        ofs << endl;

        if( ! ofs ) {
            sout << endl;
            sout << "<b><red> ERROR: The gaussian job was detected but unable to write data to the job script '" << job_file << "'!</red></b>" << endl;
            sout << "<b><red>        Check if you have the write permission to the job input directory.</red></b>" << endl;
            return(ERS_FAILED);
        }
    }

    // include module version into job_type
    job_type << ":" << gmodver;

    job.SetItem("basic/jobinput","INF_JOB_NAME",job_file);
    job.SetItem("basic/jobinput","INF_JOB_TYPE",job_type);
    job.SetItem("basic/jobinput","INF_JOB_CHECK",GaussianJobTypeID.GetFullStringForm());

    return(ERS_OK);
}

//------------------------------------------------------------------------------

bool CGaussianJobType::CheckInputFile(CJob& job,std::ostream& sout)
{
    // check if there is no more than one node request
    CSmallString snnodes = job.GetItem("specific/resources","INF_NNODES");
    int nnodes = snnodes.ToInt();

    if( nnodes > 1 ){
        sout << endl;
        sout << "<b><red> ERROR: The parallel execution of gaussian job is allowed only within</red></b>" << endl;
        sout << "<b><red>        a single computational node!</red></b>" << endl;
        sout << endl;
        sout << "<b><red>        The number of requested computational nodes : " << nnodes << "</red></b>" << endl;
        return(false);
    }

    // check if the input file contains correct number of CPUs
    CSmallString sncpus = job.GetItem("specific/resources","INF_NCPUS");
    int ncpus = sncpus.ToInt();

    CSmallString job_name = job.GetItem("basic/jobinput","INF_JOB_NAME");
    int uncpus = GetNProcShared(job,job_name);

    if( uncpus != ncpus ){
        sout << endl;
        sout << "<b><blue> WARNING: Inconsistency in the number of requested CPUs was detected</blue></b>" << endl;
        sout << "<b><blue>          in the gaussian input file!</blue></b>" << endl;
        sout << endl;
        sout << "<b><blue>          The number of CPUs requested via psubmit command        : " << ncpus << "</blue></b>" << endl;
        sout << "<b><blue>          The number of CPUs requested in the gaussian input file : " << uncpus << " (via %NProcShared)</blue></b>" << endl;

        if( UpdateNProcShared(job,job_name,ncpus) == false ){
            sout << endl;
            sout << "<b><red> ERROR: Unable to save updated gaussian input file (NProcShared)!</red></b>" << endl;
            return(false);
        }
    }

    // what amount of memory should be dedicated to the orca job
    int             perc = 80; // in %
    long long       mem = 0; // in MB
    stringstream    minfo;

    CSmallString appmem = job.GetItem("specific/resources","INF_APPMEM");
    long long    amem = CResourceValue::GetSize(appmem); // in kB
    if( (appmem != NULL) && (amem > 0) ){
        // provided as memory value
        mem = amem / 1024; // in MB
        minfo << "appmem("<< appmem << ") = " << mem << " MB";
    } else {
        // provided as fraction of memory request
        if( appmem != NULL ){
            perc = appmem.ToDouble()*100.0;
        }
        CSmallString smem = job.GetItem("specific/resources","INF_MEMORY");
        mem = CResourceValue::GetSize(smem); // in kb
        if( mem > 0 ){
            mem = mem * perc / 1024 / 100; // in MB
            minfo << "appmem("<< perc << "%)*mem("<< smem << ") = " << mem << " MB";
        } else {
            mem = 0;
        }
    }

    bool memory_changed = false;

    // check memory keyword
    CSmallString smem = job.GetItem("specific/resources","INF_MEMORY");
    if( smem != NULL ){
        long long umem = GetMemory(job,job_name); // in MB

        if( abs(umem - mem) > 2 ){
            sout << endl;
            sout << "<b><blue> WARNING: Inconsistency in the amount of requested memory was detected</blue></b>" << endl;
            sout << "<b><blue>          in the gaussian input file!</blue></b>" << endl;
            sout << endl;
            sout << "<b><blue>          The ammount of memory requested via psubmit command        : " << setw(7) << mem  << " MB (" << minfo.str() << ")</blue></b>" << endl;
            sout << "<b><blue>          The ammount of memory requested in the gaussian input file : " << setw(7) << umem << " MB (via %Mem)</blue></b>" << endl;

            if( UpdateMemory(job,job_name,mem) == false ){
                sout << endl;
                sout << "<b><red> ERROR: Unable to save updated gaussian input file (%Mem)!</red></b>" << endl;
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

bool CGaussianJobType::UpdateNProcShared(CJob& job,const CSmallString& name,int nprocshared)
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
        if( iline.find("%nprocshared") != string::npos ){
            transformed = true;
        } else {
            lines.push_back(line);
        }
    }
    ifs.close();

    // no change
    if( (transformed == false) && (nprocshared == 1) ) return(true);

    // save changed file
    ofstream ofs(full_name);

    if( nprocshared > 1 ){
        ofs << "%NProcShared=" << left << nprocshared << endl;
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

bool CGaussianJobType::UpdateMemory(CJob& job,const CSmallString& name,long long mem)
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
        if( iline.find("%mem") != string::npos ){
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
        ofs << "%Mem=" << left << mem << "MB" << endl;
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

int CGaussianJobType::GetNProcShared(CJob& job,const CSmallString& name)
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

long long CGaussianJobType::GetMemory(CJob& job,const CSmallString& name)
{
    CSmallString full_name = name + job.GetItem("basic/jobinput","INF_JOB_INPUT_EXT");
    ifstream ifs(full_name);
    if( ! ifs ) return(false);

    string          line;

    while( getline(ifs,line) ){
        string iline = line;
        to_lower(iline);
        if( iline.find("%mem") != string::npos ){
            vector<string>  items;
            split(items,iline,is_any_of("="));
            if( items.size() == 2 ){
                long long mem = 0;
                string munit;
                stringstream sstr(items[1]);
                sstr >> mem >> munit;
                to_lower(munit);
                if( munit == "b" )  mem = mem * 1;
                if( munit == "kb" ) mem = mem * 1024;
                if( munit == "mb" ) mem = mem * 1024 * 1024;
                if( munit == "gb" ) mem = mem * 1024 * 1024 * 1024;
                if( munit == "tb" ) mem = mem * 1024 * 1024 * 1024 * 1024;
                if( munit == "w" )  mem = mem * 8;
                if( munit == "kw" ) mem = mem * 8 * 1024;
                if( munit == "mw" ) mem = mem * 8 * 1024 * 1024;
                if( munit == "gw" ) mem = mem * 8 * 1024 * 1024 * 1024;
                if( munit == "tw" ) mem = mem * 8 * 1024 * 1024 * 1024 * 1024;
                // convert to MB
                mem = mem / 1024 / 1024;
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

