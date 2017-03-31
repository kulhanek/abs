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
        p_objectinfo->GetObjectUUID().GetName();
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

bool CResourceValue::TestSizeValue(CResourceList* p_rl,std::ostream& sout,bool& rstatus)
{
    string svalue(Value);

    string legall_characters = "01234567890bkmgt";

    if( svalue.find_first_not_of(legall_characters) != string::npos ){
        if( rstatus == true ) sout << endl;
        sout << "<b><red> ERROR: Illegal '" << Name << "' resource specification!" << endl;
        sout <<         "        Only integer number and one of b,kb,mb,gb,tb sufixes are allowed!</red></b>" << endl;
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

    if( munit == "b" )  size = size * 1;
    if( munit == "kb" ) size = size * 1024;
    if( munit == "mb" ) size = size * 1024 * 1024;
    if( munit == "gb" ) size = size * 1024 * 1024 * 1024;
    if( munit == "tb" ) size = size * 1024 * 1024 * 1024 * 1024;

    return( size );
}

//------------------------------------------------------------------------------

bool CResourceValue::TestNumberValue(CResourceList* p_rl,std::ostream& sout,bool& rstatus)
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

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

