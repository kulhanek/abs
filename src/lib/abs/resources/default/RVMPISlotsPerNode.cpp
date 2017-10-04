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

#include <RVMPISlotsPerNode.hpp>
#include <CategoryUUID.hpp>
#include <ABSModule.hpp>
#include <ResourceList.hpp>

// -----------------------------------------------------------------------------

CComObject* RVMPISlotsPerNodeCB(void* p_data);

CExtUUID        RVMPISlotsPerNodeID(
                    "{RV_MPI_SLOTS_PER_NODE:851f47b7-fe28-46e1-95ea-88a114cbf99c}",
                    "mpislotspernode");

CPluginObject   RVMPISlotsPerNodeObject(&ABSPlugin,
                    RVMPISlotsPerNodeID,RESOURCES_CAT,
                    RVMPISlotsPerNodeCB);

// -----------------------------------------------------------------------------

CComObject* RVMPISlotsPerNodeCB(void* p_data)
{
    CComObject* p_object = new CRVMPISlotsPerNode();
    return(p_object);
}

//------------------------------------------------------------------------------

using namespace std;

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

CRVMPISlotsPerNode::CRVMPISlotsPerNode(void)
    : CResourceValue(&RVMPISlotsPerNodeObject)
{
}

//------------------------------------------------------------------------------

void CRVMPISlotsPerNode::PreTestValue(CResourceList* p_rl,std::ostream& sout,bool& rstatus)
{
    if( TestNumberValue(sout,rstatus) == false ) return;
    long long value = GetNumber();
    if( value <= 0 ) {
        if( rstatus == true ) sout << endl;
        sout << "<b><red> ERROR: Illegal '" << Name << "' resource specification!" << endl;
        sout <<         "        At least one MPI slot must be requested but '" << value << "' is specified!</red></b>" << endl;
        rstatus = false;
        return;
    }
}

//------------------------------------------------------------------------------

void CRVMPISlotsPerNode::PostTestValue(CResourceList* p_rl,std::ostream& sout,bool& rstatus)
{
    int ncpuspernode = 0;
    int ncpus = p_rl->GetNumOfCPUs();
    int nnodes = p_rl->GetNumOfNodes();
    if( nnodes > 0 ){
        // nnodes <= 0 is illegal and tested elsewhere
        ncpuspernode = ncpus / nnodes;
    }

    if( ncpuspernode <= 0 ){
        if( rstatus == true ) sout << endl;
        sout << "<b><red> ERROR: Illegal '" << Name << "' resource specification!" << endl;
        sout <<         "        Unable to calculate ncpuspernode or ncpuspernode is set to an illegal value!</red></b>" << endl;
        rstatus = false;
        return;
    }
    long long value = GetNumber();
    if( value > ncpuspernode ){
        if( rstatus == true ) sout << endl;
        sout << "<b><red> ERROR: Illegal '" << Name << "' resource specification!" << endl;
        sout <<         "        mpislotspernode=" << value << " must be less than or equal to ncpuspernode=" << ncpuspernode << "!</red></b>" << endl;
        rstatus = false;
        return;
    }
}

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

