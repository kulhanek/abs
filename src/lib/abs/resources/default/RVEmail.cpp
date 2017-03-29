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

#include <ResourceValue.hpp>
#include <iostream>
#include <boost/algorithm/string.hpp>
#include <sstream>
#include <iomanip>
#include <User.hpp>
#include <sys/stat.h>
#include <FileSystem.hpp>
#include <grp.h>

//------------------------------------------------------------------------------

using namespace std;
using namespace boost;

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

CResourceValue::CResourceValue(void)
{
    Name = "email";
}

//------------------------------------------------------------------------------

bool CResourceValue::IsHelperToken(void)
{
    if( (Name == "account")  ||
        (Name == "props") ||
        (Name == "ncpus") ||
        (Name == "ngpus") ||
        (Name == "maxcpuspernode") ||
        (Name == "umask") ||
        (Name == "group") ||
        (Name == "numa") ||
        (Name == "email") ||
        (Name == "cpu_freq") ||
        (Name == "host") ||
        (Name == "surrogate") ||
        ( HelperValue == true )
        ) return(true);

    return(false);
}

//------------------------------------------------------------------------------

void CResourceValue::Finalize(std::map<std::string,std::string>& variables)
{
//    FIXME
//    if( Name == "scratch_type" ){
//        bool pass = false;
//        ABSConfig.TestScratchType(Value,pass);
//        HelperValue = ! pass;
//        return;
//    }
    // resource variables
    if( Name == "numa" && Value == "on" ){
        variables["INF_NUMA"] = "ON";
    }
    if( Name == "email" ){
        variables["INF_EMAIL"] = Value;
    }
}

//------------------------------------------------------------------------------

void CResourceValue::TestValue(std::ostream& sout,bool& rstatus)
{
    bool found = false;

    if( Name == "ncpus" ){
        TestNCPUs(sout,rstatus);
        found = true;
    }
    if( Name == "ngpus" ){
        TestNGPUs(sout,rstatus);
        found = true;
    }
    if( Name == "maxcpuspernode" ){
        TestMaxCPUsPerNode(sout,rstatus);
        found = true;
    }
    if( Name == "props" ){
        TestProps(sout,rstatus);
        found = true;
    }
    if( Name == "mem" ){
        TestMem(sout,rstatus);
        found = true;
    }

    if( Name == "walltime" ){
        TestWallTime(sout,rstatus);
        found = true;
    }
    if( Name == "scratch" ){
        TestScratch(sout,rstatus);
        found = true;
    }
    if( Name == "account" ){
        rstatus = true;
        found = true;
    }
    if( Name == "place" ){
        TestPlace(sout,rstatus);
        found = true;
    }
    if( Name == "scratch_type" ){
        TestScratchType(sout,rstatus);
        found = true;
    }
    if( Name == "umask" ){
        TestUMask(sout,rstatus);
        found = true;
    }
    if( Name == "group" ){
        TestGroup(sout,rstatus);
        found = true;
    }
    if( Name == "cpu_freq" ){
        TestCPUFreq(sout,rstatus);
        found = true;
    }
    if( Name == "numa" ){
        TestNUMA(sout,rstatus);
        found = true;
    }
    if( Name == "email" ){
        TestEmail(sout,rstatus);
        found = true;
    }
    if( Name == "host" ){
        TestHost(sout,rstatus);
        found = true;
    }
    if( Name == "surrogate" ){
        TestSurrogate(sout,rstatus);
        found = true;
    }

// FIXME
//    if( found == true ){
//        if( ABSConfig.TestResourceTokens(Name) == false ){
//            if( rstatus == true ) sout << endl;
//            sout << "<b><red> ERROR: Resource token '" << Name << "' is recognized but not allowed on this site!</red></b>" << endl;
//            sout << "<b><red>        Allowed tokens: ";
//            ABSConfig.PrintAllowedResTokens(sout);
//            sout << "!</red></b>" << endl;
//            rstatus = false;
//        }
//    } else {
//        if( rstatus == true ) sout << endl;
//        sout << "<b><red> ERROR: Unsupported resource '" << Name << "'!</red></b>" << endl;
//        rstatus = false;
//    }
// FIXME
    rstatus = found;
}

//------------------------------------------------------------------------------

bool CResourceValue::TestValue(std::ostream& sout)
{
    bool rstatus = true;
    TestValue(sout,rstatus);
    return( rstatus );
}

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

