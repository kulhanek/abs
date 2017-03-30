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

#include <RVMaxCPUsPerNode.hpp>
#include <CategoryUUID.hpp>
#include <ABSModule.hpp>

// -----------------------------------------------------------------------------

CComObject* RVMaxCPUsPerNodeCB(void* p_data);

CExtUUID        RVMaxCPUsPerNodeID(
                    "{MAX_CPUS_PER_NODE:93c5d47b-7a1b-47b5-852e-84691704974c}",
                    "maxcpuspernode");

CPluginObject   RVMaxCPUsPerNodeObject(&ABSPlugin,
                    RVMaxCPUsPerNodeID,RESOURCES_CAT,
                    RVMaxCPUsPerNodeCB);

// -----------------------------------------------------------------------------

CComObject* RVMaxCPUsPerNodeCB(void* p_data)
{
    CComObject* p_object = new CRVMaxCPUsPerNode();
    return(p_object);
}

//------------------------------------------------------------------------------

using namespace std;

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

CRVMaxCPUsPerNode::CRVMaxCPUsPerNode(void)
    : CResourceValue(&RVMaxCPUsPerNodeObject)
{
}

//------------------------------------------------------------------------------

void CRVMaxCPUsPerNode::TestValue(std::ostream& sout,bool& rstatus)
{

}

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

