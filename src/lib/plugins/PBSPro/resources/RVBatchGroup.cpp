// =============================================================================
// ABS - Advanced Batch System
// -----------------------------------------------------------------------------
//    Copyright (C) 2017 Petr Kulhanek, kulhanek@chemi.muni.cz
//    Copyright (C) 2011-2012 Petr Kulhanek, kulhanek@chemi.muni.cz
//    Copyright (C) 2001-2008 Petr Kulhanek, kulhanek@chemi.muni.cz
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

#include <RVBatchGroup.hpp>
#include <CategoryUUID.hpp>
#include <PBSProModule.hpp>
#include <ResourceList.hpp>
#include <ErrorSystem.hpp>
#include <User.hpp>
#include <FileSystem.hpp>
#include <Job.hpp>
#include <pbs_ifl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <grp.h>

// -----------------------------------------------------------------------------

CComObject* RVBatchGroupCB(void* p_data);

CExtUUID        RVBatchGroupID(
                    "{BATCH_GROUP:4eaddbda-7bf3-45e6-bd5f-dc1eec3754ee}",
                    "batchgroup");

CPluginObject   RVBatchGroupObject(&PBSProPlugin,
                    RVBatchGroupID,RESOURCES_CAT,
                    RVBatchGroupCB);

// -----------------------------------------------------------------------------

CComObject* RVBatchGroupCB(void* p_data)
{
    CComObject* p_object = new CRVBatchGroup();
    return(p_object);
}

//------------------------------------------------------------------------------

using namespace std;

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

CRVBatchGroup::CRVBatchGroup(void)
    : CResourceValue(&RVBatchGroupObject)
{
}

//------------------------------------------------------------------------------

void CRVBatchGroup::PreTestValue(CResourceList* p_rl,std::ostream& sout,bool& rstatus)
{
    CJob* p_job = p_rl->GetJob();

    // we need the job
    if( p_job == NULL ){
        ES_ERROR("job not set");
        rstatus = false;
        return;
    }

    // get group namespaces
    CSmallString input_machine_groupns = p_job->GetItem("specific/resources","INF_INPUT_MACHINE_GROUPNS");
    CSmallString storage_machine_groupns = p_job->GetItem("specific/resources","INF_STORAGE_MACHINE_GROUPNS");
    CSmallString batch_server_groupns = p_job->GetItem("specific/resources","INF_BATCH_SERVER_GROUPNS");

    if( Value == "current" ){
        if( input_machine_groupns != batch_server_groupns ){
            if( rstatus == true ) sout << endl;
            sout << "<b><red> ERROR: Illegal '" << Name << "' resource specification!" << endl;
            sout <<         "        The usage of 'group=current' requires that the input machine group namespace '" << input_machine_groupns;
            sout << "' is the same as the batch server group namespace '" << batch_server_groupns << "'!</red></b>" << endl;
            rstatus = false;
            return;
        }
        Value = User.GetEGroup();
    } else if( Value == "jobdir" ){
        if( storage_machine_groupns != batch_server_groupns ){
            if( rstatus == true ) sout << endl;
            sout << "<b><red> ERROR: Illegal '" << Name << "' resource specification!" << endl;
            sout <<         "        The usage of 'group=jobdir' requires that the storage machine group namespace '" << input_machine_groupns;
            sout << "' is the same as the batch server group namespace '" << batch_server_groupns << "'!</red></b>" << endl;
            rstatus = false;
            return;
        }
        struct stat info;
        string      storage_group;
        CSmallString pwd;
        if( CFileSystem::GetCurrentDir(pwd) == true ){
            if( stat(pwd, &info) == 0 ){
                struct group* p_gr = getgrgid(info.st_gid);
                if( p_gr ){
                    storage_group = p_gr->gr_name;
                }
            }
        }
        if( storage_group.empty() ){
            ES_WARNING("unable to determine the jobdir group");
            if( rstatus == true ) sout << endl;
            sout << "<b><blue> WARNING: Unable to determine value of the '" << Name << "' resource!" << endl;
            sout <<          "          Falling back to the user primary group!</blue></b>" << endl;
            rstatus = false;
            Value = User.GetEGroup();
            return;
        }

        // remove realm if present
        Value = storage_group.substr(0,storage_group.find("@"));

        // validate realm
        if( storage_group.find("@") != string::npos ){
            string realm = storage_group.substr(storage_group.find("@")+1,string::npos);
            if( p_job->GetItem("specific/resources","INF_STORAGE_MACHINE_REALM") != CSmallString(realm) ){
                sout << "<b><red> ERROR: Illegal '" << Name << "' resource specification!" << endl;
                sout <<         "        Inconsistent group realms: '" << realm << "'' vs '" <<  p_job->GetItem("specific/resources","INF_STORAGE_MACHINE_REALM") << "'!</red></b>" << endl;
                rstatus = false;
                return;
            }
        }
    }

    if( input_machine_groupns == batch_server_groupns ){
        // we can check if the value is correct
        if( User.IsInPosixGroup(Value) == false ){
            sout << "<b><red> ERROR: Illegal '" << Name << "' resource specification!" << endl;
            sout <<         "        The specified group '" << Value << "' is not one of user groups: " << User.GetPosixGroups() << "!</red></b>" << endl;
            rstatus = false;
            return;
        }
        return;
    }

    // do not check the group as we do not have any information about groups at the batch server
    return;
}

//------------------------------------------------------------------------------

void CRVBatchGroup::GetAttribute(CSmallString& name, CSmallString& resource, CSmallString& value)
{
    name = ATTR_g;
    resource = NULL;
    value = Value;
}

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

