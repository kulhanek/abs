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

#include <ResourceValue.hpp>
#include <PluginDatabase.hpp>

//------------------------------------------------------------------------------

using namespace std;

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

CResourceValue::CResourceValue(CPluginObject* p_objectinfo)
    : CComObject(p_objectinfo)
{
    if( p_objectinfo ){
        PluginDatabase.FindObjectConfigValue(p_objectinfo->GetObjectUUID(),"_name",Name);
    }
}

//------------------------------------------------------------------------------

void CResourceValue::TestValue(std::ostream& sout,bool& rstatus)
{
    // nothing to be here
}

//------------------------------------------------------------------------------

void CResourceValue::ResolveConflicts(void)
{
    // nothing to be here
}

//------------------------------------------------------------------------------

void CResourceValue::FinalizeResource(void)
{
    // nothing to be here
}

//------------------------------------------------------------------------------

void CResourceValue::SetVariables(std::map<std::string,std::string>& variables)
{
    // nothing to be here
}

//------------------------------------------------------------------------------

const CSmallString CResourceValue::GetBatchResource(void)
{
    // nothing to be here
    return("");
}

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

