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

#include <RVPBSProGeneric.hpp>
#include <CategoryUUID.hpp>
#include <ABSModule.hpp>
#include <ResourceList.hpp>

// -----------------------------------------------------------------------------

CComObject* RVPBSProGenericCB(void* p_data);

CExtUUID        RVPBSProGenericID(
                    "{PBSPRO_GENERIC:5cf1351c-883d-4927-8348-6448c0baa888}",
                    "pbsprogeneric");

CPluginObject   RVPBSProGenericObject(&ABSPlugin,
                    RVPBSProGenericID,RESOURCES_CAT,
                    RVPBSProGenericCB);

// -----------------------------------------------------------------------------

CComObject* RVPBSProGenericCB(void* p_data)
{
    CComObject* p_object = new CRVPBSProGeneric();
    return(p_object);
}

//------------------------------------------------------------------------------

using namespace std;

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

CRVPBSProGeneric::CRVPBSProGeneric(void)
    : CResourceValue(&RVPBSProGenericObject)
{
}

//------------------------------------------------------------------------------

void CRVPBSProGeneric::GetAttribute(CSmallString& name, CSmallString& resource, CSmallString& value)
{
    name = "Resource_List";
    resource = Name;
    value = Value;
}

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

