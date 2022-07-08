// =============================================================================
// ABS - Advanced Batch System
// -----------------------------------------------------------------------------
//    Copyright (C) 2022 Petr Kulhanek, kulhanek@chemi.muni.cz
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

#include <RVRandomize.hpp>
#include <CategoryUUID.hpp>
#include <ABSModule.hpp>

// -----------------------------------------------------------------------------

CComObject* RVRandomizeCB(void* p_data);

CExtUUID        RVRandomizeID(
                    "{RV_RANDOMIZE:785b2888-80bd-47fe-bd57-a4b17f21a2f1}",
                    "randomize");

CPluginObject   RVRandomizeObject(&ABSPlugin,
                    RVRandomizeID,RESOURCES_CAT,
                    RVRandomizeCB);

// -----------------------------------------------------------------------------

CComObject* RVRandomizeCB(void* p_data)
{
    CComObject* p_object = new CRVRandomize();
    return(p_object);
}

//------------------------------------------------------------------------------

using namespace std;

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

CRVRandomize::CRVRandomize(void)
    : CResourceValue(&RVRandomizeObject)
{
}

//------------------------------------------------------------------------------

void CRVRandomize::PreTestValue(CResourceList* p_rl,std::ostream& sout,bool& rstatus)
{
    if( Value.IsInt() == false ){
        if( rstatus == true ) sout << endl;
        sout << "<b><red> ERROR: Illegal '" << Name << "' resource specification!" << endl;
        sout << "<b><red>        Integer number expected!</red></b>" << endl;
        rstatus = false;
        return;
    }
}

//------------------------------------------------------------------------------

void CRVRandomize::GetVariable(CSmallString& name, CSmallString& value)
{
    name = "INF_RANDOMIZE";
    value = Value;
}

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

