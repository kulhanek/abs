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

#include "TurbomoleJobType.hpp"
#include "BasicJobTypesModule.hpp"
#include <CategoryUUID.hpp>
#include <FileName.hpp>
#include <Job.hpp>
#include <FileSystem.hpp>
#include <PluginDatabase.hpp>
#include <ErrorSystem.hpp>
#include <ABSConfig.hpp>
#include <XMLElement.hpp>
#include <boost/algorithm/string/join.hpp>

//------------------------------------------------------------------------------

using namespace std;
using namespace boost;

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

CComObject* TurbomoleJobTypeCB(void* p_data);

CExtUUID        TurbomoleJobTypeID(
                    "{TURBOMOLE_JOB_TYPE:6ac8291a-4788-47dc-ae8b-67294fb117a4}",
                    "Turbomole Job Type",
                    "checking resources for TURBOMOLE job execution");

CPluginObject   TurbomoleJobTypeObject(&BasicJobTypesPlugin,
                    TurbomoleJobTypeID,JOB_TYPE_CAT,
                    TurbomoleJobTypeCB);

// -----------------------------------------------------------------------------

CComObject* TurbomoleJobTypeCB(void* p_data)
{
    CComObject* p_object = new CTurbomoleJobType();
    return(p_object);
}

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

CTurbomoleJobType::CTurbomoleJobType(void)
    : CJobType(&TurbomoleJobTypeObject)
{
}

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

ERetStatus CTurbomoleJobType::DetectJobType(CJob& job,bool& detected,std::ostream& sout)
{
    detected = false;

    CFileName job_file = job.GetItem("basic/arguments","INF_ARG_JOB");

    CSmallString job_type;
    if( PluginDatabase.FindObjectConfigValue(TurbomoleJobTypeID,"_type",job_type) == false ){
        ES_ERROR("_type key is not provided");
        return(ERS_FAILED);
    }

    // the turbomole job should have the 'control' file in the job directory
    if( CFileSystem::IsFile("control") == false ){
        return(ERS_OK);
    }

    // the control file should contain '$atoms' section
    ifstream ifs("control");
    if( ! ifs ) {
        return(ERS_OK);
    }

    // try to find the line with '$atoms' key
    string line;
    while( getline(ifs,line) ){
        if( line.find("$atoms") != string::npos ){
            detected = true;
            job.SetItem("basic/jobinput","INF_JOB_NAME",job_file);
            job.SetItem("basic/jobinput","INF_JOB_TYPE",job_type);
            job.SetItem("basic/jobinput","INF_JOB_CHECK",TurbomoleJobTypeID.GetFullStringForm());
            return(ERS_OK);
        }
    }

    return(ERS_OK);
}

//------------------------------------------------------------------------------

bool CTurbomoleJobType::CheckInputFile(CJob& job,std::ostream& sout)
{
    // if number of nodes is higher than 1 then sync mode must be jobdir
    CSmallString snnodes = job.GetItem("specific/resources","INF_NNODES");
    int nnodes = snnodes.ToInt();

    if( nnodes <= 1 ) return(true);

    // get workdir that support copy-shared and keep
    CXMLElement* p_ele = ABSConfig.GetWorkDirConfig();
    if( p_ele == NULL ){
        sout << endl;
        sout << "<b><red> ERROR: No workdir types configured (ask for support)!</red></b>" << endl;
        return(false);
    }

    // assembly list of types
    std::vector<std::string> workdirs;

    CXMLElement* p_wele = p_ele->GetFirstChildElement("workdir");
    while( p_wele != NULL ){
        std::string name;
        p_wele->GetAttribute("name",name);

        bool resok = false;
        CXMLElement* p_rele = p_wele->GetFirstChildElement("resource");
        while( p_rele != NULL ){
            CSmallString rname,rvalue;
            p_rele->GetAttribute("name",rname);
            p_rele->GetAttribute("value",rvalue);
            if( (rname == "datain") && ( (rvalue == "copy-shared") || (rvalue == "keep") ) ){
                resok = true;
            }
            p_rele = p_rele->GetNextSiblingElement("resource");
        }
        if( resok ){
            workdirs.push_back(name);
        }

        p_wele = p_wele->GetNextSiblingElement("workdir");
    }

    CSmallString datain = job.GetItem("specific/resources","INF_DATAIN");

    if( (datain != "copy-shared") && (datain != "keep") ){
        sout << endl;
        sout << "<b><red> ERROR: The parallel execution of turbomole job on more than one computational node</red></b>" << endl;
        sout << "<b><red>        is allowed only when the datain resource is copy-shared or keep!</red></b>" << endl;
        sout << endl;
        sout << "<b><red>        The number of requested computational nodes : " << nnodes << "</red></b>" << endl;
        sout << "<b><red>        Currently requested datain mode             : " << datain << "</red></b>" << endl;
        sout << "<b><red>        workdir types providing correct datain      : " << join(workdirs,", ") << "</red></b>" << endl;
        return(false);
    }

    return(true);
}

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

