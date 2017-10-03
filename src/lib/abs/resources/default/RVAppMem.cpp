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

#include <RVAppMem.hpp>
#include <CategoryUUID.hpp>
#include <ABSModule.hpp>
#include <ResourceList.hpp>

// -----------------------------------------------------------------------------

CComObject* RVAppMemCB(void* p_data);

CExtUUID        RVAppMemID(
                    "{APPMEM:b1a03b58-a6b0-4ae9-a83f-63ba8e9890d0}",
                    "appmem");

CPluginObject   RVAppMemObject(&ABSPlugin,
                    RVAppMemID,RESOURCES_CAT,
                    RVAppMemCB);

// -----------------------------------------------------------------------------

CComObject* RVAppMemCB(void* p_data)
{
    CComObject* p_object = new CRVAppMem();
    return(p_object);
}

//------------------------------------------------------------------------------

using namespace std;

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

CRVAppMem::CRVAppMem(void)
    : CResourceValue(&RVAppMemObject)
{
}

//------------------------------------------------------------------------------

void CRVAppMem::TestValue(CResourceList* p_rl,std::ostream& sout,bool& rstatus)
{
    string svalue(Value);

    bool fraction = true;
    string legall_characters_size = "01234567890.";
    if( svalue.find_first_not_of(legall_characters_size) != string::npos ){
        fraction = false;
    }

    if( fraction ){
        double ratio = GetFloatNumber();
        if( (ratio <= 0) || (ratio > 1.0) ) {
            if( rstatus == true ) sout << endl;
            sout << "<b><red> ERROR: Illegal '" << Name << "' resource specification!" << endl;
            sout <<         "        Application memory fraction must be within (0.0; 1.0> interval but " << GetFloatNumber() << " is specified!</red></b>" << endl;
            rstatus = false;
            return;
        }
    } else {
        if( TestSizeValue(sout,rstatus) == false ) return;
        long long size = GetSize();
        if( size <= 0 ) {
            if( rstatus == true ) sout << endl;
            sout << "<b><red> ERROR: Illegal '" << Name << "' resource specification!" << endl;
            sout <<         "        Application memory size must be larger than zero but " << GetSizeString() << " is specified!</red></b>" << endl;
            rstatus = false;
            return;
        }
        // get requested memory from batch system and test if provided appmem is smaller
        long long mem = 0;
        const CResourceValuePtr p_rv = p_rl->FindResource("mem");
        if( p_rv != NULL ){
            mem = p_rv->GetSize();
        } else {
            const CResourceValuePtr p_rv1 = p_rl->FindResource("mempercpu");
            const CResourceValuePtr p_rv2 = p_rl->FindResource("ncpus");
            if( (p_rv1 != NULL) && (p_rv2 != NULL) ){
                mem = p_rv1->GetSize()  * p_rv2->GetNumber();
            }
        }
        if( (mem > 0) && (size > mem) ){
            if( rstatus == true ) sout << endl;
            sout << "<b><red> ERROR: Illegal '" << Name << "' resource specification!" << endl;
            sout <<         "        Application memory size must be smaler than " << GetSizeString(mem) << " but " << GetSizeString() << " is specified!</red></b>" << endl;
            rstatus = false;
            return;
        }

    }
}

//------------------------------------------------------------------------------

void CRVAppMem::ResolveConflicts(CResourceList* p_rl)
{
    p_rl->RemoveResource("appmempernode");
}

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