bool CResourceValue::TestUMask(std::ostream& sout,bool& rstatus)
{
    if( Value == "current" ){
        Value = User.GetUMask();
    }  else if ( Value == "jobdir" ) {
        Value = User.GetUMask(); // fallback value
        struct stat info;
        CSmallString pwd;
        if( CFileSystem::GetCurrentDir(pwd) == true ){
            if( stat(pwd, &info) == 0 ){
                mode_t mode = info.st_mode;
                int u = (mode & 0700) >> 6;
                int g = (mode & 0070) >> 3;
                int o = mode & 0007;
                // invert mode
                u = (~u) & 0x7;
                g = (~g) & 0x7;
                o = (~o) & 0x7;
                stringstream str;
                str << u << g << o;
                Value = str.str().c_str();
            }
        }
    }

    if( Value.GetLength() != 3 ){
        if( rstatus == true ) sout << endl;
        sout << "<b><red> ERROR: User file creation mask (umask) must be composed from three numbers!</red></b>" << endl;
        rstatus = false;
        return(false);
    }
    for(int i=0; i < 3; i++){
        if( (Value[i] < '0') || (Value[i] > '7') ){
            if( rstatus == true ) sout << endl;
            sout << "<b><red> ERROR: Illegal number '" << Value[i] << "' at possition " << i+ 1 << " in the user file creation mask (umask)!" << endl;
            sout <<         "        Allowed values: 0-7</red></b>" << endl;
            rstatus = false;
            return(false);
        }
    }

    return(true);
}

//------------------------------------------------------------------------------

bool CResourceValue::TestGroup(std::ostream& sout,bool& rstatus)
{
    if( Value == "current" ){
        Value = User.GetEGroup();
    } else if( Value == "jobdir" ){
        Value = User.GetEGroup(); // fallback value
        struct stat info;
        CSmallString pwd;
        if( CFileSystem::GetCurrentDir(pwd) == true ){
            if( stat(pwd, &info) == 0 ){
                struct group* p_gr = getgrgid(info.st_gid);
                if( p_gr ){
                    Value = p_gr->gr_name;
                }
            }
        }
    }

    // do not check the group here as surrogate option need to obe processed first
    return(true);
}

//------------------------------------------------------------------------------

bool CResourceValue::TestEmail(std::ostream& sout,bool& rstatus)
{
    HelperValue = true;
    return(true);
}

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

bool CResourceValue::TestWallTime(std::ostream& sout,bool& rstatus)
{
    string svalue(Value);

    string legall_characters = "01234567890:dhms";

    if( svalue.find_first_not_of(legall_characters) != string::npos ){
        if( rstatus == true ) sout << endl;
        sout << "<b><red> ERROR: Illegal walltime token specification!" << endl;
        sout <<         "        Supported types: hh:mm:ss, NUMBERd, NUMBERh, HUMBERm, or NUMBERs</red></b>" << endl;
        rstatus = false;
        return(false);
    }

    if( svalue.find_first_of("wdhms") != string::npos ){
        // short notation
        long int        mvalue = 0;
        string          munit;
        stringstream    str(svalue);
        str >> mvalue >> munit;

        to_lower(munit);
        if( (munit != "s")  &&
            (munit != "m") &&
            (munit != "h") &&
            (munit != "d") &&
            (munit != "w") ) {
            if( rstatus == true ) sout << endl;
            sout << "<b><red> ERROR: Illegal suffix for value of walltime token (short notation)!" << endl;
            sout <<         "        Allowed suffixes are: s (seconds), m (minutes), h (hours), d (days), or w (weeks)</red></b>" << endl;
            rstatus = false;
            return(false);
        }

        if( mvalue <= 0 ){
            if( rstatus == true ) sout << endl;
            sout << "<b><red> ERROR: Illegal value of walltime token (short notation)!" << endl;
            sout <<         "        Value must be greater than zero!</red></b>" << endl;
            rstatus = false;
            return(false);
        }

        TransformWallTimeShortNotation();

    } else {
        int             hvalue = 0;
        int             mvalue = 0;
        int             ssvalue = 0;
        stringstream    str(svalue);
        char            d1,d2;
        str >> hvalue >> d1 >> mvalue >> d2 >> ssvalue;

        if( (d1 != ':') || (d2 != ':') ){
            if( rstatus == true ) sout << endl;
            sout << "<b><red> ERROR: Illegal syntax of walltime token (long notation)!" << endl;
            sout <<         "        Expected: hh:mm:ss</red></b>" << endl;
            rstatus = false;
            return(false);
        }

        if( (ssvalue < 0) || (ssvalue >= 60) ){
            if( rstatus == true ) sout << endl;
            sout << "<b><red> ERROR: Illegal number of seconds specified in walltime token (long notation)!" << endl;
            sout <<         "        Expected: 0-59</red></b>" << endl;
            rstatus = false;
            return(false);
        }

        if( (mvalue < 0) || (mvalue >= 60) ){
            if( rstatus == true ) sout << endl;
            sout << "<b><red> ERROR: Illegal number of minutes specified in walltime token (long notation)!" << endl;
            sout <<         "        Expected: 0-59</red></b>" << endl;
            rstatus = false;
            return(false);
        }

        if( hvalue < 0 ){
            if( rstatus == true ) sout << endl;
            sout << "<b><red> ERROR: Illegal number of hours specified in walltime token (long notation)!" << endl;
            sout <<         "        Value must be equal or greater than zero!</red></b>" << endl;
            rstatus = false;
            return(false);
        }

    }

    return(true);
}

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

