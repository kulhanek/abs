// =============================================================================
// ABS - Advanced Batch System
// -----------------------------------------------------------------------------
//    Copyright (C) 2023 Petr Kulhanek, kulhanek@chemi.muni.cz
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

#include <RVInputData.hpp>
#include <CategoryUUID.hpp>
#include <ABSModule.hpp>
#include <XMLElement.hpp>
#include <ABSConfig.hpp>
#include <ResourceList.hpp>

// -----------------------------------------------------------------------------

CComObject* RVInputDataCB(void* p_data);

CExtUUID        RVInputDataID(
                    "{INPUT_DATA:3ecd9382-d88b-40c2-be05-07a67812a36f}",
                    "inputdata");

CPluginObject   RVInputDataObject(&ABSPlugin,
                    RVInputDataID,RESOURCES_CAT,
                    RVInputDataCB);

// -----------------------------------------------------------------------------

CComObject* RVInputDataCB(void* p_data)
{
    CComObject* p_object = new CRVInputData();
    return(p_object);
}

//------------------------------------------------------------------------------

using namespace std;

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

CRVInputData::CRVInputData(void)
    : CResourceValue(&RVInputDataObject)
{
}

//------------------------------------------------------------------------------

void CRVInputData::PreTestValue(CResourceList* p_rl,std::ostream& sout,bool& rstatus)
{
    if( (Value != "local") && (Value != "storage") ){
        if( rstatus == true ) sout << endl;
        sout << "<b><red> ERROR: Illegal '" << Name << "' resource specification!" << endl;
        sout <<         "        Allowed values 'local, storage, default' but '" << Value << "' is specified!</red></b>" << endl;
        rstatus = false;
        return;
    }
}

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

