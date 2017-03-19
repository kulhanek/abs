#ifndef ResourcesH
#define ResourcesH
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
#include <vector>
#include <iostream>
#include <SmallString.hpp>

// -----------------------------------------------------------------------------
/*

ncpus[/cpuspernode][:props][,ext=value]
ncpus@node[,ext=value]

*/
// -----------------------------------------------------------------------------

class ABS_PACKAGE CResources {
public:
// constructor -----------------------------------------------------------------
        CResources(void);

// methods ---------------------------------------------------------------------
    //! parse resources
    bool Parse(const CSmallString& resources);

    //! merge with user resources
    bool MergeWithUser(const CResources& resources,std::ostream& sout);

    //! set max cpus per node
    void SetMaxCPUsPerNode(int set);

// information methods ---------------------------------------------------------
    //! get Torque resources
    const CSmallString GetTorqueResources(void);

    //! is local mode
    bool IsLocal(void);

    //! get number of CPUs
    int GetNumOfCPUs(void);

    //! get number of nodes
    int GetNumOfNodes(void);

    //! get max number of CPUs per node
    int GetMaxCPUsPerNode(void);

    //! get node name
    const CSmallString GetNodeName(void);

    //! get properties
    const CSmallString GetProperties(void);

// section of private data -----------------------------------------------------
private:
    int                         MaxCPUsPerNode;
    CSmallString                HostName;
    // parsed resources ----------------
    CSmallString                NodeName;
    int                         NCPUs;
    CSmallString                Properties;
    std::vector<CSmallString>   Extensions;
};

// -----------------------------------------------------------------------------

#endif
