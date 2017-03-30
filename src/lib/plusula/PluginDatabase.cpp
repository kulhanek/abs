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

#include <PluginDatabase.hpp>
#include <ErrorSystem.hpp>
#include <XMLIterator.hpp>
#include <XMLParser.hpp>
#include <DirectoryEnum.hpp>
#include <XMLPrinter.hpp>
#include <SystemFS.hpp>

//------------------------------------------------------------------------------

/*
<PLUGIN name="" path="" uuid="" mode="">
    <OBJECT uuid="">
        <ITEM name="" value""/>
        ...
    </OBJECT>
    ...
</PLUGIN>
*/

//------------------------------------------------------------------------------

CPluginDatabase PluginDatabase;

DEFINE_EVENT(PluginDatabase_Changed);

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

CPluginDatabase::CPluginDatabase(void)
{
}

//---------------------------------------------------------------------------

CPluginDatabase::~CPluginDatabase(void)
{
}

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

void CPluginDatabase::SetPluginPath(const CFileName& plugin_dir)
{
    PluginsPath = plugin_dir;
}

//---------------------------------------------------------------------------

bool CPluginDatabase::LoadPlugins(const CFileName& config_dir)
{
    ConfigPath =  config_dir;

    CSmallString   error;

    // create root directory
    if( PluginConfig.CreateChildElement("PLUGINS") == NULL ) {
        error = "unable to create PLUGINS element";
        ES_ERROR(error);
        return(false);
    }

    // and enumerate config directory
    CDirectoryEnum denum(ConfigPath);

    if( denum.StartFindFile("*.plg") == false ) {
        error = "unable to enumerate config directory";
        ES_ERROR(error);
        return(false);
    }

    bool result = true;

    CFileName   plg_name;
    while( denum.FindFile(plg_name) == true ) {
        result &= AddPluginConfig(ConfigPath / plg_name);
    }

    denum.EndFindFile();

    return(result);
}

//---------------------------------------------------------------------------

bool CPluginDatabase::AddPluginConfig(const CFileName& file_name)
{
    CSmallString   error;

    // load config file -----------------------------
    CXMLDocument config_file;
    CXMLParser xml_parser;

    xml_parser.SetOutputXMLNode(&config_file);
    if( xml_parser.Parse(file_name) == false ) {
        error = "unable load plugin config from " + file_name;
        ES_ERROR(error);
        return(false);
    }

    // some check procedure -------------------------
    CXMLElement* p_ele;
    p_ele = config_file.GetFirstChildElement("PLUGIN");
    if( p_ele == NULL ) {
        error = "config file " + file_name + " does not have PLUGIN element";
        ES_ERROR(error);
        return(false);
    }

    // load plugin ----------------------------------
    CSmallString   suuid;
    CExtUUID  ext_uuid;

    if( p_ele->GetAttribute("uuid",suuid) == false) {
        CSmallString error;
        error << "attribute 'uuid' is not specified or has incorrect structure (" << file_name << ")";
        ES_ERROR(error);
        return(false);
    }

    if( ext_uuid.LoadFromString(suuid) == false ) {
        CSmallString error;
        error << "attribute 'uuid = " << suuid << "' has incorrect structure (" << file_name << ")";
        ES_ERROR(error);
        return(false);
    }

    // copy plugin element as new one to database
    CXMLElement* p_dele = PluginConfig.GetFirstChildElement("PLUGINS");
    if( p_dele == NULL ) {
        // this should not happend
        ES_ERROR("PLUGINS is NULL");
        return(false);
    }

    // put some checks here to ensure that there are not any overlaps

    p_dele->CopyChildNodesFrom(&config_file);

    if( AttachModule(ext_uuid) == false ) {
        CSmallString error;
        error << "unable to attach plugin " << ext_uuid.GetFullStringForm();
        ES_ERROR(error);
        return(false);
    }
    return(true);
}

//---------------------------------------------------------------------------

bool CPluginDatabase::UnloadPlugins(void)
{
    // we need to unload all modules
    CSimpleIterator<CPluginModule> I(PluginModules);
    CPluginModule*                 p_module;

    // call FINALIZE callback function
    while((p_module = I.Current())!=NULL) {
        p_module->FinalizeModule();
        I++;
    }

    // unload all packages
    PluginPackages.RemoveAll();

    return(true);
}

