// =============================================================================
// ABS - Advanced Batch System
// -----------------------------------------------------------------------------
//    Copyright (C) 2017 Petr Kulhanek, kulhanek@chemi.muni.cz
//    Copyright (C) 2011-2013 Petr Kulhanek, kulhanek@chemi.muni.cz
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

#include <ResourceList.hpp>
#include <vector>
#include <boost/algorithm/string/split.hpp>
#include <boost/algorithm/string.hpp>
#include <boost/algorithm/string/classification.hpp>
#include <boost/lexical_cast.hpp>
#include <SimpleIterator.hpp>
#include <PluginDatabase.hpp>
#include <CategoryUUID.hpp>

//------------------------------------------------------------------------------

using namespace std;
using namespace boost;
using namespace boost::algorithm;

//------------------------------------------------------------------------------

CResourceList ResourceList;

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

CResourceList::CResourceList(void)
{
}

//------------------------------------------------------------------------------

CResourceList::~CResourceList(void)
{
}

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

void CResourceList::AddResources(const CSmallString& resources,std::ostream& sout,bool& rstatus)
{
    string          svalue = string(resources);
    vector<string>  items;

    // split to items
    split(items,svalue,is_any_of(","));

    vector<string>::iterator it = items.begin();
    vector<string>::iterator ie = items.end();

    while( it != ie ){
        string sres = *it;
        it++;
        // resource is empty string
        if( sres.size() == 0 ) continue;

        char   command = '+';
        string name;
        string value;

        if( sres[0] == '-' ){
            command = '-';
            sres = string(sres.begin()+1,sres.end()); // strip down a letter
        }

        // resource is empty string
        if( sres.size() == 0 ) continue;

        // is it an integer number?
        try {
            lexical_cast<int>(sres);
            name = "ncpus";
            value = sres;
        } catch(bad_lexical_cast &) {
            if( sres.find('=') == string::npos ){
                name = sres;
                value = "";
            } else {
                // it contains '=', split on the first occurence
                string::iterator pos = find(sres.begin(), sres.end(), '=');
                name = string(sres.begin(), pos);
                value = string(pos + 1, sres.end());
            }
        }

        // process resource
        switch(command){
            case '+':
                AddResource(name,value,sout,rstatus);
                break;
            case '-':
                RemoveResource(name);
                break;
        }
    }
}

//------------------------------------------------------------------------------

bool CResourceList::AddResources(const CSmallString& resources)
{
    stringstream sout;
    bool         result = true;
    AddResources(resources,sout,result);
    return(result);
}

//------------------------------------------------------------------------------

void CResourceList::AddResource(const CSmallString& name,const CSmallString& value,std::ostream& sout,bool& rstatus)
{
    // be sure that the resource is unique
    RemoveResource(name);

    // try to find resource plugin object
    CSimpleIteratorC<CPluginObject> I(PluginDatabase.GetObjectList());
    CPluginObject* p_pobj;
    while( (p_pobj = I.Current()) ){
        if( p_pobj->GetCategoryUUID() == RESOURCES_CAT ){
            CSmallString res_name = p_pobj->GetObjectUUID().GetDescription();
            if( name == res_name ){
                CComObject* p_obj = p_pobj->CreateObject(NULL);
                CResourceValue* p_res = dynamic_cast<CResourceValue*>(p_obj);
                if( p_res != NULL ){
                    CResourceValuePtr res_ptr(p_res);
                    res_ptr->Value = value;
                    push_back(res_ptr);
                    return;
                } else {
                    delete p_obj;
                }
            }
        }
        I++;
    }

    // plugin object was not found
    if( rstatus == true ) sout << endl;
    sout << "<red>ERROR: Resource '" << name << "' is not supported!</red>" << endl;
    rstatus = false;
}

//------------------------------------------------------------------------------

void CResourceList::AddResource(const CResourceValuePtr& res)
{
    if( res == NULL ) return;
    push_back(res);
}

//------------------------------------------------------------------------------

void CResourceList::RemoveResource(const CSmallString& name)
{
    std::list<CResourceValuePtr>::iterator     it = begin();
    std::list<CResourceValuePtr>::iterator     ie = end();

    while( it != ie ){
        CResourceValuePtr p_rv = *it;
        if( p_rv->Name == name ){
            erase(it);
            return;
        }
        it++;
    }
}

//------------------------------------------------------------------------------

CResourceValuePtr CResourceList::FindResource(const CSmallString& name) const
{
    std::list<CResourceValuePtr>::const_iterator     it = begin();
    std::list<CResourceValuePtr>::const_iterator     ie = end();

    while( it != ie ){
        CResourceValuePtr p_rv = *it;
        if( p_rv->Name == name ){
            return( p_rv );
        }
        it++;
    }

    return(CResourceValuePtr());
}