bool CResourceValue::TestNCPUs(std::ostream& sout,bool& rstatus)
{
    string svalue(Value);

    string legall_characters = "01234567890";

    if( svalue.find_first_not_of(legall_characters) != string::npos ){
        if( rstatus == true ) sout << endl;
        sout << "<b><red> ERROR: Number of CPUs (ncpus) is not an integer number!</red></b>" << endl;
        rstatus = false;
        return(false);
    }

    if( Value.ToInt() <= 0 ){
        if( rstatus == true ) sout << endl;
        sout << "<b><red> ERROR: Number of CPUs (ncpus) must be greater than zero!</red></b>" << endl;
        rstatus = false;
        return(false);
    }
    return(true);
}

//------------------------------------------------------------------------------

bool CResourceValue::TestNGPUs(std::ostream& sout,bool& rstatus)
{
    string svalue(Value);

    string legall_characters = "01234567890";

    if( svalue.find_first_not_of(legall_characters) != string::npos ){
        if( rstatus == true ) sout << endl;
        sout << "<b><red> ERROR: Number of GPUs (ngpus) is not an integer number!</red></b>" << endl;
        rstatus = false;
        return(false);
    }

    if( Value.ToInt() <= 0 ){
        if( rstatus == true ) sout << endl;
        sout << "<b><red> ERROR: Number of GPUs (ngpus) must be greater than or equal to zero!</red></b>" << endl;
        rstatus = false;
        return(false);
    }
    return(true);
}

//------------------------------------------------------------------------------

bool CResourceValue::TestMaxCPUsPerNode(std::ostream& sout,bool& rstatus)
{
    string svalue(Value);

    string legall_characters = "01234567890";

    if( svalue.find_first_not_of(legall_characters) != string::npos ){
        if( rstatus == true ) sout << endl;
        sout << "<b><red> ERROR: Max number of CPUs per node (maxcpuspernode) is not an integer number!</red></b>" << endl;
        rstatus = false;
        return(false);
    }

    if( Value.ToInt() <= 0 ){
        if( rstatus == true ) sout << endl;
        sout << "<b><red> ERROR: Max number of CPUs per node (maxcpuspernode) must be greater than zero!</red></b>" << endl;
        rstatus = false;
        return(false);
    }
    return(true);
}

//------------------------------------------------------------------------------

bool CResourceValue::TestProps(std::ostream& sout,bool& rstatus)
{
    // FIXME
    return(true);
}

//------------------------------------------------------------------------------

bool CResourceValue::TestMem(std::ostream& sout,bool& rstatus)
{
    string svalue(Value);

    string legall_characters = "01234567890bkmgt";

    if( svalue.find_first_not_of(legall_characters) != string::npos ){
        if( rstatus == true ) sout << endl;
        sout << "<b><red> ERROR: Illegal memory token (mem) specification!" << endl;
        sout <<         "        Only integer number and one of b,kb,mb,gb,tb sufixes is allowed!</red></b>" << endl;
        rstatus = false;
        return(false);
    }

    long int        mvalue;
    string          munit;
    stringstream    str(svalue);

    str >> mvalue >> munit;

    to_lower(munit);
    if( (munit != "b")  &&
        (munit != "kb") &&
        (munit != "mb") &&
        (munit != "gb") &&
        (munit != "tb") ) {
        if( rstatus == true ) sout << endl;
        sout << "<b><red> ERROR: Illegal suffix for memory token (mem)!" << endl;
        sout <<         "        Allowed suffixes are: b,kb,mb,gb,tb.</red></b>" << endl;
        rstatus = false;
        return(false);
    }

    return(true);
}

