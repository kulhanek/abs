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

#include <PluginModule.hpp>
#include <PluginDatabase.hpp>
#include <ErrorSystem.hpp>
#include <XMLElement.hpp>
#include <XMLIterator.hpp>

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

CPluginModule::CPluginModule(const CExtUUID& moduleuuid,
                             TModuleCallbackFunction callbackfce)
    : ModuleUUID(moduleuuid)
{
    ModuleCallback = callbackfce;
    ModuleConfig = NULL;
    ModulePackage = NULL;
    PluginDatabase.PluginModules.InsertToEnd(this);
}

//---------------------------------------------------------------------------

CPluginModule::~CPluginModule(void)
{
    PluginDatabase.PluginModules.Remove(this);
}

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

bool CPluginModule::InitializeModule(CDynamicPackage* p_package,
                                     CXMLElement* p_moduleconfig)
{
    if( p_moduleconfig == NULL ) return(false);
    ModuleConfig = p_moduleconfig;

    ModulePackage = p_package;

    // get module name and path
    if( ModuleConfig->GetAttribute("name",ModuleName) == false ) {
        // this should not happen
        ES_ERROR("name attribute is missing for module");
        return(false);
    }

    if( ModuleConfig->GetAttribute("path",Path) == false ) {
        // take path from global setup;
        Path = PluginDatabase.PluginsPath;
    }

    // reconnect plugin objects to plugin module
    CSimpleIteratorC<CPluginObject> I(PluginDatabase.GetObjectList());
    CPluginObject* p_object;

    while((p_object = I.Current()) !=NULL) {
        if( p_object->GetPluginModule() == this ) {
            if( ! ModuleObjects.IsInList(p_object) ) ModuleObjects.InsertToEnd(p_object);
        }
        I++;
    }

    // connect to module objects -------------------
    CXMLIterator    S(ModuleConfig);
    CXMLElement*    p_tele;

    while( (p_tele = S.GetNextChildElement("OBJECT")) != NULL ) {
        CSmallString uuid;
        p_tele->GetAttribute("uuid",uuid);
        CExtUUID ext_uuid;
        ext_uuid.LoadFromString(uuid);

        // find object
        CPluginObject* p_object = PluginDatabase.FindPluginObject(ext_uuid);
        if( p_object != NULL ) {
            if( (p_object->GetPluginModule() != NULL) && (p_object->GetPluginModule() != this) ) {
                CSmallString error;
                error << "object ownership mismatch: " << ext_uuid.GetFullStringForm()
                      << " is not member of " << ModuleUUID.GetFullStringForm();
                ES_ERROR(error);
                return(false);
            }

            p_object->InitializeObject(p_tele);

            // add module to the list if it is not there
            if( ! ModuleObjects.IsInList(p_object) ) ModuleObjects.InsertToEnd(p_object);
        } else {
            // report eror behaviour
            CSmallString error;
            error << ext_uuid.GetFullStringForm() << " was not found in plugin database";
            ES_ERROR(error);
            return(false);
        }
    }

    // initialize module
    bool result = true;
    if( ModuleCallback != NULL ) {
        result &= ModuleCallback(EMR_INITIALIZE,NULL);
        if( result == false ) {
            CSmallString error;
            error << "EMR_INITIALIZE failed for module " << ModuleUUID.GetFullStringForm();
            ES_ERROR(error);
        }
    }

    return(result);
}

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

bool CPluginModule::FinalizeModule(void)
{
    // check if there are any opened objects ---------------------
    CSimpleIterator<CPluginObject> P(ModuleObjects);
    CPluginObject* p_pod;

    while((p_pod = P.Current())!=NULL) {
        if( p_pod->GetObjectCount() > 0 ) {
            CSmallString error;
            error << p_pod->GetObjectUUID().GetFullStringForm()
                  << " has still opened objects (" << p_pod->GetObjectCount() << ")";
            ES_ERROR(error);
        }
        P++;
    }

    // finalize package -------------------------------
    bool result = true;
    if( ModuleCallback != NULL ) {
        result &= ModuleCallback(EMR_FINALIZE,NULL);
        if( result == false ) {
            CSmallString error;
            error << "EMR_FINALIZE failed for module " << ModuleUUID.GetFullStringForm();
            ES_ERROR(error);
        }
    }

    return(true);
}

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

const CSmallString&  CPluginModule::GetModuleName(void) const
{
    return(ModuleName);
}

//---------------------------------------------------------------------------

const CExtUUID& CPluginModule::GetModuleUUID(void) const
{
    return(ModuleUUID);
}

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

const CSimpleList<CPluginObject>&   CPluginModule::GetObjectList(void) const
{
    return(ModuleObjects);
}

//---------------------------------------------------------------------------

CDynamicPackage* CPluginModule::GetDynamicPackage(void) const
{
    return(ModulePackage);
}

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================


