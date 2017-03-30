#ifndef ResourceListH
#define ResourceListH
// =============================================================================
// ABS - Advanced Batch System
// -----------------------------------------------------------------------------
//    Copyright (C) 2017 Petr Kulhanek, kulhanek@chemi.muni.cz
//    Copyright (C) 2011-2012 Petr Kulhanek, kulhanek@chemi.muni.cz
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
#include <SmallString.hpp>
#include <ResourceValue.hpp>
#include <list>
#include <SimpleList.hpp>
#include <map>
#include <string>
#include <SmallTime.hpp>

// -----------------------------------------------------------------------------

/// list of resources

class ABS_PACKAGE CResourceList : private std::list<CResourceValuePtr> {
public:
// constructor -----------------------------------------------------------------
        CResourceList(void);
        ~CResourceList(void);

// executive methods -----------------------------------------------------------
    /// add resources - override existing setup
    bool AddResources(const CSmallString& reslist);

    /// add resources - override existing setup
    void AddResources(const CSmallString& reslist,std::ostream& sout,bool& rstatus);

    /// add a single resource
    void AddResource(const CSmallString& name,const CSmallString& value,std::ostream& sout,bool& rstatus);

    /// add a specific resource
    void AddResource(const CResourceValuePtr& res);

    /// find resource
    CResourceValuePtr FindResource(const CSmallString& name) const;

    /// remove resource
    void RemoveResource(const CSmallString& name);

    /// remove all resources
    void RemoveAllResources(void);

    /// sort by name
    void SortByName(void);

    /// resolve conflicts
    void ResolveConflicts(void);

    /// test all resources
    void TestResourceValues(std::ostream& sout,bool& rstatus);

    /// finalize resources
    void FinalizeResources(void);

// information methods ---------------------------------------------------------
    /// get resource value
    const CSmallString GetResourceValue(const CSmallString& name) const;

    /// print resources to the string
    const CSmallString ToString(bool include_spaces) const;

    /// get number of CPUs
    int GetNumOfCPUs(void) const;

    /// get number of GPUs
    int GetNumOfGPUs(void) const;

    /// get maximum number of CPUS per node
    int GetMaxNumOfCPUsPerNode(void) const;

    /// get number of nodes
    int GetNumOfNodes(void) const;

    /// get memory in Bytes
    long int GetMemory(void) const;

    /// get walltime
    bool GetWallTime(CSmallTime& time) const;

// section of private data -----------------------------------------------------
private:
    /// helper method
    static bool SortCompName(const CResourceValuePtr& p_left,const CResourceValuePtr& p_right);

    friend class CJob;
};

// -----------------------------------------------------------------------------

extern CResourceList ResourceList;

// -----------------------------------------------------------------------------

#endif
