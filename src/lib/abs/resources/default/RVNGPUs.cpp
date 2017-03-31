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

#include <RVNGPUs.hpp>
#include <CategoryUUID.hpp>
#include <ABSModule.hpp>

// -----------------------------------------------------------------------------

CComObject* RVNGPUsCB(void* p_data);

CExtUUID        RVNGPUsID(
                    "{NGPUS:97946b9a-96bd-499c-bc4b-ddc9e549a3ec}",
                    "ngpus");

CPluginObject   RVNNGUsObject(&ABSPlugin,
                    RVNGPUsID,RESOURCES_CAT,
                    RVNGPUsCB);

// -----------------------------------------------------------------------------

CComObject* RVNGPUsCB(void* p_data)
{
    CComObject* p_object = new CRVNGPUs();
    return(p_object);
}

//------------------------------------------------------------------------------

using namespace std;

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

CRVNGPUs::CRVNGPUs(void)
    : CResourceValue(&RVNNGUsObject)
{
}

//------------------------------------------------------------------------------

void CRVNGPUs::TestValue(CResourceList* p_rl,std::ostream& sout,bool& rstatus)
{
    if( TestNumberValue(p_rl,sout,rstatus) == false ) return;
    long long size = GetNumber();
    if( size < 0 ) {
        if( rstatus == true ) sout << endl;
        sout << "<b><red> ERROR: Illegal '" << Name << "' resource specification!" << endl;
        sout <<         "        Number of GPUs must be zero or positive number but '" << size << "' is specified!</red></b>" << endl;
        rstatus = false;
    }
}

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

