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

#include <ExtUUID.hpp>
#include <PluginModule.hpp>

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

CExtUUID::CExtUUID(void)
{
}

// -----------------------------------------------------------------------------

CExtUUID::CExtUUID(const CSmallString& extuuid)
{
    LoadFromString(extuuid);
}

// -----------------------------------------------------------------------------

CExtUUID::CExtUUID(const CSmallString& extuuid,const CSmallString& name)
{
    LoadFromString(extuuid);
    Name = name;
}

// -----------------------------------------------------------------------------

CExtUUID::CExtUUID(const CSmallString& extuuid,const CSmallString& name,const CSmallString& description)
{
    LoadFromString(extuuid);
    Name = name;
    Description = description;
}

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

// {class_name:xxxxxxxx-xxxx-xxxx-xxxx-xxxxxxxxxxxx}

bool CExtUUID::LoadFromString(const CSmallString& string)
{
    int namelength=0;

    CSmallString tmp(string);

    for(unsigned int i=1; i<tmp.GetLength(); i++) {
        if( tmp.GetBuffer()[i] == ':'  ) {
            namelength = i-1;
            break;
        }
    }

    if(namelength == 0)return(false);

    UUIDName = tmp.GetSubString(1,namelength);
    return(SetFromStringForm(tmp.GetSubString(namelength+2,tmp.GetLength()-3-namelength)));
}

// -----------------------------------------------------------------------------

const CSmallString CExtUUID::GetFullStringForm(void) const
{
    CSmallString string;
    string = "{" + UUIDName + ":" + GetStringForm() + "}";
    return(string);
}

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

const CSmallString& CExtUUID::GetUUIDName(void) const
{
    return(UUIDName);
}

// -----------------------------------------------------------------------------

const CSmallString& CExtUUID::GetName(void) const
{
    return(Name);
}

// -----------------------------------------------------------------------------

const CSmallString& CExtUUID::GetDescription(void) const
{
    return(Description);
}

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================
