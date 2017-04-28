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

#include <RVGroup.hpp>
#include <CategoryUUID.hpp>
#include <PBSProModule.hpp>
#include <ResourceList.hpp>
#include <pbs_ifl.h>

// -----------------------------------------------------------------------------

CComObject* RVGroupCB(void* p_data);

CExtUUID        RVGroupID(
                    "{GROUP:4eaddbda-7bf3-45e6-bd5f-dc1eec3754ee}",
                    "group");

CPluginObject   RVGroupObject(&PBSProPlugin,
                    RVGroupID,RESOURCES_CAT,
                    RVGroupCB);

// -----------------------------------------------------------------------------

CComObject* RVGroupCB(void* p_data)
{
    CComObject* p_object = new CRVGroup();
    return(p_object);
}

//------------------------------------------------------------------------------

using namespace std;

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

CRVGroup::CRVGroup(void)
    : CResourceValue(&RVGroupObject)
{
}

//------------------------------------------------------------------------------

void CRVGroup::TestValue(CResourceList* p_rl,std::ostream& sout,bool& rstatus)
{
// FIXME
//    if( Value == "current" ){
//        Value = User.GetEGroup();
//    } else if( Value == "jobdir" ){
//        Value = User.GetEGroup(); // fallback value
//        struct stat info;
//        CSmallString pwd;
//        if( CFileSystem::GetCurrentDir(pwd) == true ){
//            if( stat(pwd, &info) == 0 ){
//                struct group* p_gr = getgrgid(info.st_gid);
//                if( p_gr ){
//                    Value = p_gr->gr_name;
//                }
//            }
//        }
//    }

//    // do not check the group here as surrogate option need to obe processed first
//    return(true);
}

//------------------------------------------------------------------------------

void CRVGroup::GetAttribute(CSmallString& name, CSmallString& resource, CSmallString& value)
{
    name = ATTR_g;
    resource = NULL;
    value = Value;
}

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

