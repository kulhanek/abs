#ifndef AliasH
#define AliasH
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
#include <ResourceList.hpp>
#include <boost/shared_ptr.hpp>
#include <iostream>

// -----------------------------------------------------------------------------

class CXMLElement;

// -----------------------------------------------------------------------------

class ABS_PACKAGE CAlias {
public:
// constructor -----------------------------------------------------------------
        CAlias(void);
        CAlias(const CSmallString& name,const CSmallString& queue,const CSmallString& resources);

// executive methods -----------------------------------------------------------
    /// test alias
    bool TestAlias(std::ostream& sout);

// information methods ---------------------------------------------------------
    //! print line info about the node
    void PrintLineInfo(std::ostream& sout);

    //! is system alias
    bool IsSystemAlias(void) const;

    //! get name
    const CSmallString& GetName(void) const;

    //! get destination
    const CSmallString& GetDestination(void) const;

    //! get resources
    const CResourceList& GetResources(void) const;

// i/o methods -----------------------------------------------------------------
    //! load alias
    bool Load(CXMLElement* p_ele,bool system);

    //! save alias
    void Save(CXMLElement* p_ele);

// section of private data -----------------------------------------------------
private:
    bool            SystemAlias;
    CSmallString    Name;
    CSmallString    Destination;
    CResourceList   Resources;
};

// -----------------------------------------------------------------------------

typedef boost::shared_ptr<CAlias>  CAliasPtr;

// -----------------------------------------------------------------------------

#endif
