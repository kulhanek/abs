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

#include <RVProps.hpp>
#include <CategoryUUID.hpp>
#include <PBSProModule.hpp>
#include <vector>
#include <boost/algorithm/string/split.hpp>
#include <boost/algorithm/string/classification.hpp>

// -----------------------------------------------------------------------------

CComObject* RVPropsCB(void* p_data);

CExtUUID        RVPropsID(
                    "{PROPS:72b2fbcd-7d51-4ab0-b045-49887346f7f9}",
                    "props");

CPluginObject   RVPropsObject(&PBSProPlugin,
                    RVPropsID,RESOURCES_CAT,
                    RVPropsCB);

// -----------------------------------------------------------------------------

CComObject* RVPropsCB(void* p_data)
{
    CComObject* p_object = new CRVProps();
    return(p_object);
}

//------------------------------------------------------------------------------

using namespace std;
using namespace boost;

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

CRVProps::CRVProps(void)
    : CResourceValue(&RVPropsObject)
{
}

//------------------------------------------------------------------------------

void CRVProps::TestValue(CResourceList* p_rl,std::ostream& sout,bool& rstatus)
{
    vector<string> slist;
    string         svalue(Value);
    split(slist,svalue,is_any_of("#:"),boost::token_compress_on);

    vector<string>::iterator it = slist.begin();
    vector<string>::iterator ie = slist.end();

    int pos = 0;
    while( it != ie ){
        string item = *it;
        it++;
        pos++;
        if( item.size() == 0 ){
            if( rstatus == true ) sout << endl;
            sout << "<b><red> ERROR: Illegal '" << Name << "' resource specification!" << endl;
            sout <<         "        Empty property specification! Position: " << pos << "</red></b>" << endl;
            rstatus = false;
            continue;
        }

        string name = item;
        if ( item.find('=') != string::npos ){
            // item already contains property name and its value
            // test only for negation, which is not permitted
            if( item[0] == '^' ){
                if( rstatus == true ) sout << endl;
                sout << "<b><red> ERROR: Illegal '" << Name << "' resource specification!" << endl;
                sout <<         "        It is not permited to negate property that has explicit value! Position: " << pos << "</red></b>" << endl;
                rstatus = false;
                continue;
            }
        } else {
            // old fashion property specification
            if( item[0] == '^' ){
                name = string(item.begin()+1,item.end());
                if( name.empty() ){
                    if( rstatus == true ) sout << endl;
                    sout << "<b><red> ERROR: Illegal '" << Name << "' resource specification!" << endl;
                    sout <<         "        Property without name! Position: " << pos << "</red></b>" << endl;
                    rstatus = false;
                    continue;
                }
            }
        }
    }
}

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

