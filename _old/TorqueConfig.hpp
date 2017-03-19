#ifndef TorqueConfigH
#define TorqueConfigH
// =============================================================================
// ABS - Advanced Batch System
// -----------------------------------------------------------------------------
//    Copyright (C) 2011      Petr Kulhanek, kulhanek@chemi.muni.cz
//    Copyright (C) 2001-2008 Petr Kulhanek, kulhanek@chemi.muni.cz
//
//     This library is free software; you can redistribute it and/or
//     modify it under the terms of the GNU Lesser General Public
//     License as published by the Free Software Foundation; either
//     version 2.1 of the License, or (at your option) any later version.
//
//     This library is distributed in the hope that it will be useful,
//     but WITHOUT ANY WARRANTY; without even the implied warranty of
//     MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
//     Lesser General Public License for more details.
//
//     You should have received a copy of the GNU Lesser General Public
//     License along with this library; if not, write to the Free Software
//     Foundation, Inc., 51 Franklin Street, Fifth Floor,
//     Boston, MA  02110-1301  USA
// =============================================================================

#include <ABSMainHeader.hpp>
#include <XMLDocument.hpp>
#include <FileName.hpp>
#include <vector>
#include <UUID.hpp>

//------------------------------------------------------------------------------

enum EAliasLevel {
    EAL_MERGED,
    EAL_USER,
    EAL_SYSTEM
    };


// -----------------------------------------------------------------------------

// torque driver mode
enum ETorqueMode{
    ETM_TORQUE = 0,         // standard torque
    ETM_TORQUE_METAVO = 1,  // metacentrum torque
    ETM_PBSPRO = 2          // PBSPro
};

//------------------------------------------------------------------------------

class ABS_PACKAGE CTorqueConfig {
public:
// constructor -----------------------------------------------------------------
            CTorqueConfig(void);
           ~CTorqueConfig(void);

    /// LoadConfig
    bool LoadConfig(void);

// system setup ----------------------------------------------------------------
    bool LoadSystemConfig(void);

    /// load site configuration
    bool LoadSystemConfig(const CSmallString& site_sid);

// user setup ------------------------------------------------------------------
    /// load user config
    bool LoadUserConfig(void);

    /// save user config
    bool SaveUserConfig(void);

    /// remove user config
    bool RemoveUserConfig(void);

// executive methods -----------------------------------------------------------
    /// have user valid ticket
    bool IsUserTicketValid(std::ostream& sout);

// configuration methods -------------------------------------------------------
    /// return system configuration value
    bool GetSystemConfigItem(const CSmallString& item_name,
                             CSmallString& item_value);

    /// return system configuration value
    const CSmallString GetSystemConfigItem(const CSmallString& item_name);

    /// return user configuration value
    bool GetUserConfigItem(const CSmallString& item_name,
                           CSmallString& item_value);

    /// return user configuration value
    bool GetUserConfigItem(const CSmallString& item_name,
                           CSmallString& item_value,bool& system);

    /// set user configuration value
    void SetUserConfigItem(const CSmallString& item_name,
                           const CSmallString& item_value);

    /// get plugin config item
    bool GetSystemConfigItem(const CUUID& pobject, const CSmallString& item_name,
                             CSmallString& item_value);

    /// get plugin config item
    const CSmallString GetSystemConfigItem(const CUUID& pobject, const CSmallString& item_name);

    /// get server type
    const CSmallString GetServerType(void);

    /// get server name
    const CSmallString GetServerName(void);

    /// get node group configuration
    CXMLElement* GetNodeGroupConfig(void);

    /// print allowed sync modes
    void PrintAllowedSyncModes(std::ostream& sout);

    /// print allowed res tokens
    void PrintAllowedResTokens(std::ostream& sout);

    /// print allowed res tokens
    bool TestResourceTokens(const CSmallString& name);

    //! return Torque mode
    ETorqueMode GetTorqueMode(void) const;

    //! return scratch dir specification
    const CSmallString GetScratchDir(const CSmallString& type,const CSmallString& mode);

    //! return surrogate machine
    const CSmallString GetSurrogateMachine(const CSmallString& machine,const CSmallString& jobdir);

    //! return surrogate mapping user group
    bool GetSurrogateGroup(const CSmallString& machine,const CSmallString& jobdir,CSmallString& ugroup,CSmallString& ugroup_realm);

    //! test scratch type
    bool TestScratchType(const CSmallString& name,bool& pass);

    //! get default scratch type
    const CSmallString GetDefaultScratchType(void);

    //! get allowed scratch types
    void PrintAllowedScratchTypes(std::ostream& sout);

// section of private data -----------------------------------------------------
private:
    CXMLDocument    SystemConfig;
    CXMLDocument    UserConfig;
    ETorqueMode     Mode;

    bool GetSystemConfigItem(CXMLElement* p_root, const CSmallString& item_name,
                             CSmallString& item_value);
};

//------------------------------------------------------------------------------

extern CTorqueConfig TorqueConfig;

//------------------------------------------------------------------------------

#endif
