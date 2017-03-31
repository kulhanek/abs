#ifndef ResourceValueH
#define ResourceValueH
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
#include <boost/shared_ptr.hpp>
#include <ComObject.hpp>
#include <map>
#include <string>

// -----------------------------------------------------------------------------

class CResourceList;

// -----------------------------------------------------------------------------

/// resource key and its value

class ABS_PACKAGE CResourceValue : public CComObject {
public:
// constructor -----------------------------------------------------------------
        CResourceValue(CPluginObject* p_objectinfo);

// information methods ---------------------------------------------------------
    /// test value if it is in expected range
    virtual void TestValue(CResourceList* p_rl,std::ostream& sout,bool& rstatus);

    /// resolve conflicts
    virtual void ResolveConflicts(CResourceList* p_rl);

    /// resolve dynamic resource
    virtual void ResolveDynamicResource(CResourceList* p_rl);

    /// get variable
    virtual void GetVariable(CSmallString& name, CSmallString& value);

    /// get batch attribute
    virtual void GetAttribute(CSmallString& name, CSmallString& resource, CSmallString& value);

    /// get size in kB from Value
    long long GetSize(void);

    /// set size in kB
    void SetSize(long long size);

    /// get number from Value
    long long GetNumber(void);

// section of private data -----------------------------------------------------
protected:
    CSmallString    Name;           // derived from plusula object configuration
    CSmallString    Value;

    /// test value if it is in expected range (number[b][kb][mb][gb][tb])
    bool TestSizeValue(CResourceList* p_rl,std::ostream& sout,bool& rstatus);

    /// test value if it is in expected range (number)
    bool TestNumberValue(CResourceList* p_rl,std::ostream& sout,bool& rstatus);

    friend class CResourceList;
};

// -----------------------------------------------------------------------------
#include <map>
#include <string>
typedef boost::shared_ptr<CResourceValue>  CResourceValuePtr;

// -----------------------------------------------------------------------------

#endif
