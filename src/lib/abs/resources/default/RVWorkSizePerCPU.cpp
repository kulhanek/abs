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

#include <RVWorkSizePerCPU.hpp>
#include <CategoryUUID.hpp>
#include <ABSModule.hpp>

// -----------------------------------------------------------------------------

CComObject* RVWorkSizePerCPUCB(void* p_data);

CExtUUID        RVWorkSizePerCPUID(
                    "{WORK_SIZE_PER_CPU:1274fdf8-8a65-49e8-b410-491c32861e75}",
                    "worksizepercpu");

CPluginObject   RVWorkSizePerCPUObject(&ABSPlugin,
                    RVWorkSizePerCPUID,RESOURCES_CAT,
                    RVWorkSizePerCPUCB);

// -----------------------------------------------------------------------------

CComObject* RVWorkSizePerCPUCB(void* p_data)
{
    CComObject* p_object = new CRVWorkSizePerCPU();
    return(p_object);
}

//------------------------------------------------------------------------------

using namespace std;

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

CRVWorkSizePerCPU::CRVWorkSizePerCPU(void)
    : CResourceValue(&RVWorkSizePerCPUObject)
{

}

//------------------------------------------------------------------------------

void CRVWorkSizePerCPU::TestValue(std::ostream& sout,bool& rstatus)
{

}

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================
