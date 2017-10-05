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

#include <RVMemPerNode.hpp>
#include <CategoryUUID.hpp>
#include <ABSModule.hpp>
#include <ResourceList.hpp>
#include <sstream>

// -----------------------------------------------------------------------------

CComObject* RVMemPerNodeCB(void* p_data);

CExtUUID        RVMemPerNodeID(
                    "{RV_MEM_PER_NODE:7e8fbe6a-00d2-47b2-a35b-bde0efdb3790}",
                    "mempernode");

CPluginObject   RVMemPerNodeObject(&ABSPlugin,
                    RVMemPerNodeID,RESOURCES_CAT,
                    RVMemPerNodeCB);

// -----------------------------------------------------------------------------

CComObject* RVMemPerNodeCB(void* p_data)
{
    CComObject* p_object = new CRVMemPerNode();
    return(p_object);
}

//------------------------------------------------------------------------------

using namespace std;

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

CRVMemPerNode::CRVMemPerNode(void)
    : CResourceValue(&RVMemPerNodeObject)
{
    Requires.push_back("nnodes");
    Provides.push_back("mem");
}

//------------------------------------------------------------------------------

void CRVMemPerNode::PreTestValue(CResourceList* p_rl,std::ostream& sout,bool& rstatus)
{
    if( TestSizeValue(sout,rstatus) == false ) return;
    long long size = GetSize(); // in kb
    if( size <= 1 ) {
        if( rstatus == true ) sout << endl;
        sout << "<b><red> ERROR: Illegal '" << Name << "' resource specification!" << endl;
        sout <<         "        Size must be larger than 1kb but " << GetSizeString() << " is specified!</red></b>" << endl;
        rstatus = false;
        return;
    }
}

//------------------------------------------------------------------------------

void CRVMemPerNode::ResolveDynamicResource(CResourceList* p_rl,bool& delete_me)
{
    CResourceValuePtr res = p_rl->FindResource("nnodes");
    int nnodes = 1;
    if( res != NULL ){
        nnodes = res->GetNumber();
    }
    long long mem = GetSize() * nnodes;
    stringstream    str;
    str << mem;

    p_rl->AddRawResource("mem",str.str().c_str());
}

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

