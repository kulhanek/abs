// =============================================================================
// ABS - Advanced Batch System
// -----------------------------------------------------------------------------
//    Copyright (C) 2021 Petr Kulhanek, kulhanek@chemi.muni.cz
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

#include <RVVNode.hpp>
#include <CategoryUUID.hpp>
#include <PBSProModule.hpp>
#include <ResourceList.hpp>
#include <pbs_ifl.h>

// -----------------------------------------------------------------------------

CComObject* RVVNodeCB(void* p_data);

CExtUUID        RVVNodeID(
                    "{VNODE:d233e435-0110-4a8d-88cf-60442053fa8e}",
                    "vnode");

CPluginObject   RVVNodeObject(&PBSProPlugin,
                    RVVNodeID,RESOURCES_CAT,
                    RVVNodeCB);

// -----------------------------------------------------------------------------

CComObject* RVVNodeCB(void* p_data)
{
    CComObject* p_object = new CRVVNode();
    return(p_object);
}

//------------------------------------------------------------------------------

using namespace std;

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

CRVVNode::CRVVNode(void)
    : CResourceValue(&RVVNodeObject)
{
}

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

