// =============================================================================
// ABS - Advanced Batch System
// -----------------------------------------------------------------------------
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

#include <RVEmail.hpp>
#include <CategoryUUID.hpp>
#include <ABSModule.hpp>

// -----------------------------------------------------------------------------

CComObject* RVEmailCB(void* p_data);

CExtUUID        RVEmailID(
                    "{RV_EMAIL:3f07ddbf-f19b-423f-80fb-2346493454e8}",
                    "e-mail resource");

CPluginObject   PBSProServerObject(&ABSPlugin,
                    RVEmailID,RESOURCES_CAT,
                    RVEmailCB);

// -----------------------------------------------------------------------------

CComObject* RVEmailCB(void* p_data)
{
    CComObject* p_object = new CRVEmail();
    return(p_object);
}

//------------------------------------------------------------------------------

using namespace std;

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

CRVEmail::CRVEmail(void)
    : CResourceValue(&PBSProServerObject)
{
    Name = "email";
}

//------------------------------------------------------------------------------

void CRVEmail::TestValue(std::ostream& sout,bool& rstatus)
{

}

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