//------------------------------------------------------------------------------

const CSmallString CResourceList::GetResourceValue(const CSmallString& name) const
{
   CResourceValuePtr rs_ptr = FindResource(name);
   if( rs_ptr == NULL ) return("");
   return(rs_ptr->Value);
}

//------------------------------------------------------------------------------

void CResourceList::RemoveAllResources(void)
{
    clear();
}

//------------------------------------------------------------------------------

void CResourceList::ResolveConflicts(void)
{
    std::list<CResourceValuePtr>::iterator     it = begin();
    std::list<CResourceValuePtr>::iterator     ie = end();

    while( it != ie ){
        CResourceValuePtr p_rv = *it;
        p_rv->ResolveConflicts();
        it++;
    }
}

//------------------------------------------------------------------------------

void CResourceList::TestResourceValues(std::ostream& sout,bool& rstatus)
{
    std::list<CResourceValuePtr>::iterator     it = begin();
    std::list<CResourceValuePtr>::iterator     ie = end();

    while( it != ie ){
        CResourceValuePtr p_rv = *it;
        p_rv->TestValue(sout,rstatus);
        it++;
    }
}

//------------------------------------------------------------------------------

void CResourceList::FinalizeResources(void)
{
    std::list<CResourceValuePtr>::iterator     it = begin();
    std::list<CResourceValuePtr>::iterator     ie = end();

    while( it != ie ){
        CResourceValuePtr p_rv = *it;
        p_rv->FinalizeResource();
        it++;
    }
}

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

const CSmallString CResourceList::ToString(bool include_spaces) const
{
    std::list<CResourceValuePtr>::const_iterator     it = begin();
    std::list<CResourceValuePtr>::const_iterator     ie = end();

    CSmallString output;

    while( it != ie ){
        if( it != begin() ){
            output << ",";
            if( include_spaces ) output << " ";
        }
        CResourceValuePtr p_rv = *it;
        output << p_rv->Name << "=" << p_rv->Value;
        it++;
    }

    return(output);
}



//------------------------------------------------------------------------------

int CResourceList::GetNumOfCPUs(void) const
{
    const CResourceValuePtr p_rv = FindResource("ncpus");
    if( p_rv == NULL ) return(1);
    return( p_rv->Value.ToInt() );
}

//------------------------------------------------------------------------------

int CResourceList::GetNumOfGPUs(void) const
{
    const CResourceValuePtr p_rv = FindResource("ngpus");
    if( p_rv == NULL ) return(0);
    return( p_rv->Value.ToInt() );
}

//------------------------------------------------------------------------------

int CResourceList::GetMaxNumOfCPUsPerNode(void) const
{
    const CResourceValuePtr p_rv = FindResource("maxcpuspernode");
    if( p_rv == NULL ) return(1);
    return( p_rv->Value.ToInt() );
}

//------------------------------------------------------------------------------

int CResourceList::GetNumOfNodes(void) const
{
    // FIXME
    const CResourceValuePtr p_rv = FindResource("ncpus");
    return(1);
}

//------------------------------------------------------------------------------

long int CResourceList::GetMemory(void) const
{
    const CResourceValuePtr p_rv = FindResource("mem");
    if( p_rv == NULL ) return(0);

    string          svalue(p_rv->Value);
    long int        mem;
    string          munit;
    stringstream    str(svalue);

    str >> mem >> munit;

    to_lower(munit);

    if( munit == "b" )  mem = mem * 1;
    if( munit == "kb" ) mem = mem * 1024;
    if( munit == "mb" ) mem = mem * 1024 * 1024;
    if( munit == "gb" ) mem = mem * 1024 * 1024 * 1024;
    if( munit == "tb" ) mem = mem * 1024 * 1024 * 1024 * 1024;

    return( mem );
}

//------------------------------------------------------------------------------

bool CResourceList::GetWallTime(CSmallTime& time) const
{
    const CResourceValuePtr p_rv = FindResource("walltime");
    if( p_rv == NULL ) return(false);

    int             hvalue = 0;
    int             mvalue = 0;
    int             ssvalue = 0;
    stringstream    str(p_rv->Value.GetBuffer());
    char            d1,d2;
    str >> hvalue >> d1 >> mvalue >> d2 >> ssvalue;
    CSmallTime ltime(hvalue*3600 + mvalue*60 + ssvalue);
    time = ltime;
    return(true);
}

//------------------------------------------------------------------------------

bool CResourceList::SortCompName(const CResourceValuePtr& p_left,const CResourceValuePtr& p_right)
{
    return( strcmp(p_left->Name,p_right->Name) < 0 );
}

//------------------------------------------------------------------------------

void CResourceList::SortByName(void)
{
    sort(SortCompName);
}

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

