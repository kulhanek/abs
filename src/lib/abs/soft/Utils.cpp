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

#include <Utils.hpp>
#include <string.h>
#include <Site.hpp>
#include <DirectoryEnum.hpp>
#include <FileName.hpp>
#include <AmsUUID.hpp>
#include <errno.h>
#include <ErrorSystem.hpp>
#include <XMLIterator.hpp>
#include <iomanip>
#include <list>
#include <GlobalConfig.hpp>
#include <UUID.hpp>

//------------------------------------------------------------------------------

using namespace std;

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

const CSmallString CUtils::GetSiteID(const CSmallString& site_name)
{
    // go through the list of all available sites -------------
    CFileName ams_sites;
    ams_sites = GlobalConfig.GetAMSRootDir() / "etc" / "sites";

    CDirectoryEnum dir_enum(ams_sites);

    dir_enum.StartFindFile("*");
    CFileName site_sid;
    while( dir_enum.FindFile(site_sid) ) {
        CAmsUUID    site_id;
        CSite       site;
        if( site_id.LoadFromString(site_sid) == false ) continue;
        if( site.LoadConfig(site_sid) == false ) continue;

        // site was found - return site id
        if( site.GetName() == site_name ) return(site.GetID());
    }
    dir_enum.EndFindFile();

    // site was not found or it is not correctly configured
    return("");
}

//------------------------------------------------------------------------------

const CSmallString CUtils::GetSiteName(const CSmallString& site_sid)
{
    if( site_sid == NULL ) return("");
    CSite   site;
    if( site.LoadConfig(site_sid) == false ){
        return("");
    }
    return(site.GetName());
}

//------------------------------------------------------------------------------

bool CUtils::IsSiteIDValid(const CSmallString& site_id)
{
    CSite   site;
    return(site.LoadConfig(site_id));
}

//------------------------------------------------------------------------------

CSmallString CUtils::GenerateUUID(void)
{
    CUUID my_uuid;
    my_uuid.CreateUUID();
    return(my_uuid.GetStringForm());
}

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

bool CUtils::ParseModuleName(const CSmallString& module,
        CSmallString& name,
        CSmallString& ver,
        CSmallString& arch,
        CSmallString& mode)
{
    CSmallString tmp(module);
    if( tmp == NULL ) return(false);

    char* p_saveptr = NULL;
    name = strtok_r(tmp.GetBuffer(),":",&p_saveptr);
    ver = strtok_r(NULL,":",&p_saveptr);
    arch = strtok_r(NULL,":",&p_saveptr);
    mode = strtok_r(NULL,":",&p_saveptr);

    return(true);
}

//------------------------------------------------------------------------------

bool CUtils::ParseModuleName(const CSmallString& module,
        CSmallString& name,
        CSmallString& ver,
        CSmallString& arch)
{
    CSmallString tmp(module);
    if( tmp == NULL ) return(false);

    char* p_saveptr = NULL;
    name = strtok_r(tmp.GetBuffer(),":",&p_saveptr);
    ver = strtok_r(NULL,":",&p_saveptr);
    arch = strtok_r(NULL,":",&p_saveptr);

    return(true);
}

//------------------------------------------------------------------------------

bool CUtils::ParseModuleName(const CSmallString& module,
        CSmallString& name,
        CSmallString& ver)
{
    CSmallString tmp(module);
    if( tmp == NULL ) return(false);

    char* p_saveptr = NULL;
    name = strtok_r(tmp.GetBuffer(),":",&p_saveptr);
    ver = strtok_r(NULL,":",&p_saveptr);

    return(true);
}

//------------------------------------------------------------------------------

bool CUtils::ParseModuleName(const CSmallString& module,
        CSmallString& name)
{
    CSmallString tmp(module);
    if( tmp == NULL ) return(false);

    char* p_saveptr = NULL;
    name = strtok_r(tmp.GetBuffer(),":",&p_saveptr);

    return(true);
}

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

const CSmallString CUtils::GetModuleName(const CSmallString& module)
{
    CSmallString tmp(module);
    if( tmp == NULL ) return("");

    char* p_saveptr = NULL;
    return(strtok_r(tmp.GetBuffer(),":",&p_saveptr));
}

//------------------------------------------------------------------------------

const CSmallString CUtils::GetModuleVer(const CSmallString& module)
{
    CSmallString tmp(module);
    if( tmp == NULL ) return("");

    char* p_saveptr = NULL;
    strtok_r(tmp.GetBuffer(),":",&p_saveptr);
    return(strtok_r(NULL,":",&p_saveptr));
}

//------------------------------------------------------------------------------

const CSmallString CUtils::GetModuleArch(const CSmallString& module)
{
    CSmallString tmp(module);
    if( tmp == NULL ) return("");

    char* p_saveptr = NULL;
    strtok_r(tmp.GetBuffer(),":",&p_saveptr);
    strtok_r(NULL,":",&p_saveptr);
    return(strtok_r(NULL,":",&p_saveptr));
}

//------------------------------------------------------------------------------

const CSmallString CUtils::GetModuleMode(const CSmallString& module)
{
    CSmallString tmp(module);
    if( tmp == NULL ) return("");

    char* p_saveptr = NULL;
    strtok_r(tmp.GetBuffer(),":",&p_saveptr);
    strtok_r(NULL,":",&p_saveptr);
    strtok_r(NULL,":",&p_saveptr);
    return(strtok_r(NULL,":",&p_saveptr));
}

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

