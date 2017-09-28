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

#include <RVDataIn.hpp>
#include <CategoryUUID.hpp>
#include <ABSModule.hpp>
#include <ResourceList.hpp>

// -----------------------------------------------------------------------------

CComObject* RVDataInCB(void* p_data);

CExtUUID        RVDataInID(
                    "{DATAIN:7f6c7810-d684-438d-a1cc-1712f7a228d7}",
                    "datain");

CPluginObject   RVDataInObject(&ABSPlugin,
                    RVDataInID,RESOURCES_CAT,
                    RVDataInCB);

// -----------------------------------------------------------------------------

CComObject* RVDataInCB(void* p_data)
{
    CComObject* p_object = new CRVDataIn();
    return(p_object);
}

//------------------------------------------------------------------------------

using namespace std;

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

CRVDataIn::CRVDataIn(void)
    : CResourceValue(&RVDataInObject)
{
}

//------------------------------------------------------------------------------

void CRVDataIn::TestValue(CResourceList* p_rl,std::ostream& sout,bool& rstatus)
{
    if( TestKeyValue(sout,rstatus,"keep,copy,shared") == false ) return;

    CResourceValuePtr val_ptr = p_rl->FindResource("workdir");
    if( val_ptr != NULL ){
        if( (val_ptr->GetValue() == "jobdir") && (Value != "keep") ){
            if( rstatus == true ) sout << endl;
            sout << "<b><red> ERROR: Illegal '" << Name << "' resource specification!" << endl;
            sout << "<b><red>        workdir=jobdir must have datain=keep!</red></b>" << endl;
            rstatus = false;
            return;
        }
        if( (val_ptr->GetValue() != "jobdir") && ( (Value != "copy") && (Value != "shared") ) ){
            if( rstatus == true ) sout << endl;
            sout << "<b><red> ERROR: Illegal '" << Name << "' resource specification!" << endl;
            sout << "<b><red>        workdir!=jobdir must have datain=copy or datain=shared!</red></b>" << endl;
            rstatus = false;
            return;
        }
    }
}

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

