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

#include <RVWalltime.hpp>
#include <CategoryUUID.hpp>
#include <ABSModule.hpp>
#include <ResourceList.hpp>
#include <sstream>
#include <iomanip>
#include <boost/algorithm/string.hpp>

// -----------------------------------------------------------------------------

CComObject* RVWalltimeCB(void* p_data);

CExtUUID        RVWalltimeID(
                    "{WALLTIME:0bf8863e-67fa-4027-9c0e-41e42be50ecc}",
                    "walltime");

CPluginObject   RVWalltimeObject(&ABSPlugin,
                    RVWalltimeID,RESOURCES_CAT,
                    RVWalltimeCB);

// -----------------------------------------------------------------------------

CComObject* RVWalltimeCB(void* p_data)
{
    CComObject* p_object = new CRVWalltime();
    return(p_object);
}

//------------------------------------------------------------------------------

using namespace std;
using namespace boost;

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

CRVWalltime::CRVWalltime(void)
    : CResourceValue(&RVWalltimeObject)
{
}

//------------------------------------------------------------------------------

void CRVWalltime::TestValue(CResourceList* p_rl,std::ostream& sout,bool& rstatus)
{
    string svalue(Value);

    string legall_characters = "01234567890:dhmsw";

    if( svalue.find_first_not_of(legall_characters) != string::npos ){
        if( rstatus == true ) sout << endl;
        sout << "<b><red> ERROR: Illegal 'walltime' resource specification!" << endl;
        sout <<         "        Supported types: hh:mm:ss, NUMBERw, NUMBERd, NUMBERh, HUMBERm, or NUMBERs</red></b>" << endl;
        rstatus = false;
        return;
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
            sout << "<b><red> ERROR: Illegal suffix for value of 'walltime' resource (short notation)!" << endl;
            sout <<         "        Allowed suffixes are: s (seconds), m (minutes), h (hours), d (days), or w (weeks)</red></b>" << endl;
            rstatus = false;
            return;
        }

        if( mvalue <= 0 ){
            if( rstatus == true ) sout << endl;
            sout << "<b><red> ERROR: Illegal value of 'walltime' resource (short notation)!" << endl;
            sout <<         "        Value must be greater than zero!</red></b>" << endl;
            rstatus = false;
            return;
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
            sout << "<b><red> ERROR: Illegal syntax of 'walltime' resource (long notation)!" << endl;
            sout <<         "        Expected: hh:mm:ss</red></b>" << endl;
            rstatus = false;
            return;
        }

        if( (ssvalue < 0) || (ssvalue >= 60) ){
            if( rstatus == true ) sout << endl;
            sout << "<b><red> ERROR: Illegal number of seconds specified in 'walltime' resource (long notation)!" << endl;
            sout <<         "        Expected: 0-59</red></b>" << endl;
            rstatus = false;
            return;
        }

        if( (mvalue < 0) || (mvalue >= 60) ){
            if( rstatus == true ) sout << endl;
            sout << "<b><red> ERROR: Illegal number of minutes specified in 'walltime' resource (long notation)!" << endl;
            sout <<         "        Expected: 0-59</red></b>" << endl;
            rstatus = false;
            return;
        }

        if( hvalue < 0 ){
            if( rstatus == true ) sout << endl;
            sout << "<b><red> ERROR: Illegal number of hours specified in 'walltime' resource (long notation)!" << endl;
            sout <<         "        Value must be equal or greater than zero!</red></b>" << endl;
            rstatus = false;
            return;
        }

    }

}

//------------------------------------------------------------------------------

void CRVWalltime::TransformWallTimeShortNotation(void)
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

//------------------------------------------------------------------------------

void CRVWalltime::GetAttribute(CSmallString& name, CSmallString& resource, CSmallString& value)
{
    name = "Resource_List";
    resource = Name;
    value = Value;
}

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

