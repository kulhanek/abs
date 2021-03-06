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
#include <vector>

// -----------------------------------------------------------------------------

class CResourceList;

// -----------------------------------------------------------------------------

/// resource key and its value

class ABS_PACKAGE CResourceValue : public CComObject {
public:
// constructor -----------------------------------------------------------------
        CResourceValue(CPluginObject* p_objectinfo);

// information methods ---------------------------------------------------------
    /// test value if it is in expected range - initial test
    virtual void PreTestValue(CResourceList* p_rl,std::ostream& sout,bool& rstatus);

    /// test value if it is in expected range - final test after all resources are completed
    virtual void PostTestValue(CResourceList* p_rl,std::ostream& sout,bool& rstatus);

    /// resolve conflicts
    virtual void ResolveConflicts(CResourceList* p_rl);

    /// resolve dynamic resource
    virtual void ResolveDynamicResource(CResourceList* p_rl,bool& delete_me);

    /// get variable
    virtual void GetVariable(CSmallString& name, CSmallString& value);

    /// get batch attribute
    virtual void GetAttribute(CSmallString& name, CSmallString& resource, CSmallString& value);

    /// get size in kB from Value
    long long GetSize(void);

    /// get size in kB from Value
    static long long GetSize(const CSmallString& value);

    /// get size in variable unit
    CSmallString GetSizeString(void);

    /// get size in variable unit
    static CSmallString GetSizeString(long long size);

    /// set size in kB
    void SetSize(long long size);

    /// get number from Value
    long long GetNumber(void);

    /// get float number from Value
    double GetFloatNumber(void);

    /// get value
    const CSmallString& GetValue(void);

    /// does it provide specified resource?
    bool DoesItProvide(const CSmallString& name);

// section of private data -----------------------------------------------------
protected:
    CSmallString                Server;         // batch server - short server name
    CSmallString                Name;           // derived from plusula object configuration
    CSmallString                Value;
    // dependencies for ResolveDynamicResource
    std::vector<CSmallString>   Requires;
    std::vector<CSmallString>   Provides;

    /// test value if it is in expected range (number[b][kb][mb][gb][tb])
    bool TestSizeValue(std::ostream& sout,bool& rstatus);

    /// test value if it is in expected range (number)
    bool TestNumberValue(std::ostream& sout,bool& rstatus);

    /// test value if it is in expected range (key)
    bool TestKeyValue(std::ostream& sout,bool& rstatus,const CSmallString& keys);

    friend class CResourceList;
};

// -----------------------------------------------------------------------------
#include <map>
#include <string>
typedef boost::shared_ptr<CResourceValue>  CResourceValuePtr;

// -----------------------------------------------------------------------------

#endif
