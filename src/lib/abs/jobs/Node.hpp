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
#include <map>
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
    /// print line info about the node
    void PrintLineInfo(std::ostream& sout,const std::set<std::string>& gprops,int ncolumns);

    /// print jobs on the node
    void PrintJobsInfo(std::ostream& sout);

    /// get node name
    const CSmallString& GetName(void) const;

    /// get host name
    const CSmallString& GetHost(void) const;

    //! get short server name
    const CSmallString GetShortServerName(void);

    /// get number of CPUs
    int GetNumOfCPUs(void) const;

    /// return number of free CPUs
    int GetNumOfFreeCPUs(void) const;

    /// get number of GPUs
    int GetNumOfGPUs(void) const;

    /// return number of free GPUs
    int GetNumOfFreeGPUs(void) const;

    /// return total memory in kb
    long long GetMemory(void) const;

    /// return free memory in kb
    long long  GetFreeMemory(void) const;

    /// get property list
    const std::vector<std::string>& GetPropertyList(void) const;

    /// get all properties
    const std::vector<std::string>& GetAllProps(void) const;

    /// has node given property
    bool HasProperty(const std::string& prop);

    /// has node all given properties
    bool HasAllProperties(const std::vector<std::string>& props);

    /// has node any of given properties
    bool HasAnyProperties(const std::vector<std::string>& props);

    /// has node any of given properties
    bool IsInAnyQueueWithServer(const std::vector<std::string>& qlist);

    /// has node any of given properties
    bool IsInQueueWithServer(const std::string& queue);

    /// get extra properties
    const std::set<std::string> GetExtraProperties(const std::set<std::string>& props);

    /// get node type
    const CSmallString& GetType(void) const;

    /// get state list
    const std::vector<std::string> GetStateList(void) const;

    /// get node type
    bool IsDown(void) const;

    /// get compressed status
    char GetStateCode(void);

// section of private data -----------------------------------------------------
protected:
    CSmallString                BatchServerName;
    CSmallString                ShortServerName;

    CSmallString                Name;
    CSmallString                MOM;
    CSmallString                Comment;

    int                         NCPUs;
    int                         AssignedCPUs;

    int                         NGPUs;
    int                         AssignedGPUs;

    size_t                      Memory;
    size_t                      AssignedMemory;

    size_t                      ScratchLocal;
    size_t                      ScratchShared;
    size_t                      ScratchSSD;

    CSmallString                State;
    CSmallString                Type;

    CSmallString                Jobs;
    std::set<std::string>       JobList;        // unique list of jobs
    std::map<std::string, std::vector<int> >  JobSlots;

    CSmallString                Properties;
    std::vector<std::string>    PropList;
    std::vector<std::string>    AllPropList;

    CSmallString                Queues;
    std::vector<std::string>    QueueList;

    const std::string GetNiceSize(size_t size);
};

// -----------------------------------------------------------------------------

#endif
