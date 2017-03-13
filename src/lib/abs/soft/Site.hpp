#ifndef SiteH
#define SiteH
// =============================================================================
// ABS - Advanced Batch System
// -----------------------------------------------------------------------------
//     Copyright (C) 2012 Petr Kulhanek (kulhanek@chemi.muni.cz)
//     Copyright (C) 2011 Petr Kulhanek (kulhanek@chemi.muni.cz)
//     Copyright (C) 2004,2005,2008,2010 Petr Kulhanek (kulhanek@chemi.muni.cz)
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
#include <SmallString.hpp>
#include <XMLDocument.hpp>
#include <XMLNode.hpp>

//------------------------------------------------------------------------------

class ABS_PACKAGE CSite {
public:
    // constructor and destructor ------------------------------------------------
    CSite(void);
    ~CSite(void);

    // input methods --------------------------------------------------------------
    /// load active site configuration
    bool LoadConfig(void);

    /// load site configuration
    bool LoadConfig(const CSmallString& site_sid);

    // information methods --------------------------------------------------------
    /// return name of the site
    const CSmallString GetName(void) const;

    /// return group desc of the site
    const CSmallString GetGroupDesc(void) const;

    /// return ID of the site
    const CSmallString GetID(void);

    /// return organization
    const CSmallString GetOrganizationName(void);

    /// return organization
    const CSmallString GetOrganizationURL(void);

    /// return name of site support name
    const CSmallString GetSupportName(void);

    /// return support contact email
    const CSmallString GetSupportEMail(void);

    /// return documentation text
    const CSmallString GetDocumentationText(void);

    /// return documentation url
    const CSmallString GetDocumentationURL(void);

    /// return long site description
    CXMLNode* GetSiteDescrXML(void);

    /// return short site description
    CXMLElement* GetAutoloadedModules(void);

    /// is site active?
    bool IsActive(void);

    /// can be site activated on this computer?
    bool CanBeActivated(bool allow_localhost=false);

    /// can be site activated on this computer?
    bool CanBeActivated(int& priority,bool allow_localhost);

    // print information about site -----------------------------------------------
    /// print short info about site
    void PrintShortSiteInfo(std::ostream& vout);

    // executive methods ----------------------------------------------------------
    /// activate site
    bool ActivateSite(void);

    /// deactivate site
    bool DeactivateSite(void);

    /// do postactions
    bool ExecuteModaction(const CSmallString& action,const CSmallString& args);

    // section of private data ----------------------------------------------------
private:
    CSmallString    SiteID;             // site id
    CXMLDocument    SiteConfig;         // site specific configuration
    CSmallString    ActiveModules;      // active modules
    CSmallString    ExportedModules;    // exported modules

    void PrintAutoloadedModules(FILE* fout,CXMLElement* p_mod_ele);
    bool ActivateAutoloadedModules(CXMLElement* p_mod_ele);
};

//------------------------------------------------------------------------------

extern CSite    Site;

//------------------------------------------------------------------------------

#endif