//------------------------------------------------------------------------------

void CPluginDatabase::PrintSetup(void)
{
    printf("Plugins setup:\n");
    printf("   Plugins setup    : %s\n",(const char*)ConfigPath);
    printf("   Plugins location : %s\n",(const char*)PluginsPath);

    CXMLPrinter xml_printer;

    xml_printer.SetPrintedXMLNode(&PluginConfig);
    xml_printer.Print(stdout);

    printf("\n");
}

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

CComObject* CPluginDatabase::CreateObject(const CUUID& object_uuid,void* p_data)
{
    CPluginObject* p_pod;

    p_pod = FindPluginObject(object_uuid);

    if( p_pod == NULL ) {
        // plugin module is not attached yet

        // find module
        CUUID  module_uuid;
        if( FindObjectModule(object_uuid,module_uuid) == false ) return(NULL);

        // attach module
        if( AttachModule(module_uuid) == false ) return(NULL);

        // try to find object once again
        p_pod = FindPluginObject(object_uuid);
        if( p_pod == NULL ) {
            CSmallString error;
            error = "object " + object_uuid.GetStringForm() + " does not exist in module " + module_uuid.GetStringForm();
            ES_ERROR(error);
            return(NULL);
        }
    }

    // create object
    return( p_pod->CreateObject(p_data) );
}

//---------------------------------------------------------------------------

bool CPluginDatabase::GetObjectExtType(const CUUID& object_uuid,
                                       CExtUUID& extobject_uuid) const
{
    CXMLElement*   p_ele;
    p_ele = PluginConfig.GetFirstChildElement("PLUGINS");
    CXMLIterator   E(p_ele);
    CXMLElement*   p_sele;

    while( (p_sele = E.GetNextChildElement("PLUGIN")) != NULL ) {
        CXMLIterator    S(p_sele);
        CXMLElement*    p_tele;
        while( (p_tele = S.GetNextChildElement("OBJECT")) != NULL ) {
            CSmallString uuid;
            p_tele->GetAttribute("uuid",uuid);
            CExtUUID ext_uuid;
            ext_uuid.LoadFromString(uuid);
            if( ext_uuid == object_uuid ) {
                extobject_uuid = ext_uuid;
                return(true);
            }
        }
    }

    CSmallString error;
    error = "object " + object_uuid.GetStringForm() + " does not have extended uuid in database";
    ES_ERROR(error);

    return(false);
}

//---------------------------------------------------------------------------

bool CPluginDatabase::FindObjectConfigValue(const CUUID& object_uuid,
        const CSmallString& key, CUUID& value) const
{
    CSmallString uuid_value;
    if( FindObjectConfigValue(object_uuid,key,uuid_value) == false ) return(false);
    CExtUUID extuuid;
    extuuid.LoadFromString(uuid_value);
    value = extuuid;
    return( value.IsValidUUID() );
}

//---------------------------------------------------------------------------

bool CPluginDatabase::FindObjectConfigValue(const CUUID& object_uuid,
        const CSmallString& key, CExtUUID& value) const
{
    CSmallString uuid_value;
    if( FindObjectConfigValue(object_uuid,key,uuid_value) == false ) return(false);
    CExtUUID extuuid;
    extuuid.LoadFromString(uuid_value);
    value = extuuid;
    return( value.IsValidUUID() );
}

//---------------------------------------------------------------------------

