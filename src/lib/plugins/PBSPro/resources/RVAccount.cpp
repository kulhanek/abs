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

#include <RVAccount.hpp>
#include <CategoryUUID.hpp>
#include <PBSProModule.hpp>
#include <ResourceList.hpp>

// -----------------------------------------------------------------------------

CComObject* RVAccountCB(void* p_data);

CExtUUID        RVAccountID(
                    "{ACCOUNT:4d347675-abd8-4b0d-8672-ee35d0e8062e}",
                    "account");

CPluginObject   RVAccountObject(&PBSProPlugin,
                    RVAccountID,RESOURCES_CAT,
                    RVAccountCB);

// -----------------------------------------------------------------------------

CComObject* RVAccountCB(void* p_data)
{
    CComObject* p_object = new CRVAccount();
    return(p_object);
}

//------------------------------------------------------------------------------

using namespace std;

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

CRVAccount::CRVAccount(void)
    : CResourceValue(&RVAccountObject)
{
}

//------------------------------------------------------------------------------

void CRVAccount::GetAttribute(CSmallString& name, CSmallString& resource, CSmallString& value)
{
    name = Name;
    resource = NULL;
    value = Value;
}

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

