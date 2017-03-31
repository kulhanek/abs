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

#include <RVWalltime.hpp>
#include <CategoryUUID.hpp>
#include <ABSModule.hpp>
#include <ResourceList.hpp>

// -----------------------------------------------------------------------------

CComObject* RVWalltimeCB(void* p_data);

CExtUUID        RVWalltimeID(
                    "{WALLTIME:0bf8863e-67fa-4027-9c0e-41e42be50ecc}",
                    "walltime");

CPluginObject   RVWalltimeObject(&ABSPlugin,
                    RVWalltimeID,RESOURCES_CAT,
                    RVWalltimeCB);

// -----------------------------------------------------------------------------

CComObject* RVWalltimeCB(void* p_data)
{
    CComObject* p_object = new CRVWalltime();
    return(p_object);
}

//------------------------------------------------------------------------------

using namespace std;

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

CRVWalltime::CRVWalltime(void)
    : CResourceValue(&RVWalltimeObject)
{
}

//------------------------------------------------------------------------------

void CRVWalltime::TestValue(CResourceList* p_rl,std::ostream& sout,bool& rstatus)
{

}

//------------------------------------------------------------------------------

void CRVWalltime::GetAttribute(CSmallString& name, CSmallString& resource, CSmallString& value)
{
    name = Name;
    resource = "";
    value = Value;
}

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