bool CPluginDatabase::FindObjectConfigValue(const CUUID& object_uuid,
        const CSmallString& key, CSmallString& value) const
{
    CXMLElement*   p_ele;
    p_ele = PluginConfig.GetFirstChildElement("PLUGINS");
    CXMLIterator   E(p_ele);
    CXMLElement*   p_sele;

    while( (p_sele = E.GetNextChildElement("PLUGIN")) != NULL ) {
        CXMLIterator    S(p_sele);
        CXMLElement*    p_tele;
        while( (p_tele = S.GetNextChildElement("OBJECT")) != NULL ) {
            CSmallString uuid;
            p_tele->GetAttribute("uuid",uuid);
            CExtUUID ext_uuid;
            ext_uuid.LoadFromString(uuid);
            if( ext_uuid == object_uuid ) {
                CXMLIterator    T(p_tele);
                CXMLElement*    p_mele;
                while( (p_mele = T.GetNextChildElement("CONFIG")) != NULL) {
                    CSmallString lname;
                    p_mele->GetAttribute("key",lname);
                    if( lname == key ) {
                        p_mele->GetAttribute("value",value);
                        return(true);
                    }
                }
            }
        }
    }

    CSmallString error;
    error << "object " << object_uuid.GetStringForm()
          << " does not have a config value for the key '" << key <<"'";
    ES_ERROR(error);

    return(false);
}

//---------------------------------------------------------------------------

const CSimpleList<CPluginObject>& CPluginDatabase::GetObjectList(void) const
{
    return(PluginObjects);
}

//---------------------------------------------------------------------------

const CSimpleList<CPluginModule>& CPluginDatabase::GetModuleList(void) const
{
    return(PluginModules);
}

//---------------------------------------------------------------------------

void CPluginDatabase::NotifyDescriptionChange(CPluginObject* changedobject)
{
    ThrowEvent(PluginDatabase_Changed,changedobject);
}

//---------------------------------------------------------------------------

void CPluginDatabase::NotifyDescriptionChange(CPluginModule* changedmodule)
{
    ThrowEvent(PluginDatabase_Changed,changedmodule);
}

//---------------------------------------------------------------------------

CPluginObject* CPluginDatabase::FindPluginObject(const CUUID& object_uuid)
{
    CSimpleIterator<CPluginObject> O(PluginObjects);
    CPluginObject* p_pod;

    while((p_pod = O.Current())!=NULL) {
        if( p_pod->GetObjectUUID() == object_uuid ) {
            return(p_pod);
        }
        O++;
    }

    // object module is not attached yet
    return(NULL);
}

//---------------------------------------------------------------------------

CPluginModule* CPluginDatabase::FindPluginModule(const CUUID& module_uuid)
{
    CSimpleIterator<CPluginModule> O(PluginModules);
    CPluginModule* p_pod;

    while((p_pod = O.Current())!=NULL) {
        if( p_pod->GetModuleUUID() == module_uuid ) {
            return(p_pod);
        }
        O++;
    }

    // module is not attached yet
    return(NULL);
}

//---------------------------------------------------------------------------

CXMLElement* CPluginDatabase::GetRootConfigElement(void) const
{
    CXMLElement*   p_ele;
    p_ele = PluginConfig.GetFirstChildElement("PLUGINS");
    return(p_ele);
}

//---------------------------------------------------------------------------

CXMLElement* CPluginDatabase::GetModuleElement(const CUUID& module_uuid) const
{
    CXMLElement*   p_ele;
    p_ele = PluginConfig.GetFirstChildElement("PLUGINS");
    CXMLIterator   E(p_ele);
    CXMLElement*   p_sele;

    while( (p_sele = E.GetNextChildElement("PLUGIN")) != NULL ) {
        CSmallString uuid;
        p_sele->GetAttribute("uuid",uuid);
        CExtUUID ext_uuid;
        ext_uuid.LoadFromString(uuid);
        if( ext_uuid == module_uuid ) return(p_sele);
    }

    CSmallString error;
    error = "module " + module_uuid.GetStringForm() + " does not have record in plugin database";
    ES_ERROR(error);

    return(NULL);
}

//---------------------------------------------------------------------------

CXMLElement* CPluginDatabase::GetObjectElement(const CUUID& object_uuid) const
{
    CXMLElement*   p_ele;
    p_ele = PluginConfig.GetFirstChildElement("PLUGINS");
    CXMLIterator   E(p_ele);
    CXMLElement*   p_sele;

    while( (p_sele = E.GetNextChildElement("PLUGIN")) != NULL ) {
        CXMLIterator    S(p_sele);
        CXMLElement*    p_tele;
        while( (p_tele = S.GetNextChildElement("OBJECT")) != NULL ) {
            CSmallString uuid;
            p_tele->GetAttribute("uuid",uuid);
            CExtUUID ext_uuid;
            ext_uuid.LoadFromString(uuid);
            if( ext_uuid == object_uuid ) return(p_tele);
        }
    }

    CSmallString error;
    error = "object " + object_uuid.GetStringForm() + " does not have record in plugin database";
    ES_ERROR(error);

    return(NULL);
}

