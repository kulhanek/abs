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

#include "NWChemJobType.hpp"
#include "BasicJobTypesModule.hpp"
#include <CategoryUUID.hpp>
#include <FileName.hpp>
#include <Job.hpp>
#include <FileSystem.hpp>
#include <PluginDatabase.hpp>
#include <ErrorSystem.hpp>
#include <GlobalConfig.hpp>
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

CComObject* NWChemJobTypeCB(void* p_data);

CExtUUID        NWChemJobTypeID(
                    "{NWCHEM_JOB_TYPE:481e8667-8de2-498e-a6e1-ad9f427f537a}",
                    "NWChem Job Type",
                    "autodetection of NWChem jobs, update resource specification in the input file");

CPluginObject   NWChemJobTypeObject(&BasicJobTypesPlugin,
                    NWChemJobTypeID,JOB_TYPE_CAT,
                    NWChemJobTypeCB);

// -----------------------------------------------------------------------------

CComObject* NWChemJobTypeCB(void* p_data)
{
    CComObject* p_object = new CNWChemJobType();
    return(p_object);
}

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

CNWChemJobType::CNWChemJobType(void)
    : CJobType(&NWChemJobTypeObject)
{
}

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

ERetStatus CNWChemJobType::DetectJobType(CJob& job,bool& detected,std::ostream& sout)
{
    detected = false;

    CSmallString job_type;
    if( PluginDatabase.FindObjectConfigValue(NWChemJobTypeID,"_type",job_type) == false ){
        ES_ERROR("_type key is not provided");
        return(ERS_FAILED);
    }

    CFileName arg_job = job.GetItem("basic/arguments","INF_ARG_JOB");

    // is the job name a file with .nwc extension?
    if( arg_job.GetFileNameExt() != ".nwc" ) return(ERS_OK);

    detected = true;

    if( CFileSystem::IsFile(arg_job) == false ){
        sout << endl;
        sout << "<b><red> ERROR: The nwchem job was detected but the input file '" << arg_job << "' does not exist!</red></b>" << endl;
        return(ERS_FAILED);
    }

    // get module name
    CSmallString nmodule;
    if( PluginDatabase.FindObjectConfigValue(NWChemJobTypeID,"_module",nmodule) == false ){
        ES_ERROR("_module key is not provide");
        return(ERS_FAILED);
    }

    // is nwchem module loaded?
    if( GlobalConfig.IsModuleActive(nmodule) == false ){
        sout << endl;
        sout << "<b><red> ERROR: The nwchem job was detected but none nwchem module is active!</red></b>" << endl;
        sout << "<b><red>        Type 'module add nwchem' and resubmit the job.</red></b>" << endl;
        return(ERS_FAILED);
    }

    CFileName job_file = arg_job.GetFileNameWithoutExt();

    ofstream ofs(job_file);
    if( ! ofs ) {
        sout << endl;
        sout << "<b><red> ERROR: The nwchem job was detected but unable to open the job script '" << job_file << "' for writing!</red></b>" << endl;
        sout << "<b><red>        Remove the file and check if you have the write permission in the job directory.</red></b>" << endl;
        return(ERS_FAILED);
    }

    ofs << "#!/usr/bin/env infinity-env" << endl;
    ofs << "# ----------------------------------------------------------" << endl;
    ofs << "# Please do not remove above line as it protects you against" << endl;
    ofs << "# accident execution of this job script." << endl;
    ofs << "# ----------------------------------------------------------" << endl;
    ofs << endl;
    ofs << "# activate nwchem module -------------------------" << endl;
    ofs << "module add " << nmodule << endl;
    ofs << endl;
    ofs << "# start job --------------------------------------" << endl;
    ofs << "nwchem " << arg_job << endl;
    ofs << endl;
    ofs << "# clean ------------------------------------------" << endl;
    ofs << "rm -f core" << endl;
    ofs << endl;

    if( ! ofs ) {
        sout << endl;
        sout << "<b><red> ERROR: The nwchem job was detected but unable to write data to the job script '" << job_file << "'!</red></b>" << endl;
        sout << "<b><red>        Check if you have the write permission in the job directory.</red></b>" << endl;
        return(ERS_FAILED);
    }

    job.SetItem("basic/jobinput","INF_JOB_NAME",job_file);
    job.SetItem("basic/jobinput","INF_JOB_TYPE",job_type);
    job.SetItem("basic/jobinput","INF_JOB_CHECK",NWChemJobTypeID.GetFullStringForm());

    return(ERS_OK);
}

