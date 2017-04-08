#ifndef AliasListH
#define AliasListH
// =============================================================================
// ABS - Advanced Batch System
// -----------------------------------------------------------------------------
//    Copyright (C) 2011      Petr Kulhanek, kulhanek@chemi.muni.cz
//    Copyright (C) 2001-2008 Petr Kulhanek, kulhanek@chemi.muni.cz
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

#include <ABSMainHeader.hpp>
#include <Alias.hpp>
#include <list>

// -----------------------------------------------------------------------------

class ABS_PACKAGE CAliasList : private std::list<CAliasPtr> {
public:
// constructor -----------------------------------------------------------------
    CAliasList(void);
    ~CAliasList(void);

// input/output methods --------------------------------------------------------
    /// load all aliases
    bool LoadConfig(void);

    /// save user aliases
    bool SaveUserConfig(void);

    /// load system aliases
    bool LoadSystemConfig(void);

    /// load user aliases
    bool LoadUserConfig(void);

// executive methods -----------------------------------------------------------
    /// add new alias
    bool AddAlias(std::ostream& sout,bool expertmode,
                  const CSmallString& name,const CSmallString& queue,
                  const CSmallString& resources);

    /// remove the alias
    void RemoveAlias(const CSmallString& name);

    /// clear all aliases
    void ClearAllAliases(void);

// information methods ---------------------------------------------------------
    /// find alias
    CAliasPtr FindAlias(const CSmallString& name);

    /// print info about all aliases
    void PrintInfos(std::ostream& sout);

    /// get aliases suggestions
    void GetAliasesSuggestions(std::list<CSmallString>& suggestions);

// section of private data ----------------------------------------------------
private:
    /// load aliases
    bool LoadAliases(CXMLElement* p_ele,bool system);

    /// save only user aliases
    bool SaveUserAliases(CXMLElement* p_ele);

    /// compare two queues by name
    static bool SortCompName(const CAliasPtr& left,const CAliasPtr& right);
};

// -----------------------------------------------------------------------------

extern CAliasList AliasList;

// -----------------------------------------------------------------------------

#endif