//---------------------------------------------------------------------------

CXMLElement* CPluginDatabase::GetObjectElement(CPluginObject* p_pod) const
{
    if( p_pod == NULL ) return(NULL);
    return( p_pod->ObjectConfig );
}

//---------------------------------------------------------------------------

bool CPluginDatabase::FindObjectModule(const CUUID& object_uuid,CUUID& module_uuid)
{
    CXMLElement*   p_ele;
    p_ele = PluginConfig.GetFirstChildElement("PLUGINS");
    CXMLIterator   E(p_ele);
    CXMLElement*   p_sele;

    while( (p_sele = E.GetNextChildElement("PLUGIN")) != NULL ) {
        CXMLIterator    S(p_sele);
        CXMLElement*    p_tele;
        while( (p_tele = S.GetNextChildElement("OBJECT")) != NULL ) {
            CSmallString uuid;
            p_tele->GetAttribute("uuid",uuid);
            CExtUUID ext_uuid;
            ext_uuid.LoadFromString(uuid);
            if( ext_uuid == object_uuid ) {
                CSmallString muuid;
                p_sele->GetAttribute("uuid",muuid);
                CExtUUID mext_uuid;
                mext_uuid.LoadFromString(muuid);
                module_uuid = mext_uuid;
                return( module_uuid.IsValidUUID() );
            }
        }
    }

    CSmallString error;
    error = "object " + object_uuid.GetStringForm() + " does not have module";
    ES_ERROR(error);

    return(false);
}

//---------------------------------------------------------------------------

bool CPluginDatabase::AttachModule(CUUID& module_uuid)
{
    CPluginModule* p_module = FindPluginModule(module_uuid);
    if( p_module != NULL ) {
        if( p_module->GetModuleName() != NULL ) return(true);
    }

    CXMLElement* p_ele = GetModuleElement(module_uuid);
    if( p_ele == NULL ) return(false);

    // get plugin whole name
    CFileName      name,path,wholename;
    if( p_ele->GetAttribute("name",name) == false ) {
        CSmallString error;
        error << "'name' attribute is not specified for " << module_uuid.GetStringForm();
        ES_ERROR(error);
        return(false);
    }
    p_ele->GetAttribute("path",path);
    if( path == NULL ) {
        path = PluginsPath;
    }

    CFileName prefix = CSystemFS::GetLibraryPrefix();
    CFileName affix  = CSystemFS::GetLibraryAffix();
    wholename = path / prefix + name + affix;

    // temporarily load module
    CDynamicPackage* p_package = new CDynamicPackage;

    if( p_package->Open(wholename) == false ) {
        delete p_package;
        CSmallString error;
        error << "module " << module_uuid.GetStringForm() << " with name '"
              << wholename << "' cannot be loaded";
        ES_ERROR(error);
        return(false);
    }

    // now try to find module
    p_module = FindPluginModule(module_uuid);

    bool result;

    if( p_module != NULL ) {
        result = p_module->InitializeModule(p_package,p_ele);
    } else {
        CSmallString error;
        error = "module " + module_uuid.GetStringForm() + " was not loaded to database";
        ES_ERROR(error);
        result = false;
    }

    PluginPackages.InsertToEnd(p_package,0,true);

    NotifyDescriptionChange(p_module);

    return(result);
}

//---------------------------------------------------------------------------

bool CPluginDatabase::DetachModule(CUUID& module_uuid)
{
    CPluginModule* p_module = FindPluginModule(module_uuid);
    if( p_module == NULL ) return(false);

    // finalize module
    p_module->FinalizeModule();

    // unload module
    CDynamicPackage* p_package = p_module->GetDynamicPackage();
    if( p_package != NULL ) {
        PluginPackages.Remove(p_package); // it automatically unload module
    }

    NotifyDescriptionChange(p_module);

    return(true);
}

//---------------------------------------------------------------------------

