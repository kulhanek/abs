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

#include <RVMemPerCPU.hpp>
#include <CategoryUUID.hpp>
#include <ABSModule.hpp>

// -----------------------------------------------------------------------------

CComObject* RVMemPerCPUCB(void* p_data);

CExtUUID        RVMemPerCPUID(
                    "{MEM_PER_CPU:cf7326f2-1172-4e69-8356-ba40661ed1bb}",
                    "mempernode");

CPluginObject   RVMemPerCPUObject(&ABSPlugin,
                    RVMemPerCPUID,RESOURCES_CAT,
                    RVMemPerCPUCB);

// -----------------------------------------------------------------------------

CComObject* RVMemPerCPUCB(void* p_data)
{
    CComObject* p_object = new CRVMemPerCPU();
    return(p_object);
}

//------------------------------------------------------------------------------

using namespace std;

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

CRVMemPerCPU::CRVMemPerCPU(void)
    : CResourceValue(&RVMemPerCPUObject)
{
}

//------------------------------------------------------------------------------

void CRVMemPerCPU::TestValue(std::ostream& sout,bool& rstatus)
{

}

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

