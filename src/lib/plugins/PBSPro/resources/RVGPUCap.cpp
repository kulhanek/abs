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

#include <GPUCap.hpp>
#include <CategoryUUID.hpp>
#include <ABSModule.hpp>
#include <ResourceList.hpp>
#include <pbs_ifl.h>

// -----------------------------------------------------------------------------

CComObject* GPUCapCB(void* p_data);

CExtUUID        GPUCapID(
                    "{PLACE:9f391d49-b3f2-4a36-b4c5-cd211223d9b4}",
                    "place");

CPluginObject   GPUCapObject(&ABSPlugin,
                    GPUCapID,RESOURCES_CAT,
                    GPUCapCB);

// -----------------------------------------------------------------------------

CComObject* GPUCapCB(void* p_data)
{
    CComObject* p_object = new CGPUCap();
    return(p_object);
}

//------------------------------------------------------------------------------

using namespace std;

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

CGPUCap::CGPUCap(void)
    : CResourceValue(&GPUCapObject)
{
}

//------------------------------------------------------------------------------

void CGPUCap::ResolveDynamicResource(CResourceList* p_rl,bool delete_me)
{
    CResourceValuePtr p_ngpus = p_rl->FindResources("ngpus");
    // remove this attribute if ngpus == 0
    if( (p_ngpus == NULL) || (p_ngpus->GetNumber() == 0) ) delete_me = true;
}

//------------------------------------------------------------------------------

void CGPUCap::GetAttribute(CSmallString& name, CSmallString& resource, CSmallString& value)
{
    name = ATTR_l;
    resource = "gpu_cap";
    value = Value;
}

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

