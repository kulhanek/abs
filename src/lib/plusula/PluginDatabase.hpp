#ifndef PluginDatabaseH
#define PluginDatabaseH
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

#include <PluSuLaMainHeader.hpp>
#include <PluginModule.hpp>
#include <PluginObject.hpp>
#include <XMLDocument.hpp>
#include <FileName.hpp>
#include <EventManager.hpp>

//------------------------------------------------------------------------------

class CComObject;

//------------------------------------------------------------------------------

DECLARE_EVENT(PLUSULA_PACKAGE,PluginDatabase_Changed);

//------------------------------------------------------------------------------

class PLUSULA_PACKAGE CPluginDatabase : public CEventManager {
public:
// konstruktor a destruktor ---------------------------------------------------
    CPluginDatabase(void);
    virtual ~CPluginDatabase(void);

// initialize -----------------------------------------------------------------
    //! set path to all plugins, it could be overwriter in particular plugin setup
    void SetPluginPath(const CFileName& plugin_dir);

    //! populate plugin database with config and load all plugins
    bool LoadPlugins(const CFileName& config_dir);

    //! unload all plugins
    bool UnloadPlugins(void);

    //! print info about path setup to standard output
    void PrintSetup(void);

// object management ----------------------------------------------------------
    CComObject* CreateObject(const CUUID& object_uuid,void* p_data=NULL);

    bool GetObjectCategory(const CUUID& object_uuid,CExtUUID& object_cat) const;
    bool GetObjectExtType(const CUUID& object_uuid,CExtUUID& extobject_uuid) const;

    bool FindObjectConfigValue(const CUUID& object_uuid,
                               const CSmallString& key,
                               CUUID& value) const;
    bool FindObjectConfigValue(const CUUID& object_uuid,
                               const CSmallString& key,
                               CExtUUID& value) const;
    bool FindObjectConfigValue(const CUUID& object_uuid,
                               const CSmallString& key,
                               CSmallString& value) const;

// individual object setup
    CXMLElement* GetRootConfigElement(void) const;
    CXMLElement* GetModuleElement(const CUUID& module_uuid) const;
    CXMLElement* GetObjectElement(const CUUID& object_uuid) const;
    CXMLElement* GetObjectElement(CPluginObject* p_pod) const;

// information about databases -----------------------------------------------
    const CSimpleList<CPluginModule>& GetModuleList(void) const;
    const CSimpleList<CPluginObject>& GetObjectList(void) const;

    CPluginModule* FindPluginModule(const CUUID& module_uuid);
    CPluginObject* FindPluginObject(const CUUID& object_uuid);

    bool AttachModule(CUUID& module_uuid);
    bool DetachModule(CUUID& module_uuid);

// section of private data ----------------------------------------------------
private:
    CFileName                       PluginsPath;
    CFileName                       ConfigPath;

    CXMLDocument                    PluginConfig;
    CSimpleList<CPluginModule>      PluginModules;
    CSimpleList<CDynamicPackage>    PluginPackages;
    CSimpleList<CPluginObject>      PluginObjects;


    bool FindObjectModule(const CUUID& object_uuid,CUUID& module_uuid);
    bool AddPluginConfig(const CFileName& file_name);

    // send event about change
    void NotifyDescriptionChange(CPluginObject* p_changedobject);
    void NotifyDescriptionChange(CPluginModule* p_changedmodule);

    friend class CPluginObject;
    friend class CPluginModule;
};

//------------------------------------------------------------------------------

extern PLUSULA_PACKAGE CPluginDatabase PluginDatabase;

//------------------------------------------------------------------------------
#endif
