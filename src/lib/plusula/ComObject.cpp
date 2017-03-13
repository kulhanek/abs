// =============================================================================
// NEMESIS - Molecular Modelling Package
// -----------------------------------------------------------------------------
//    Copyright (C) 2010      Petr Kulhanek, kulhanek@chemi.muni.cz
//    Copyright (C) 2008-2009 Petr Kulhanek, kulhanek@enzim.hu,
//                            Jakub Stepan, xstepan3@chemi.muni.cz
//    Copyright (C) 1998-2004 Petr Kulhanek, kulhanek@chemi.muni.cz
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

#include <ComObject.hpp>
#include <PluginObject.hpp>

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

CComObject::CComObject(CPluginObject* objectinfo)
{
    ObjectInfo = objectinfo;
    if( ObjectInfo != NULL ) ObjectInfo->CreateObject();
}

// -----------------------------------------------------------------------------

CComObject::~CComObject(void)
{
    if( ObjectInfo != NULL ) ObjectInfo->DestroyObject();
}

// -----------------------------------------------------------------------------

const CExtUUID& CComObject::GetType(void) const
{
    static CExtUUID uuid;
    if( ObjectInfo == NULL ) return(uuid);
    return(ObjectInfo->GetObjectUUID());
}

// -----------------------------------------------------------------------------

const CExtUUID& CComObject::GetCategory(void) const
{
    static CExtUUID uuid;
    if( ObjectInfo == NULL ) return(uuid);
    return(ObjectInfo->GetCategoryUUID());
}

//------------------------------------------------------------------------------

CPluginObject*  CComObject::GetPluginObject(void) const
{
    return(ObjectInfo);
}

// -----------------------------------------------------------------------------

