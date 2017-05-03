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
    if( (arg_job.GetFileNameExt() != ".com") || (arg_job.GetFileNameExt() != ".gjf") ) return(ERS_OK);

    detected = true;

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

    // is gaussian module loaded?
    if( AMSGlobalConfig.IsModuleActive(gmodule) == false ){
        sout << endl;
        sout << "<b><red> ERROR: The gaussian job was detected but none gaussian module is active!</red></b>" << endl;
        sout << "<b><red>        Type 'module add gaussian' and resubmit the job.</red></b>" << endl;
        return(ERS_FAILED);
    }

    CSmallString gmodver;
    AMSGlobalConfig.GetActiveModuleVersion(gmodule,gmodver);
    CSmallString gexec;
    if( PluginDatabase.FindObjectConfigValue(GaussianJobTypeID,gmodver,gexec) == false ){
        CSmallString error;
        error << "unable to find key '" << gmodver << "'";
        ES_ERROR(error);
        return(ERS_FAILED);
    }

    CFileName job_file = arg_job.GetFileNameWithoutExt();

    ofstream ofs(job_file);
    if( ! ofs ) {
        sout << endl;
        sout << "<b><red> ERROR: The gaussian job was detected but unable to open the job script '" << job_file << "' for writing!</red></b>" << endl;
        sout << "<b><red>        Remove the file and check if you have the write permission in the job directory.</red></b>" << endl;
        return(ERS_FAILED);
    }

    ofs << "#!/usr/bin/env infinity-env" << endl;
    ofs << "# ----------------------------------------------------------" << endl;
    ofs << "# Please do not remove above line as it protects you against" << endl;
    ofs << "# accidental execution of this job script." << endl;
    ofs << "# ----------------------------------------------------------" << endl;
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
    int uncpus = GetNProcShared(job_name);

    if( uncpus != ncpus ){
        sout << endl;
        sout << "<b><blue> WARNING: Inconsistency in the number of requested CPUs was detected</blue></b>" << endl;
        sout << "<b><blue>          in the gaussian input file!</blue></b>" << endl;
        sout << endl;
        sout << "<b><blue>          The number of CPUs requested via psubmit command        : " << ncpus << "</blue></b>" << endl;
        sout << "<b><blue>          The number of CPUs requested in the gaussian input file : " << uncpus << " (via %NProcShared)</blue></b>" << endl;

        if( UpdateNProcShared(job_name,ncpus) == false ){
            sout << endl;
            sout << "<b><red> ERROR: Unable to save updated gaussian input file!</red></b>" << endl;
            return(false);
        }
    }

    // check memory keyword
    long int mem = job.ResourceList.GetMemory();

    long int umem = GetMemory(job_name);

    if( abs(umem/1024/1024 - mem*90/100/1024/1024) > 2 ){
        sout << endl;
        sout << "<b><blue> WARNING: Inconsistency in the amount of requested memory was detected</blue></b>" << endl;
        sout << "<b><blue>          in the gaussian input file!</blue></b>" << endl;
        sout << endl;
        sout << "<b><blue>          The ammount of memory requested via psubmit command (90%)  : " << setw(7) << mem*90/100/1024/1024 << " MB</blue></b>" << endl;
        sout << "<b><blue>          The ammount of memory requested in the gaussian input file : " << setw(7) << umem/1024/1024 << " MB (via %Mem)</blue></b>" << endl;

        if( UpdateMemory(job_name,mem*90/100) == false ){
            sout << endl;
            sout << "<b><red> ERROR: Unable to save updated gaussian input file!</red></b>" << endl;
            return(false);
        }
    }

    if( ( abs(umem/1024/1024 - mem*90/100/1024/1024) > 2 ) || (uncpus != ncpus) ){
        sout << endl;
        sout << "<b><blue> WARNING: The input file was updated!</blue></b>" << endl;
    }

    return(true);
}

//------------------------------------------------------------------------------

bool CGaussianJobType::UpdateNProcShared(const CSmallString& name,int nprocshared)
{
    CSmallString full_name = name + ".com";
    ifstream ifs(full_name);

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

bool CGaussianJobType::UpdateMemory(const CSmallString& name,long int mem)
{
    CSmallString full_name = name + ".com";
    ifstream ifs(full_name);

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

    if( mem > 1024*1024 ){
        ofs << "%Mem=" << left << mem/1024/1024 << "MB" << endl;
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

int CGaussianJobType::GetNProcShared(const CSmallString& name)
{
    CSmallString full_name = name + ".com";
    ifstream ifs(full_name);

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

long int CGaussianJobType::GetMemory(const CSmallString& name)
{
    CSmallString full_name = name + ".com";
    ifstream ifs(full_name);

    string          line;

    while( getline(ifs,line) ){
        string iline = line;
        to_lower(iline);
        if( iline.find("%mem") != string::npos ){
            vector<string>  items;
            split(items,iline,is_any_of("="));
            if( items.size() == 2 ){
                long int mem = 0;
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

