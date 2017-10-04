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

#include <RVNCPUsPerNode.hpp>
#include <CategoryUUID.hpp>
#include <ABSModule.hpp>
#include <ResourceList.hpp>
#include <sstream>

// -----------------------------------------------------------------------------

CComObject* RVNCPUsPerNodeCB(void* p_data);

CExtUUID        RVNCPUsPerNodeID(
                    "{RV_NCPUS_PER_NODE:4b20d511-97ee-47c4-af31-b1b492712d76}",
                    "ncpuspernode");

CPluginObject   RVNCPUsPerNodeObject(&ABSPlugin,
                    RVNCPUsPerNodeID,RESOURCES_CAT,
                    RVNCPUsPerNodeCB);

// -----------------------------------------------------------------------------

CComObject* RVNCPUsPerNodeCB(void* p_data)
{
    CComObject* p_object = new CRVNCPUsPerNode();
    return(p_object);
}

//------------------------------------------------------------------------------

using namespace std;

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

CRVNCPUsPerNode::CRVNCPUsPerNode(void)
    : CResourceValue(&RVNCPUsPerNodeObject)
{
    Requires.push_back("ncpus");
    Provides.push_back("nnodes");
}

//------------------------------------------------------------------------------

void CRVNCPUsPerNode::PreTestValue(CResourceList* p_rl,std::ostream& sout,bool& rstatus)
{
    if( TestNumberValue(sout,rstatus) == false ) return;
    long long value = GetNumber();
    if( value <= 0 ) {
        if( rstatus == true ) sout << endl;
        sout << "<b><red> ERROR: Illegal '" << Name << "' resource specification!" << endl;
        sout <<         "        At least one CPU must be requested but '" << value << "' is specified!</red></b>" << endl;
        rstatus = false;
        return;
    }
    int ncpus = p_rl->GetNumOfCPUs();
    // ncpus <= 0 is illegal and tested elsewhere
    if( ncpus > 0 ){
        if( ncpus % value != 0 ){
            if( rstatus == true ) sout << endl;
            sout << "<b><red> ERROR: Illegal '" << Name << "' resource specification!" << endl;
            sout <<         "        ncpus=" << ncpus << " must be divisible by ncpuspernode=" << value << "!</red></b>" << endl;
            rstatus = false;
            return;
        }
        if( ncpus / value == 0 ){
            if( rstatus == true ) sout << endl;
            sout << "<b><red> ERROR: Illegal '" << Name << "' resource specification!" << endl;
            sout <<         "        ncpus=" << ncpus << " must be greater than ncpuspernode=" << value << "!</red></b>" << endl;
            rstatus = false;
            return;
        }
    }
}

//------------------------------------------------------------------------------

void CRVNCPUsPerNode::ResolveDynamicResource(CResourceList* p_rl,bool& delete_me)
{
    CResourceValuePtr res = p_rl->FindResource("ncpus");
    int ncpus = 1;
    if( res != NULL ){
        ncpus = res->GetNumber();
    }
    long long nnodes = ncpus / GetNumber();
    stringstream    str;
    str << nnodes;

    p_rl->AddRawResource("nnodes",str.str().c_str());
}

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

