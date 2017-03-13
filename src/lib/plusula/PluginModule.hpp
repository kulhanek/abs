#ifndef PluginModuleH
#define PluginModuleH
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
#include <SmallString.hpp>
#include <DynamicPackage.hpp>
#include <SimpleList.hpp>
#include <ExtUUID.hpp>
#include <PluginObject.hpp>

//------------------------------------------------------------------------------

enum EModuleRequest {
    EMR_INITIALIZE,     // initialize module
    EMR_FINALIZE        // finalize module
};

//------------------------------------------------------------------------------

typedef bool (*TModuleCallbackFunction)(EModuleRequest request,void* p_data);

//------------------------------------------------------------------------------

class PLUSULA_PACKAGE CPluginModule {
public:
// constructor and destructor -------------------------------------------------
    CPluginModule(const CExtUUID& moduleuuid,
                  TModuleCallbackFunction callbackfce);
    ~CPluginModule(void);

    bool InitializeModule(CDynamicPackage* p_package,
                          CXMLElement* p_moduleconfig);
    bool FinalizeModule(void);

// basic information about module ---------------------------------------------
    const CSmallString&     GetModuleName(void) const;
    const CExtUUID&         GetModuleUUID(void) const;

// information methods --------------------------------------------------------
    const CSimpleList<CPluginObject>&   GetObjectList(void) const;
    CDynamicPackage*                    GetDynamicPackage(void) const;

// section of private data ----------------------------------------------------
private:
    CSimpleList<CPluginObject>  ModuleObjects;
    CSmallString                ModuleName;
    CExtUUID                    ModuleUUID;
    CSmallString                Path;
    CXMLElement*                ModuleConfig;
    TModuleCallbackFunction     ModuleCallback;
    CDynamicPackage*            ModulePackage;

    friend class CPluginDatabase;
    friend class CPluginObject;
};

//---------------------------------------------------------------------------
#endif