//------------------------------------------------------------------------------

bool CResourceValue::TestNUMA(std::ostream& sout,bool& rstatus)
{
    bool ok = false;
    if( Value == "none" ) ok = true;
    if( Value == "cpuset" ) ok = true;
    if( Value == "auto" ) ok = true;

    if( ok == false ){
        if( rstatus == true ) sout << endl;
        sout << "<b><red> ERROR: Illegal NUMA resource (numa) specification!" << endl;
        sout <<         "        Allowed values: default, cpuset, auto!</red></b>" << endl;
        rstatus = false;
        return(false);
    }
    HelperValue = true;
    return(true);
}

//------------------------------------------------------------------------------

bool CResourceValue::TestPlace(std::ostream& sout,bool& rstatus)
{
     // too complex setup in IT4I
//    bool ok = false;
//    if( Value == "free" ) ok = true;
//    if( Value == "pack" ) ok = true;
//    if( Value == "scatter" ) ok = true;
//    if( Value == "vscatter" ) ok = true;
//    if( Value == "excl" ) ok = true;
//    if( Value == "exclhost" ) ok = true;

//    if( ok == false ){
//        if( rstatus == true ) sout << endl;
//        sout << "<b><red> ERROR: Illegal place resource (place) specification!" << endl;
//        sout <<         "        Allowed values: free, pack, scatter, vscatter, excl, exclhost!</red></b>" << endl;
//        rstatus = false;
//        return(false);
//    }

    return(true);
}

//------------------------------------------------------------------------------

bool CResourceValue::TestNode(std::ostream& sout,bool& rstatus)
{
    // FIXME - check against list of computational nodes
    HelperValue = true;
    return(true);
}

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

bool CResourceValue::TestWorkSize(std::ostream& sout,bool& rstatus)
{
    string svalue(Value);

    string legall_characters = "01234567890bkmgt";

    if( svalue.find_first_not_of(legall_characters) != string::npos ){
        if( rstatus == true ) sout << endl;
        sout << "<b><red> ERROR: Illegal scratch size token (worksize) specification!" << endl;
        sout <<         "        Only integer number and one of b,kb,mb,gb,tb sufixes is allowed!</red></b>" << endl;
        rstatus = false;
        return(false);
    }

    long int        mvalue;
    string          munit;
    stringstream    str(svalue);

    str >> mvalue >> munit;

    to_lower(munit);
    if( (munit != "b")  &&
        (munit != "kb") &&
        (munit != "mb") &&
        (munit != "gb") &&
        (munit != "tb") ) {
        if( rstatus == true ) sout << endl;
        sout << "<b><red> ERROR: Illegal suffix for scratch size token (worksize)!" << endl;
        sout <<         "        Allowed suffixes are: b,kb,mb,gb,tb.</red></b>" << endl;
        rstatus = false;
        return(false);
    }

    return(true);
}

//------------------------------------------------------------------------------

bool CResourceValue::TestWorkDir(std::ostream& sout,bool& rstatus)
{
    bool pass = false;

// FIXME
//    if( ABSConfig.TestScratchType(Value,pass) == false ){
//        if( rstatus == true ) sout << endl;
//        sout << "<b><red> ERROR: Illegal scratch type (scratch_type) specification!" << endl;
//        sout <<         "        Allowed values: ";
//        ABSConfig.PrintAllowedScratchTypes(sout);
//        sout << "!</red></b>" << endl;
//        rstatus = false;
//        return(false);
//    }

    HelperValue = true;

    return(true);
}

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

void CResourceValue::TransformWallTimeShortNotation(void)
{
    string svalue(Value);

    // short notation
    long int        mvalue = 0;
    string          munit;
    stringstream    str(svalue);
    str >> mvalue >> munit;

    to_lower(munit);
    if( munit == "s" ) {
        mvalue = mvalue * 1;
    }
    if( munit == "m" ) {
        mvalue = mvalue * 60;
    }
    if( munit == "h" ) {
        mvalue = mvalue * 3600;
    }
    if( munit == "d" ) {
        mvalue = mvalue * 86400;
    }
    if( munit == "w" ) {
        mvalue = mvalue * 86400 * 7;
    }

    int s,m,h;
    s = mvalue % 60;
    mvalue = mvalue / 60;
    m = mvalue % 60;
    mvalue = mvalue / 60;
    h = mvalue;

    stringstream    ostr;
    ostr << h << ":" << setw(2) << setfill('0') << m << ":" << setw(2) << setfill('0') << s;
    Value = CSmallString(ostr.str());
}

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

