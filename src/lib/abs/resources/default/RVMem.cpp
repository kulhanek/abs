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

#include <RVMem.hpp>
#include <CategoryUUID.hpp>
#include <ABSModule.hpp>

// -----------------------------------------------------------------------------

CComObject* RVMemCB(void* p_data);

CExtUUID        RVMemID(
                    "{MEM:45b8522a-e690-47ef-93b6-725bc2cb74f0}",
                    "mem");

CPluginObject   RVMemObject(&ABSPlugin,
                    RVMemID,RESOURCES_CAT,
                    RVMemCB);

// -----------------------------------------------------------------------------

CComObject* RVMemCB(void* p_data)
{
    CComObject* p_object = new CRVMem();
    return(p_object);
}

//------------------------------------------------------------------------------

using namespace std;

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

CRVMem::CRVMem(void)
    : CResourceValue(&RVMemObject)
{
}

//------------------------------------------------------------------------------

void CRVMem::TestValue(std::ostream& sout,bool& rstatus)
{

}

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

