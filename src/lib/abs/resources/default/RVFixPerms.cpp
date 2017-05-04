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

#include <RVFixPerms.hpp>
#include <CategoryUUID.hpp>
#include <ABSModule.hpp>
#include <ResourceList.hpp>
#include <list>
#include <set>
#include <string>
#include <boost/algorithm/string/split.hpp>
#include <boost/algorithm/string/classification.hpp>

// -----------------------------------------------------------------------------

CComObject* RVFixPermsCB(void* p_data);

CExtUUID        RVFixPermsID(
                    "{FIXPERMS:5ddfd83f-5495-4db7-994c-363ef4568fb8}",
                    "fixperms");

CPluginObject   RVFixPermsObject(&ABSPlugin,
                    RVFixPermsID,RESOURCES_CAT,
                    RVFixPermsCB);

// -----------------------------------------------------------------------------

CComObject* RVFixPermsCB(void* p_data)
{
    CComObject* p_object = new CRVFixPerms();
    return(p_object);
}

//------------------------------------------------------------------------------

using namespace std;
using namespace boost;

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

CRVFixPerms::CRVFixPerms(void)
    : CResourceValue(&RVFixPermsObject)
{
}

//------------------------------------------------------------------------------

void CRVFixPerms::TestValue(CResourceList* p_rl,std::ostream& sout,bool& rstatus)
{
    string allowed = "jobdir+jobdata+parent+none";
    set<string>  keys;
    split(keys,allowed,is_any_of("+"));

    list<string> items;
    string svalue(Value);
    split(items,svalue,is_any_of("+"),token_compress_on);

    list<string>::iterator it = items.begin();
    list<string>::iterator ie = items.end();

    while( it != ie ){
        if( keys.count(*it) == 0 ){
            if( rstatus == true ) sout << endl;
            sout << "<b><red> ERROR: Illegal '" << Name << "' resource specification!" << endl;
            sout << "<b><red>        The mode '" << *it << "' is not one of " << allowed << "!</red></b>" << endl;
            rstatus = false;
            return;
        }
        it++;
    }
}

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

