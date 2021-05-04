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

#include <RVNCPUsPerNUMANode.hpp>
#include <CategoryUUID.hpp>
#include <ABSModule.hpp>
#include <ResourceList.hpp>
#include <sstream>

// -----------------------------------------------------------------------------

CComObject* RVNCPUsPerNUMANodeCB(void* p_data);

CExtUUID        RVNCPUsPerNUMANodeID(
                    "{RV_NCPUS_PER_NUMA_NODE:a7121430-5aec-46e8-9bcb-b1362d298dda}",
                    "ncpuspernumanode");

CPluginObject   RVNCPUsPerNUMANodeObject(&ABSPlugin,
                    RVNCPUsPerNUMANodeID,RESOURCES_CAT,
                    RVNCPUsPerNUMANodeCB);

// -----------------------------------------------------------------------------

CComObject* RVNCPUsPerNUMANodeCB(void* p_data)
{
    CComObject* p_object = new CRVNCPUsPerNUMANode();
    return(p_object);
}

//------------------------------------------------------------------------------

using namespace std;

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

CRVNCPUsPerNUMANode::CRVNCPUsPerNUMANode(void)
    : CResourceValue(&RVNCPUsPerNUMANodeObject)
{
}

//------------------------------------------------------------------------------

void CRVNCPUsPerNUMANode::PreTestValue(CResourceList* p_rl,std::ostream& sout,bool& rstatus)
{
    if( TestNumberValue(sout,rstatus) == false ) return;
    long long value = GetNumber();
    if( value <= 0 ) {
        if( rstatus == true ) sout << endl;
        sout << "<b><red> ERROR: Illegal '" << Name << "' resource specification!" << endl;
        sout <<         "        At least one CPU must be present in CPU NUMA node but '" << value << "' is specified!</red></b>" << endl;
        rstatus = false;
        return;
    }
}

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

