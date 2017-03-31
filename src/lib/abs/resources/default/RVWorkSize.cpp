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

#include <RVWorkSize.hpp>
#include <CategoryUUID.hpp>
#include <ABSModule.hpp>
#include <ResourceList.hpp>

// -----------------------------------------------------------------------------

CComObject* RVWorkSizeCB(void* p_data);

CExtUUID        RVWorkSizeID(
                    "{WORK_SIZE:829fae68-ffed-4aae-8fab-b701fc623974}",
                    "worksize");

CPluginObject   RVWorkSizeObject(&ABSPlugin,
                    RVWorkSizeID,RESOURCES_CAT,
                    RVWorkSizeCB);

// -----------------------------------------------------------------------------

CComObject* RVWorkSizeCB(void* p_data)
{
    CComObject* p_object = new CRVWorkSize();
    return(p_object);
}

//------------------------------------------------------------------------------

using namespace std;

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

CRVWorkSize::CRVWorkSize(void)
    : CResourceValue(&RVWorkSizeObject)
{
}

//------------------------------------------------------------------------------

void CRVWorkSize::TestValue(CResourceList* p_rl,std::ostream& sout,bool& rstatus)
{
    if( TestSizeValue(sout,rstatus) == false ) return;
    long long size = GetSize();
    if( size <= 0 ) {
        if( rstatus == true ) sout << endl;
        sout << "<b><red> ERROR: Illegal '" << Name << "' resource specification!" << endl;
        sout <<         "        Size must be larger than 1kb but " << GetSizeString() << " is specified!</red></b>" << endl;
        rstatus = false;
    }
}

//------------------------------------------------------------------------------

void CRVWorkSize::ResolveConflicts(CResourceList* p_rl)
{
    p_rl->RemoveResource("worksizepercpu");
}

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

