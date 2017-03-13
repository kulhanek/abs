#ifndef ResourceListH
#define ResourceListH
// =============================================================================
// ABS - Advanced Batch System
// -----------------------------------------------------------------------------
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

class ABS_PACKAGE CResourceList : public std::list<CResourceValuePtr> {
public:
// constructor -----------------------------------------------------------------
        CResourceList(void);
        ~CResourceList(void);

// executive methods -----------------------------------------------------------
    /// parse resources string, do not check for errors
    void Parse(const CSmallString& resources);

    /// merge resources
    void Merge(const CResourceList& source);

    /// test resources
    void TestResources(std::ostream& sout,bool& rstatus);

    /// test resources
    bool TestResources(std::ostream& sout);

    /// test for duplicate occurences
    void TestDuplicateResources(std::ostream& sout,bool& rstatus);

    /// add new resource to the begin
    void AddResourceToBegin(const CSmallString& name,const CSmallString& value);

    /// remove all resources
    void RemoveAllResources(void);

    /// remove all resources
    void RemoveHelperResources(void);

// information methods ---------------------------------------------------------
    /// print resources to the string
    const CSmallString ToString(bool include_spaces) const;

    /// get number of CPUs
    int GetNumOfCPUs(void) const;

    /// get number of GPUs
    int GetNumOfGPUs(void) const;

    /// get maximum number of CPUS per node
    int GetMaxNumOfCPUsPerNode(void) const;

    /// find resource with given name
    CResourceValuePtr GetResource(const CSmallString& name) const;

    /// get resource value
    CSmallString GetResourceValue(const CSmallString& name) const;

    /// get memory in Bytes
    long int GetMemory(void) const;

    /// get walltime
    bool GetWallTime(CSmallTime& time) const;

    /// get torque resources
    void GetTorqueResources(struct attropl* &p_prev);

    /// sort by name
    void SortByName(void);

    /// finalize resources
    void Finalize(void);

// section of public data ------------------------------------------------------
    /// resource variables
    std::map<std::string,std::string> Variables;

// section of private data -----------------------------------------------------
private:
    /// helper method
    static bool SortCompName(const CResourceValuePtr& p_left,const CResourceValuePtr& p_right);
};

// -----------------------------------------------------------------------------

#endif
