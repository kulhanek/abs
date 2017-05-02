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

class CJob;

// -----------------------------------------------------------------------------

/// list of resources

class ABS_PACKAGE CResourceList : public std::list<CResourceValuePtr> {
public:
// constructor -----------------------------------------------------------------
        CResourceList(void);
        ~CResourceList(void);

// executive methods -----------------------------------------------------------
    /// set job
    void SetJob(CJob* p_job);

    /// get job
    CJob* GetJob(void);

    /// add resources - override existing setup
    void AddResources(const CSmallString& reslist,std::ostream& sout,bool& rstatus, bool expertmode);

    /// add a single resource
    void AddResource(const CSmallString& name,const CSmallString& value,std::ostream& sout,bool& rstatus, bool expertmode);

    /// add a single resource - always in expert mode
    /// value is kB
    void AddSizeResource(const CSmallString& name,long long value);

    /// add a single resource
    void AddRawResource(const CSmallString& name,const CSmallString& value);

    /// add a single resource
    CResourceValuePtr AddEmptyResource(const CSmallString& name, bool expertmode,bool do_not_remove);

    /// find resource
    CResourceValuePtr FindResource(const CSmallString& name) const;

    /// remove resource
    void RemoveResource(const CSmallString& name);

    /// sort by name
    void SortByName(void);

// expected order of execution ....
    /// resolve conflicts
    void ResolveConflicts(const CSmallString& short_server_name);

    /// test all resources
    void TestResourceValues(std::ostream& sout,bool& rstatus);

    /// resolve dynamic resources
    void ResolveDynamicResources(void);

// information methods ---------------------------------------------------------
    /// get resource value
    const CSmallString GetResourceValue(const CSmallString& name) const;

    /// print resources to the string
    const CSmallString ToString(bool include_spaces) const;

    /// get number of CPUs
    int GetNumOfCPUs(void) const;

    /// get number of GPUs
    int GetNumOfGPUs(void) const;

    /// get number of nodes
    int GetNumOfNodes(void) const;

    /// get memory in kB
    long long GetMemory(void) const;

    /// get memory in variable unit
    const CSmallString GetMemoryString(void) const;

    /// get worksize
    const CSmallString GetWorkSizeString(void) const;

    /// get walltime
    const CSmallString GetWallTimeString(void) const;

    /// get list of variables
    const CSmallString GetVariables(void) const;

// section of private data -----------------------------------------------------
private:
    CJob*       Job;

    /// helper method
    static bool SortCompName(const CResourceValuePtr& p_left,const CResourceValuePtr& p_right);

    friend class CJob;
};

// -----------------------------------------------------------------------------

#endif
