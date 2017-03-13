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

#include <PluginObject.hpp>
#include <PluginDatabase.hpp>
#include <ErrorSystem.hpp>
#include <FileName.hpp>

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

CPluginObject::CPluginObject(CPluginModule*   p_module,
                             const CExtUUID&                objectuuid,
                             const CExtUUID&                categoryuuid,
                             TCreateObject                  createobject)
    : ObjectUUID(objectuuid), CategoryUUID(categoryuuid)
{
    ObjectCounter = 0;
    PluginModule = NULL;
    ObjectConfig = NULL;
    ObjectCallback = createobject;

    // register object to database
    PluginDatabase.PluginObjects.InsertToEnd(this);

    // register object to plugin module
    // NOTE: do not use PluginModule untill CPluginModule::InitializeModule is called
    PluginModule = p_module;
}

// -----------------------------------------------------------------------------

CPluginObject::~CPluginObject(void)
{
    PluginDatabase.PluginObjects.Remove(this);
}

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

void CPluginObject::InitializeObject(CXMLElement* p_objectconfig)
{
    ObjectConfig = p_objectconfig;
}

// -----------------------------------------------------------------------------

CComObject* CPluginObject::CreateObject(void* p_data)
{
    if( ObjectCallback == NULL ) return(NULL);
    return(ObjectCallback(p_data));
}

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

const CSmallString&  CPluginObject::GetName(void) const
{
    return(ObjectUUID.GetName());
}

//---------------------------------------------------------------------------

const CSmallString&  CPluginObject::GetDescription(void) const
{
    return (ObjectUUID.GetDescription());
}

//---------------------------------------------------------------------------

const CExtUUID& CPluginObject::GetCategoryUUID(void) const
{
    return(CategoryUUID);
}

//---------------------------------------------------------------------------

const CExtUUID& CPluginObject::GetObjectUUID(void) const
{
    return(ObjectUUID);
}

//---------------------------------------------------------------------------

int CPluginObject::GetObjectCount(void) const
{
    return(ObjectCounter);
}

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

void CPluginObject::CreateObject(void)
{
    ObjectCounter++;
    PluginDatabase.NotifyDescriptionChange(this);
}

// -----------------------------------------------------------------------------

void __hipolyspec   CPluginObject::DestroyObject(void)
{
    ObjectCounter--;
    PluginDatabase.NotifyDescriptionChange(this);
}

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

CXMLElement* CPluginObject::GetObjectConfig(void) const
{
    return(ObjectConfig);
}

// -----------------------------------------------------------------------------

CPluginModule* CPluginObject::GetPluginModule(void) const
{
    return(PluginModule);
}

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================




