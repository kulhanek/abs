#ifndef ResourceValueH
#define ResourceValueH
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
#include <boost/shared_ptr.hpp>
#include <map>
#include <string>

// -----------------------------------------------------------------------------

/// resource key and its value

class ABS_PACKAGE CResourceValue {
public:
// constructor -----------------------------------------------------------------
        CResourceValue(void);
        CResourceValue(const CSmallString& name,const CSmallString& value);

// information methods ---------------------------------------------------------
    /// test value if it is in expected range
    void TestValue(std::ostream& sout,bool& rstatus);

    /// test value if it is in expected range
    bool TestValue(std::ostream& sout);

    // aditional tests
    bool TestNCPUs(std::ostream& sout,bool& rstatus);
    bool TestNGPUs(std::ostream& sout,bool& rstatus);
    bool TestMaxCPUsPerNode(std::ostream& sout,bool& rstatus);
    bool TestProps(std::ostream& sout,bool& rstatus);
    bool TestMem(std::ostream& sout,bool& rstatus);
    bool TestVMem(std::ostream& sout,bool& rstatus);
    bool TestWallTime(std::ostream& sout,bool& rstatus);
    bool TestScratch(std::ostream& sout,bool& rstatus);
    bool TestPlace(std::ostream& sout,bool& rstatus);
    bool TestScratchType(std::ostream& sout,bool& rstatus);
    bool TestUMask(std::ostream& sout,bool& rstatus);
    bool TestGroup(std::ostream& sout,bool& rstatus);
    bool TestCPUFreq(std::ostream& sout,bool& rstatus);
    bool TestNUMA(std::ostream& sout,bool& rstatus);
    bool TestEmail(std::ostream& sout,bool& rstatus);
    bool TestHost(std::ostream& sout,bool& rstatus);  // target host in PBSPro
    bool TestSurrogate(std::ostream& sout,bool& rstatus); // surrogate machine with job data

    /// is helper token?
    bool IsHelperToken(void);

    /// finalize resource
    void Finalize(std::map<std::string,std::string>& variables);

// section of private data -----------------------------------------------------
public:
    CSmallString    Name;
    CSmallString    Value;
    bool            HelperValue;

    // NUMBERp -> hh:mm:ss
    void TransformWallTimeShortNotation(void);
};

// -----------------------------------------------------------------------------
#include <map>
#include <string>
typedef boost::shared_ptr<CResourceValue>  CResourceValuePtr;

// -----------------------------------------------------------------------------

#endif
