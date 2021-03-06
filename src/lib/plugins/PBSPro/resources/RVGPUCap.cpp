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

#include <RVGPUCap.hpp>
#include <CategoryUUID.hpp>
#include <PBSProModule.hpp>
#include <ResourceList.hpp>
#include <pbs_ifl.h>

// -----------------------------------------------------------------------------

CComObject* RVGPUCapCB(void* p_data);

CExtUUID        RVGPUCapID(
                    "{GPUCAP:f88b3567-b4d4-477c-9ff8-638d6bbbbc90}",
                    "gpucap");

CPluginObject   RVGPUCapObject(&PBSProPlugin,
                    RVGPUCapID,RESOURCES_CAT,
                    RVGPUCapCB);

// -----------------------------------------------------------------------------

CComObject* RVGPUCapCB(void* p_data)
{
    CComObject* p_object = new CRVGPUCap();
    return(p_object);
}

//------------------------------------------------------------------------------

using namespace std;

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

CRVGPUCap::CRVGPUCap(void)
    : CResourceValue(&RVGPUCapObject)
{
}

//------------------------------------------------------------------------------

void CRVGPUCap::ResolveDynamicResource(CResourceList* p_rl,bool& delete_me)
{
    CResourceValuePtr p_ngpus = p_rl->FindResource("ngpus");
    // remove this attribute if ngpus == 0
    if( (p_ngpus == NULL) || (p_ngpus->GetNumber() == 0) ) delete_me = true;
}

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

