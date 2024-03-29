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

#include <RVGPUMem.hpp>
#include <CategoryUUID.hpp>
#include <ABSModule.hpp>
#include <ResourceList.hpp>

// -----------------------------------------------------------------------------

CComObject* RVGPUMemCB(void* p_data);

CExtUUID        RVGPUMemID(
                    "{GPU_MEM:ca7c3c46-3295-4562-a5f1-5969b2a3f23d}",
                    "gpu_mem");

CPluginObject   RVGPUMemObject(&ABSPlugin,
                    RVGPUMemID,RESOURCES_CAT,
                    RVGPUMemCB);

// -----------------------------------------------------------------------------

CComObject* RVGPUMemCB(void* p_data)
{
    CComObject* p_object = new CRVGPUMem();
    return(p_object);
}

//------------------------------------------------------------------------------

using namespace std;

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

CRVGPUMem::CRVGPUMem(void)
    : CResourceValue(&RVGPUMemObject)
{
}

//------------------------------------------------------------------------------

void CRVGPUMem::PreTestValue(CResourceList* p_rl,std::ostream& sout,bool& rstatus)
{
    if( TestSizeValue(sout,rstatus) == false ) return;
    long long size = GetSize(); // in kb
    if( size <= 1 ) {
        if( rstatus == true ) sout << endl;
        sout << "<b><red> ERROR: Illegal '" << Name << "' resource specification!" << endl;
        sout <<         "        Size must be larger than 1kb but " << GetSizeString() << " is specified!</red></b>" << endl;
        rstatus = false;
        return;
    }
}

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