//------------------------------------------------------------------------------

bool CNWChemJobType::CheckInputFile(CJob& job,std::ostream& sout)
{
    // check if there is no more than one node request
    CSmallString snnodes = job.GetItem("specific/resources","INF_NNODE");
    int nnodes = snnodes.ToInt();

    if( nnodes > 1 ){
        sout << endl;
        sout << "<b><red> ERROR: The parallel execution of nwchem job is allowed only within</red></b>" << endl;
        sout << "<b><red>        a single computational node!</red></b>" << endl;
        sout << endl;
        sout << "<b><red>        The number of requested computational nodes : " << nnodes << "</red></b>" << endl;
        return(false);
    }

    CSmallString job_name = job.GetItem("basic/jobinput","INF_JOB_NAME");

    // check memory keyword
    CResourceList res_list;
    CSmallString sres = job.GetItem("specific/resources","INF_RESOURCES");
    res_list.Parse(sres);
    long int mem = res_list.GetMemory();

    long int umem = GetMemory(job_name);

    if( abs(umem/1024/1024 - mem*90/100/1024/1024) > 2 ){
        sout << endl;
        sout << "<b><blue> WARNING: Inconsistency in the amount of requested memory was detected</blue></b>" << endl;
        sout << "<b><blue>          in the nwchem input file!</blue></b>" << endl;
        sout << endl;
        sout << "<b><blue>          The ammount of memory requested via psubmit command (90%) : " << setw(7) << mem*90/100/1024/1024 << " MB</blue></b>" << endl;
        sout << "<b><blue>          The ammount of memory requested in the nwchem input file  : " << setw(7) << umem/1024/1024 << " MB (via memory)</blue></b>" << endl;

        if( UpdateMemory(job_name,mem*90/100) == false ){
            sout << endl;
            sout << "<b><red> ERROR: Unable to save updated nwchem input file!</red></b>" << endl;
            return(false);
        }
    }

    if( abs(umem/1024/1024 - mem*90/100/1024/1024) > 2 ){
        sout << endl;
        sout << "<b><blue> WARNING: The input file was updated!</blue></b>" << endl;
    }

    return(true);
}

//------------------------------------------------------------------------------

bool CNWChemJobType::UpdateMemory(const CSmallString& name,long int mem)
{
    CSmallString full_name = name + ".nwc";
    ifstream ifs(full_name);

    string          line;
    vector<string>  lines;
    bool            transformed = false;

    while( getline(ifs,line) ){
        string iline = line;
        to_lower(iline);
        if( iline.find("memory ") == 0 ){
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
        ofs << endl;
        ofs << "memory " << left << mem/1024/1024 << " mb" << endl;
        ofs << endl;
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

long int CNWChemJobType::GetMemory(const CSmallString& name)
{
    CSmallString full_name = name + ".nwc";
    ifstream ifs(full_name);

    string          line;

    while( getline(ifs,line) ){
        string iline = line;
        to_lower(iline);
        if( iline.find("memory ") == 0 ){
            vector<string>  items;
            split(items,iline,is_any_of(" \t"),token_compress_on);
            if( items.size() == 3 ){
                long int mem = 0;
                string munit,mkey;
                stringstream sstr(iline);
                sstr >> mkey >> mem >> munit;
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

