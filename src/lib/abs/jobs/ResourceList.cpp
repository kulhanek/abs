// =============================================================================
// ABS - Advanced Batch System
// -----------------------------------------------------------------------------
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
#include <pbs_ifl.h>
#include <TorqueAttr.hpp>

//------------------------------------------------------------------------------

using namespace std;
using namespace boost;
using namespace boost::algorithm;

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

void CResourceList::Parse(const CSmallString& resources)
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

        // does it contain '='?
        if( sres.find('=') == string::npos ){
            // is it an integer number?
            CResourceValuePtr p_ritem = CResourceValuePtr(new CResourceValue);
            try {
                lexical_cast<int>(sres);
                p_ritem->Name = "ncpus";
                p_ritem->Value = sres;
            } catch(bad_lexical_cast &) {
                p_ritem->Name = sres;
                p_ritem->Value = NULL;
            }
            push_back(p_ritem);
            continue;
        }

        // it contains '=', split on the first occurence
        string::iterator pos = find(sres.begin(), sres.end(), '=');
        string key(sres.begin(), pos);
        string value(pos + 1, sres.end());

        CResourceValuePtr p_ritem = CResourceValuePtr(new CResourceValue);
        p_ritem->Name = key;
        p_ritem->Value = value;
        push_back(p_ritem);
    }
}

//------------------------------------------------------------------------------

// put from the source only missing resources

void CResourceList::Merge(const CResourceList& source)
{
    list<CResourceValuePtr>::const_iterator it = source.begin();
    list<CResourceValuePtr>::const_iterator ie = source.end();

    while( it != ie ){
        CResourceValuePtr p_srv = *it;
        CResourceValuePtr p_drv = GetResource(p_srv->Name);
        if( p_drv == NULL ){
            CResourceValuePtr p_nrv = CResourceValuePtr(new CResourceValue);
            p_nrv->Name = p_srv->Name;
            p_nrv->Value = p_srv->Value;
            push_back(p_nrv);
        }
        it++;
    }
}

//------------------------------------------------------------------------------

bool CResourceList::TestResources(std::ostream& sout)
{
    bool rstatus = true;
    TestResources(sout,rstatus);
    return(rstatus);
}

//------------------------------------------------------------------------------

void CResourceList::TestResources(std::ostream& sout,bool& rstatus)
{
    std::list<CResourceValuePtr>::iterator     it = begin();
    std::list<CResourceValuePtr>::iterator     ie = end();

    while( it != ie ){
        CResourceValuePtr p_rv = *it;
        p_rv->TestValue(sout,rstatus);
        it++;
    }

    TestDuplicateResources(sout,rstatus);
}

//------------------------------------------------------------------------------

void CResourceList::TestDuplicateResources(std::ostream& sout,bool& rstatus)
{
    std::list<CResourceValuePtr>::iterator     oit = begin();
    std::list<CResourceValuePtr>::iterator     oie = end();

    while( oit != oie ){

        std::list<CResourceValuePtr>::iterator    iit = begin();
        std::list<CResourceValuePtr>::iterator    iie = end();

        while( iit != iie ){
            if( iit != oit ) {
                if( (*iit)->Name == (*oit)->Name ){
                    if( rstatus == true ) sout << endl;
                    sout << "<b><red> ERROR: Duplicate occurence of '" << (*oit)->Name << "' resource specification!</red></b>" << endl;
                    rstatus = false;
                    return;
                }
            }
            iit++;
        }

        oit++;
    }
}

//------------------------------------------------------------------------------

void CResourceList::AddResourceToBegin(const CSmallString& name,const CSmallString& value)
{
    CResourceValuePtr p_nrv = CResourceValuePtr(new CResourceValue);
    p_nrv->Name = name;
    p_nrv->Value = value;
    push_front(p_nrv);
}

//------------------------------------------------------------------------------

void CResourceList::RemoveAllResources(void)
{
    clear();
}

//------------------------------------------------------------------------------

void CResourceList::RemoveHelperResources(void)
{
    list<CResourceValuePtr>::iterator it = begin();
    list<CResourceValuePtr>::iterator ie = end();

    while( it != ie ){
        CResourceValuePtr p_res = *it;
        if( p_res->IsHelperToken() == true ){
            it = erase(it);
        } else {
            it++;
        }
    }
}

//------------------------------------------------------------------------------

void CResourceList::Finalize(void)
{
    list<CResourceValuePtr>::iterator it = begin();
    list<CResourceValuePtr>::iterator ie = end();

    while( it != ie ){
        CResourceValuePtr p_res = *it;
        p_res->Finalize(Variables);
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

CResourceValuePtr CResourceList::GetResource(const CSmallString& name) const
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

int CResourceList::GetNumOfCPUs(void) const
{
    const CResourceValuePtr p_rv = GetResource("ncpus");
    if( p_rv == NULL ) return(1);
    return( p_rv->Value.ToInt() );
}

//------------------------------------------------------------------------------

int CResourceList::GetNumOfGPUs(void) const
{
    const CResourceValuePtr p_rv = GetResource("ngpus");
    if( p_rv == NULL ) return(0);
    return( p_rv->Value.ToInt() );
}

//------------------------------------------------------------------------------

int CResourceList::GetMaxNumOfCPUsPerNode(void) const
{
    const CResourceValuePtr p_rv = GetResource("maxcpuspernode");
    if( p_rv == NULL ) return(1);
    return( p_rv->Value.ToInt() );
}

//------------------------------------------------------------------------------

CSmallString CResourceList::GetResourceValue(const CSmallString& name) const
{
    const CResourceValuePtr p_rv = GetResource(name);
    if( p_rv == NULL ) return("");
    return( p_rv->Value );
}

//------------------------------------------------------------------------------

long int CResourceList::GetMemory(void) const
{
    const CResourceValuePtr p_rv = GetResource("mem");
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
    const CResourceValuePtr p_rv = GetResource("walltime");
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

void CResourceList::GetTorqueResources(struct attropl* &p_prev)
{
    std::list<CResourceValuePtr>::iterator     it = begin();
    std::list<CResourceValuePtr>::iterator     ie = end();

    while( it != ie ){
        CResourceValuePtr p_res = (*it);
        if( p_res->IsHelperToken() ) {
            it++;       // skip helper tokens
            continue;
        }

        set_attribute(p_prev,ATTR_l,p_res->Name,p_res->Value);
        it++;
    }
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

