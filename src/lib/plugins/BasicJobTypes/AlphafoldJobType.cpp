// =============================================================================
//  ABS - Advanced Batch System
// -----------------------------------------------------------------------------
//    Copyright (C) 2021 Petr Kulhanek (kulhanek@chemi.muni.cz)
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

#include "AlphafoldJobType.hpp"
#include "BasicJobTypesModule.hpp"
#include <CategoryUUID.hpp>
#include <FileName.hpp>
#include <Job.hpp>
#include <FileSystem.hpp>
#include <PluginDatabase.hpp>
#include <ErrorSystem.hpp>
#include <AMSGlobalConfig.hpp>
#include <ResourceList.hpp>
#include <fstream>
#include <string>
#include <iomanip>
#include <Cache.hpp>

//------------------------------------------------------------------------------

using namespace std;

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

CComObject* AlphafoldJobTypeCB(void* p_data);

CExtUUID        AlphafoldJobTypeID(
                    "{ALPHAFOLD:abfeb6d3-5e39-468f-99a0-d1812262b5c6}",
                    "Alphafold Job Type",
                    "autodetection of alphafold jobs");

CPluginObject   AlphafoldJobTypeObject(&BasicJobTypesPlugin,
                    AlphafoldJobTypeID,JOB_TYPE_CAT,
                    AlphafoldJobTypeCB);

// -----------------------------------------------------------------------------

CComObject* AlphafoldJobTypeCB(void* p_data)
{
    CComObject* p_object = new CAlphafoldJobType();
    return(p_object);
}

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

CAlphafoldJobType::CAlphafoldJobType(void)
    : CJobType(&AlphafoldJobTypeObject)
{
}

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

ERetStatus CAlphafoldJobType::DetectJobType(CJob& job,bool& detected,std::ostream& sout)
{
    detected = false;

    CSmallString job_type;
    if( PluginDatabase.FindObjectConfigValue(AlphafoldJobTypeID,"_type",job_type) == false ){
        ES_ERROR("_type key is not provided");
        return(ERS_FAILED);
    }

    CFileName arg_job = job.GetItem("basic/arguments","INF_ARG_JOB");

    // is the job name a file with .fasta extension?
    if( arg_job.GetFileNameExt() == ".fasta" ){
        job.SetItem("basic/jobinput","INF_JOB_INPUT_EXT",".fasta");
        detected = true;
    }
    if( detected == false ) return(ERS_OK);

    if( CFileSystem::IsFile(arg_job) == false ){
        sout << endl;
        sout << "<b><red> ERROR: The alphafold job was detected but the input file '" << arg_job << "' does not exist!</red></b>" << endl;
        return(ERS_FAILED);
    }

    // get module name
    CSmallString amodule;
    if( PluginDatabase.FindObjectConfigValue(AlphafoldJobTypeID,"_module",amodule) == false ){
        ES_ERROR("_module key is not provide");
        return(ERS_FAILED);
    }

    CSmallString amodver;

    // is alphafold module loaded?
    if( AMSGlobalConfig.IsModuleActive(amodule) == true ){
        // get active module version
        AMSGlobalConfig.GetActiveModuleVersion(amodule,amodver);
    } else {
        // get default version of module
        if( Cache.LoadCache(false) == false) {
            ES_ERROR("unable to load AMS cache");
            return(ERS_FAILED);
        }
        CSmallString drch, dmode;
        CXMLElement* p_ele = Cache.GetModule(amodule);
        if( p_ele ){
            Cache.GetModuleDefaults(p_ele,amodver,drch,dmode);
        }
    }

    CFileName job_file = arg_job.GetFileName();

    // do not write the file if psanitize is used
    if( job.GetItem("basic/arguments","INF_SANITIZE_JOB",true) != "YES" ){
        ofstream ofs(job_file);
        if( ! ofs ) {
            sout << endl;
            sout << "<b><red> ERROR: The alphafold job was detected but unable to open the job script '" << job_file << "' for writing!</red></b>" << endl;
            sout << "<b><red>        Remove the file and check if you have the write permission in the job directory.</red></b>" << endl;
            return(ERS_FAILED);
        }

        ofs << "#!/usr/bin/env infinity-env" << endl;
        ofs << "# ------------------------------------------------" << endl;
        ofs << endl;
        ofs << "# activate alphafold module ----------------------" << endl;
        ofs << "amsmodule add " << amodule << ":" << amodver << endl;
        ofs << endl;
        ofs << "# start job --------------------------------------" << endl;
        ofs << "alphafold -f \"" << job_file << "\"" << endl;
        ofs << endl;
        ofs << "# clean ------------------------------------------" << endl;
        ofs << "rm -f core" << endl;
        ofs << endl;

        if( ! ofs ) {
            sout << endl;
            sout << "<b><red> ERROR: The alphafold job was detected but unable to write data to the job script '" << job_file << "'!</red></b>" << endl;
            sout << "<b><red>        Check if you have the write permission to the job input directory.</red></b>" << endl;
            return(ERS_FAILED);
        }
    }

    // include module version into job_type
    job_type  << " [" << amodule << ":" << amodver << "]";

    job.SetItem("basic/jobinput","INF_JOB_NAME",job_file);
    job.SetItem("basic/jobinput","INF_JOB_TYPE",job_type);
    job.SetItem("basic/jobinput","INF_JOB_CHECK",AlphafoldJobTypeID.GetFullStringForm());

    return(ERS_OK);
}

//------------------------------------------------------------------------------

bool CAlphafoldJobType::CheckInputFile(CJob& job,std::ostream& sout)
{
    // check if there is no more than one node request
    CSmallString snnodes = job.GetItem("specific/resources","INF_NNODES");
    int nnodes = snnodes.ToInt();

    if( nnodes > 1 ){
        sout << endl;
        sout << "<b><red> ERROR: The parallel execution of alphafold job is allowed only within</red></b>" << endl;
        sout << "<b><red>        a single computational node!</red></b>" << endl;
        sout << endl;
        sout << "<b><red>        The number of requested computational nodes : " << nnodes << "</red></b>" << endl;
        return(false);
    }

    // warn for none or more than one GPU
    CSmallString sngpus = job.GetItem("specific/resources","INF_NGPUS");
    int ngpus = sngpus.ToInt();

    if( (ngpus == 0) || (ngpus > 1) ){
        sout << endl;
        sout << "<b><blue> WARNING: Alphafold optimally runs on 1 GPU but " << ngpus << " GPUs was requested!</blue></b>" << endl;
     }

    return(true);
}

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

