#ifndef NodeH
#define NodeH
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
#include <SmallString.hpp>
#include <boost/shared_ptr.hpp>
#include <vector>
#include <list>
#include <set>
#include <iostream>

// -----------------------------------------------------------------------------

class CNode;
typedef boost::shared_ptr<CNode>  CNodePtr;

// -----------------------------------------------------------------------------

/// computational node

class ABS_PACKAGE CNode {
public:
// constructor -----------------------------------------------------------------
        CNode(void);

// methods ---------------------------------------------------------------------
    /// init node with torque information
//    bool Init(struct batch_status* p_node);

    /// print line info about the node
    void PrintLineInfo(std::ostream& sout,const std::set<std::string>& gprops,int ncolumns);

    /// get node name
    const CSmallString& GetName(void) const;

    /// get number of CPUs
    int GetNumOfCPUs(void) const;

    /// return number of free CPUs
    int GetNumOfFreeCPUs(void) const;

    /// get number of GPUs
    int GetNumOfGPUs(void) const;

    /// return number of free GPUs
    int GetNumOfFreeGPUs(void) const;

    /// get property list
    const std::vector<std::string>& GetPropertyList(void) const;

    /// has node given property
    bool HasProperty(const std::string& prop);

    /// has node all given properties
    bool HasAllProperties(const std::vector<std::string>& props);

    /// has node any of given properties
    bool HasAnyProperties(const std::vector<std::string>& props);

    /// get extra properties
    const std::set<std::string> GetExtraProperties(const std::set<std::string>& props);

    /// get node type
    const CSmallString& GetType(void) const;

    /// get state list
    const std::vector<std::string> GetStateList(void) const;

    /// get node type
    bool IsDown(void) const;

// section of private data -----------------------------------------------------
private:
    CSmallString                Name;
    int                         NCPUs;
    int                         FreeCPUs;
    int                         NGPUs;
    int                         FreeGPUs;
    int                         MaxCPUsPerNode; // for merged NUMA nodes
    CSmallString                State;
    CSmallString                Jobs;
    std::vector<std::string>    JobList;
    CSmallString                Properties;
    std::vector<std::string>    PropList;
    CSmallString                Type;
};

// -----------------------------------------------------------------------------

#endif
