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

#include "PEquiJobType.hpp"
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
#include <Utils.hpp>
#include <ModuleController.hpp>
#include <ModCache.hpp>
#include <SiteController.hpp>
#include <UserUtils.hpp>
#include <ModUtils.hpp>

//------------------------------------------------------------------------------

using namespace std;
using namespace boost;

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

CComObject* PEquiJobTypeCB(void* p_data);

CExtUUID        PEquiJobTypeID(
                    "{PEQUI_JOB_TYPE:8b5067e4-ff5b-4308-90e3-e34f05b40fdb}",
                    "PEqui Job Type",
                    "checking of input file for equilibration jobs using AMBER");

CPluginObject   PEquiJobTypeObject(&BasicJobTypesPlugin,
                    PEquiJobTypeID,JOB_TYPE_CAT,
                    PEquiJobTypeCB);

// -----------------------------------------------------------------------------

CComObject* PEquiJobTypeCB(void* p_data)
{
    CComObject* p_object = new CPEquiJobType();
    return(p_object);
}

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

CPEquiJobType::CPEquiJobType(void)
    : CJobType(&PEquiJobTypeObject)
{
}

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

ERetStatus CPEquiJobType::DetectJobType(CJob& job,bool& detected,std::ostream& sout)
{
    detected = false;

    CFileName job_file = job.GetItem("basic/arguments","INF_ARG_JOB");

    CSmallString job_type;
    if( PluginDatabase.FindObjectConfigValue(PEquiJobTypeID,"_type",job_type) == false ){
        ES_ERROR("_type key is not provided");
        return(ERS_FAILED);
    }

    // job file should be real file
    if( CFileSystem::IsFile(job_file) == false ){
        ES_ERROR("the job file is not a file");
        return(ERS_FAILED);
    }

    // cache file
    if( CacheJobFile(job_file) == false ){
        ES_ERROR("unable to cache job input file");
        return(ERS_FAILED);
    }

    CFileName storage;
    storage = "storage";  // hardcoded

    sout << endl;
    sout << "# ------------------------------------------------" << endl;
    sout << "# pequi job summary" << endl;
    sout << "# ------------------------------------------------" << endl;
    sout << "# Job script name        : " << job_file << endl;

    // check individual items
    //--------------------------------------------------------------------------
    CSmallString top_name = GetInputFileKeyValue("PEQUI_TOP");
    if( top_name == NULL ){
        sout << endl;
        sout << "<b><red> ERROR: The name of topology file (PEQUI_TOP) is not provided in the job script!</red></b>" << endl;
        return(ERS_FAILED);
    }
    sout << "# System topology        : " << top_name << endl;
    if( CFileSystem::IsFile(top_name) == false ) {
        sout << endl;
        sout << "<b><red> ERROR: The specified topology (PEQUI_TOP) is not a file or does not exist!</red></b>" << endl;
        return(ERS_FAILED);
    }

    //--------------------------------------------------------------------------
    CSmallString crd_name = GetInputFileKeyValue("PEQUI_CRD");
    if( crd_name == NULL ){
        sout << endl;
        sout << "<b><red> ERROR: The name of file with the initial coordinates (PEQUI_CRD) is not provided!</red></b>" << endl;
        return(ERS_FAILED);
    }
    sout << "# Initial coordinates    : " << crd_name << endl;
    if( CFileSystem::IsFile(crd_name) == false ) {
        sout << endl;
        sout << "<b><red> ERROR: The specified initial coordinates (PEQUI_CRD) are not a file or do not exist!</red></b>" << endl;
        return(ERS_FAILED);
    }

    //--------------------------------------------------------------------------
    // this is optional
    if( IsKeyDefined("NUM_OF_SOLUTE_RESIDUES") ){
        CSmallString sol_rnum = GetInputFileKeyValue("NUM_OF_SOLUTE_RESIDUES");
        if( sol_rnum == NULL ){
            sout << endl;
            sout << "<b><red> ERROR: The number of solute residues (NUM_OF_SOLUTE_RESIDUES) is not provided!</red></b>" << endl;
            return(ERS_FAILED);
        }
        sout << "# Num of solute residues : " << sol_rnum << endl;
        if( sol_rnum.IsInt() == false ) {
            sout << endl;
            sout << "<b><red> ERROR: The specified number of solute residues (NUM_OF_SOLUTE_RESIDUES) is not integer number!</red></b>" << endl;
            return(ERS_FAILED);
        }
    }

    // -------------------------------------------------------------------------
    ModuleController.LoadBundles(EMBC_SMALL);
    ModuleController.MergeBundles();

    //--------------------------------------------------------------------------
    CSmallString md_module = GetInputFileKeyValue("MD_MODULE");
    sout << "# MD engine module       : " << md_module << endl;

    CSmallString mname,mver,march,mmode;
    CModUtils::ParseModuleName(md_module,mname,mver,march,mmode);

    CXMLElement* p_mod = ModCache.GetModule(mname);
    if( p_mod == NULL ){
        sout << endl;
        sout << "<b><red> ERROR: The specified MD engine module (" << mname;
        sout << ") is not available for the active site (" << SiteController.GetActiveSite() << ")!</red></b>" << endl;
        return(ERS_FAILED);
    }
    if( mver != NULL ){
        if( ModCache.CheckModuleVersion(p_mod,mver) == false ){
            sout << endl;
            sout << "<b><red> ERROR: The specified MD engine module (" << mname << ":" << mver;
            sout << ") is not available for the active site (" << SiteController.GetActiveSite() << ")!</red></b>" << endl;
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

    // complete setup
    detected = true;

    job.SetItem("basic/jobinput","INF_JOB_NAME",job_file);
    job.SetItem("basic/jobinput","INF_JOB_TYPE",job_type);

    job.SetItem("basic/jobinput","INF_JOB_CHECK",PEquiJobTypeID.GetFullStringForm());
    job.SetItem("basic/jobinput","INF_MD_MODULE",mname);

    return(ERS_OK);
}

//------------------------------------------------------------------------------

bool CPEquiJobType::CheckInputFile(CJob& job,std::ostream& sout)
{
    CSmallString mname = job.GetItem("basic/jobinput","INF_MD_MODULE");

    //note: cache is already initialized

    // check if GPU resources are required by module

    // FIXME
    // if( Cache.DoesItNeedGPU(mname) == false ) return(true);

    // the module needs GPU, did we requested GPUs?
    CSmallString sngpus = job.GetItem("specific/resources","INF_NGPUS");
    int ngpus = sngpus.ToInt();

    if( ngpus == 0 ){
        sout << endl;
        sout << "<b><red> ERROR: The specified MD engine module (" << mname;
        sout << ") requires GPU resources but none were requested!</red></b>" << endl;
        return(false);
    }

    return(true);
}

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

bool CPEquiJobType::CacheJobFile(const CSmallString& file)
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

const CSmallString CPEquiJobType::GetInputFileKeyValue(const CSmallString& key)
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

//------------------------------------------------------------------------------

bool CPEquiJobType::IsKeyDefined(const CSmallString& key)
{
    std::vector<std::string>::iterator  it = JobFileCached.begin();
    std::vector<std::string>::iterator  ie = JobFileCached.end();

    while( it != ie ){
        string line = *it;
        if( line.find(key) != string::npos ){
            return(true);
        }

        it++;
    }

    return(false);
}

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

