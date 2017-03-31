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

#include <RVNCPUs.hpp>
#include <CategoryUUID.hpp>
#include <ABSModule.hpp>

// -----------------------------------------------------------------------------

CComObject* RVNCPUsCB(void* p_data);

CExtUUID        RVNCPUsID(
                    "{RV_EMAIL:87b41b5f-bdb5-4988-9130-5dff66a7a780}",
                    "ncpus");

CPluginObject   RVNCPUsObject(&ABSPlugin,
                    RVNCPUsID,RESOURCES_CAT,
                    RVNCPUsCB);

// -----------------------------------------------------------------------------

CComObject* RVNCPUsCB(void* p_data)
{
    CComObject* p_object = new CRVNCPUs();
    return(p_object);
}

//------------------------------------------------------------------------------

using namespace std;

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

CRVNCPUs::CRVNCPUs(void)
    : CResourceValue(&RVNCPUsObject)
{
}

//------------------------------------------------------------------------------

void CRVNCPUs::TestValue(CResourceList* p_rl,std::ostream& sout,bool& rstatus)
{
    if( TestNumberValue(sout,rstatus) == false ) return;
    long long size = GetNumber();
    if( size <= 0 ) {
        if( rstatus == true ) sout << endl;
        sout << "<b><red> ERROR: Illegal '" << Name << "' resource specification!" << endl;
        sout <<         "        At least one CPU must be requested but '" << size << "' is specified!</red></b>" << endl;
        rstatus = false;
    }
}

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

