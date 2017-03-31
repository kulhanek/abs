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

#include <ResourceValue.hpp>
#include <PluginDatabase.hpp>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <vector>
#include <boost/algorithm/string.hpp>

//------------------------------------------------------------------------------

using namespace std;
using namespace boost;

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

CResourceValue::CResourceValue(CPluginObject* p_objectinfo)
    : CComObject(p_objectinfo)
{
    if( p_objectinfo ){
        Name = p_objectinfo->GetObjectUUID().GetName();
    }
}

//------------------------------------------------------------------------------

void CResourceValue::TestValue(CResourceList* p_rl,std::ostream& sout,bool& rstatus)
{
    // nothing to be here
}

//------------------------------------------------------------------------------

void CResourceValue::ResolveConflicts(CResourceList* p_rl)
{
    // nothing to be here
}

//------------------------------------------------------------------------------

void CResourceValue::ResolveDynamicResource(CResourceList* p_rl)
{
    // nothing to be here
}

//------------------------------------------------------------------------------

void CResourceValue::GetVariable(CSmallString& name, CSmallString& value)
{
    // nothing to be here
}

//------------------------------------------------------------------------------

void CResourceValue::GetAttribute(CSmallString& name, CSmallString& resource, CSmallString& value)
{
    // nothing to be here
}

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

bool CResourceValue::TestSizeValue(std::ostream& sout,bool& rstatus)
{
    string svalue(Value);

    string legall_characters = "01234567890bkmgt";

    if( svalue.find_first_not_of(legall_characters) != string::npos ){
        if( rstatus == true ) sout << endl;
        sout << "<b><red> ERROR: Illegal '" << Name << "' resource specification!" << endl;
        sout <<         "        Only integer number and one of kb,mb,gb,tb sufixes are allowed!</red></b>" << endl;
        rstatus = false;
        return(false);
    }

    long int        mvalue;
    string          munit;
    stringstream    str(svalue);

    str >> mvalue >> munit;

    to_lower(munit);
    if( (munit != "kb") &&
        (munit != "mb") &&
        (munit != "gb") &&
        (munit != "tb") ) {
        if( rstatus == true ) sout << endl;
        sout << "<b><red> ERROR: Illegal '" << Name << "' resource specification!" << endl;
        sout <<         "        Allowed suffixes are kb,mb,gb,tb but '" << munit << "' is specified!</red></b>" << endl;
        rstatus = false;
        return(false);
    }

    return(true);
}

//------------------------------------------------------------------------------

long long CResourceValue::GetSize(void)
{
    string          svalue(Value);
    long long       size;
    string          munit;
    stringstream    str(svalue);

    str >> size >> munit;

    to_lower(munit);

    if( munit == "kb" ) size = size;
    if( munit == "mb" ) size = size * 1024;
    if( munit == "gb" ) size = size * 1024 * 1024;
    if( munit == "tb" ) size = size * 1024 * 1024 * 1024;

    return( size );
}

//------------------------------------------------------------------------------

CSmallString CResourceValue::GetSizeString(void)
{
    long long size = GetSize();

    string          munit;
    long long       msize;

    munit = "kb";

    if( size >= 1024 ){
        msize = size % 1024;
        if( msize == 0 ){
            munit = "mb";
            size /= 1024;
        }
    }
    if( size >= 1024 ){
        msize = size % 1024;
        if( msize == 0 ){
            munit = "gb";
            size /= 1024;
        }
    }
    if( size >= 1024 ){
        msize = size % 1024;
        if( msize == 0 ){
            munit = "tb";
            size /= 1024;
        }
    }

    stringstream    str;
    str << size << munit;

    return(str.str().c_str());
}

//------------------------------------------------------------------------------

void CResourceValue::SetSize(long long size)
{
    string          munit;
    long long       msize;

    munit = "kb";

    if( size >= 1024 ){
        msize = size % 1024;
        if( msize == 0 ){
            munit = "mb";
            size /= 1024;
        }
    }
    if( size >= 1024 ){
        msize = size % 1024;
        if( msize == 0 ){
            munit = "gb";
            size /= 1024;
        }
    }
    if( size >= 1024 ){
        msize = size % 1024;
        if( msize == 0 ){
            munit = "tb";
            size /= 1024;
        }
    }

    stringstream    str;
    str << size << munit;

    Value = str.str().c_str();
}

//------------------------------------------------------------------------------

bool CResourceValue::TestNumberValue(std::ostream& sout,bool& rstatus)
{
    string svalue(Value);

    string legall_characters = "01234567890";

    if( svalue.find_first_not_of(legall_characters) != string::npos ){
        if( rstatus == true ) sout << endl;
        sout << "<b><red> ERROR: Illegal '" << Name << "' resource specification!" << endl;
        sout <<         "        Integer number expected but '" << Value << "' is specified!</red></b>" << endl;
        rstatus = false;
        return(false);
    }

    return(true);
}

//------------------------------------------------------------------------------

long long CResourceValue::GetNumber(void)
{
    string          svalue(Value);
    long long       size;
    stringstream    str(svalue);

    str >> size;
    return(size);
}

//------------------------------------------------------------------------------

const CSmallString& CResourceValue::GetValue(void)
{
    return(Value);
}

//------------------------------------------------------------------------------

bool CResourceValue::TestKeyValue(std::ostream& sout,bool& rstatus,const CSmallString& keys)
{
    string          skeys(keys);
    vector<string>  items;

    split(items,skeys,is_any_of(","),boost::token_compress_on);

    vector<string>::iterator it = items.begin();
    vector<string>::iterator ie = items.end();

    while( it != ie ){
        if( CSmallString(*it) == Value ) return(true);
        it++;
    }

    // not found
    if( rstatus == true ) sout << endl;
    sout << "<b><red> ERROR: Illegal '" << Name << "' resource specification!" << endl;
    sout <<         "        Allowed values '" << keys << "' but '" << Value << "' is specified!</red></b>" << endl;
    rstatus = false;
    return(false);
}

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

