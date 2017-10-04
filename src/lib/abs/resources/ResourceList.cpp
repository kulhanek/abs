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
#include <iostream>
#include <BatchServers.hpp>

//------------------------------------------------------------------------------

using namespace std;
using namespace boost;
using namespace boost::algorithm;

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

CResourceList::CResourceList(void)
{
    Job = NULL;
}

//------------------------------------------------------------------------------

CResourceList::~CResourceList(void)
{
}

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

void CResourceList::SetJob(CJob* p_job)
{
    Job = p_job;
}

//------------------------------------------------------------------------------

CJob* CResourceList::GetJob(void)
{
    return(Job);
}

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

void CResourceList::AddResources(const CSmallString& resources,std::ostream& sout,
                                 bool& rstatus, bool expertmode)
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
        if( sres[0] == '+' ){
            command = '+';
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
                AddResource(name,value,sout,rstatus,expertmode);
                break;
            case '-':{
                if( ! value.empty() ){
                    if( rstatus == true ) sout << endl;
                    sout << "<b><red> ERROR: Value cannot be specified for removed resource '" << sres << "'!</red></b>" << endl;
                    rstatus = false;
                    return;
                }
                RemoveResource(name);
                break;
            }
        }
    }
}

//------------------------------------------------------------------------------

void CResourceList::AddResource(const CSmallString& name,const CSmallString& value,std::ostream& sout,
                                bool& rstatus, bool expertmode)
{
    // does name contain a batch server name?
    string srv;
    string sname(name);
    string fname(name);
    size_t qpos = sname.find("?");
    if( qpos != string::npos ){
        if( qpos == 0 ){
            if( rstatus == true ) sout << endl;
            sout << "<b><red> ERROR: Resource '" << name << "' does not contain the short name (S) of batch server (S?name=value)!</red></b>" << endl;
            rstatus = false;
            return;
        }
        if( qpos+1 == sname.size() ){
            if( rstatus == true ) sout << endl;
            sout << "<b><red> ERROR: Resource '" << name << "' does not contain the name (name) of resource (S?name=value)!</red></b>" << endl;
            rstatus = false;
            return;
        }
        fname = sname.substr(qpos+1,string::npos);
        srv = sname.substr(0,qpos);
        if( srv.size() != 1 ){
            if( rstatus == true ) sout << endl;
            sout << "<b><red> ERROR: Resource '" << name << "' does not contain the short name (S) of batch server (S?name=value) but '" << srv << "'!</red></b>" << endl;
            rstatus = false;
            return;
        }
        if( srv == "*" ){
            if( rstatus == true ) sout << endl;
            sout << "<b><red> ERROR: Resource '" << name << "' cannot contain a star character as the short name (S) of batch server (S?name=value)!</red></b>" << endl;
            rstatus = false;
            return;
        }
    }

    // be sure that the resource is unique
    RemoveResource(name);

    // add resource
    CResourceValuePtr res_ptr =  AddEmptyResource(fname,expertmode,true);
    if( res_ptr != NULL ){
        res_ptr->Server = srv;
        res_ptr->Value = value;
        return;
    }

    // plugin object was not found
    if( rstatus == true ) sout << endl;
    sout << "<b><red> ERROR: Resource '" << name << "' is not supported!</red></b>" << endl;
    rstatus = false;
}

//------------------------------------------------------------------------------

void CResourceList::AddSizeResource(const CSmallString& name,long long value)
{
    CResourceValuePtr res_ptr =  AddEmptyResource(name,true,false);
    if( res_ptr != NULL ){
        res_ptr->SetSize(value);
    }
}

//------------------------------------------------------------------------------

void CResourceList::AddRawResource(const CSmallString& name,const CSmallString& value)
{
    CResourceValuePtr res_ptr =  AddEmptyResource(name,true,false);
    if( res_ptr != NULL ){
        res_ptr->Value = value;
    }
}

//------------------------------------------------------------------------------

CResourceValuePtr CResourceList::AddEmptyResource(const CSmallString& name,bool expertmode,bool do_not_remove)
{
    // be sure that the resource is unique
    if( do_not_remove == false ) RemoveResource(name);

    // try to find resource plugin object
    CSimpleIteratorC<CPluginObject> I(PluginDatabase.GetObjectList());
    CPluginObject* p_pobj;
    while( (p_pobj = I.Current()) ){
        if( p_pobj->GetCategoryUUID() == RESOURCES_CAT ){
            CSmallString res_name = p_pobj->GetObjectUUID().GetName();
            if( name == res_name ){
                CComObject* p_obj = p_pobj->CreateObject(NULL);
                CResourceValue* p_res = dynamic_cast<CResourceValue*>(p_obj);
                if( p_res != NULL ){
                    CResourceValuePtr res_ptr(p_res);
                    push_back(res_ptr);
                    return(res_ptr);
                } else {
                    delete p_obj;
                }
            }
        }
        I++;
    }
    if( expertmode == false ) return(CResourceValuePtr());

    // create generic resource
    CSmallString gen_name = BatchServers.GetGenericResourceName();
    if( gen_name == NULL ) return(CResourceValuePtr());

    // must be always false to avoid infinite recursion
    CResourceValuePtr gen_res = AddEmptyResource(gen_name,false,do_not_remove);
    // override resource name
    gen_res->Name = name;

    return(gen_res);
}

//------------------------------------------------------------------------------

void CResourceList::RemoveResource(const CSmallString& name)
{
    // does name contain a batch server name?
    string srv;
    string sname(name);
    string fname(name);
    size_t qpos = sname.find("?");
    if( qpos != string::npos ){
        if( qpos == 0 ){
            return;
        }
        if( qpos+1 == sname.size() ){
            return;
        }
        fname = sname.substr(qpos+1,string::npos);
        srv = sname.substr(0,qpos);
        if( srv.size() != 1 ){
            return;
        }
    }

    std::list<CResourceValuePtr>::iterator     it = begin();
    std::list<CResourceValuePtr>::iterator     ie = end();

    // it is necessary to run the whole cycle because of wild server name
    while( it != ie ){
        CResourceValuePtr p_rv = *it;
        if( ( (srv != "*") && (p_rv->Name == CSmallString(fname)) && (p_rv->Server == CSmallString(srv))) ||
            ( (srv == "*") && (p_rv->Name == CSmallString(fname)) )  ){
            it = erase(it);
        } else {
            it++;
        }
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

void CResourceList::ResolveConflicts(const CSmallString& short_server_name)
{
    // remove all resources that are not general and do not belong to the server
    std::list<CResourceValuePtr>::iterator     it = begin();
    std::list<CResourceValuePtr>::iterator     ie = end();

    while( it != ie ){
        CResourceValuePtr p_rv = *it;
        if( (p_rv->Server != NULL) && (p_rv->Server != short_server_name) ){
            it = erase(it);
        } else {
            it++;
        }
    }

    // merge down resources
    it = begin();
    while( it != ie ){
        CResourceValuePtr p_srv = *it;
        std::list<CResourceValuePtr>::iterator uit = it;
        uit++;
        bool remove = false;
        while( uit != ie ){
            CResourceValuePtr p_mrv = *uit;
            if( p_mrv->Name == p_srv->Name ){
                remove = true;
                break;
            }
            uit++;
        }
        if( remove ){
            it = erase(it);
        } else {
            it++;
        }
    }

    // resolve other conflicts
    it = begin();
    while( it != ie ){
        CResourceValuePtr p_rv = *it;
        p_rv->ResolveConflicts(this);
        it++;
    }
}

//------------------------------------------------------------------------------

void CResourceList::PreTestResourceValues(std::ostream& sout,bool& rstatus)
{
    std::list<CResourceValuePtr>::iterator     it = begin();
    std::list<CResourceValuePtr>::iterator     ie = end();

    while( it != ie ){
        CResourceValuePtr p_rv = *it;
        p_rv->PreTestValue(this,sout,rstatus);
        it++;
    }
}

//------------------------------------------------------------------------------

void CResourceList::PostTestResourceValues(std::ostream& sout,bool& rstatus)
{
    std::list<CResourceValuePtr>::iterator     it = begin();
    std::list<CResourceValuePtr>::iterator     ie = end();

    while( it != ie ){
        CResourceValuePtr p_rv = *it;
        p_rv->PostTestValue(this,sout,rstatus);
        it++;
    }
}

//------------------------------------------------------------------------------

void CResourceList::ResolveDynamicResources(std::ostream& sout,bool& rstatus)
{
    std::list<CResourceValuePtr>::iterator     it;
    std::list<CResourceValuePtr>::iterator     ie;

    // setup dependencies
    it = begin();
    ie = end();
    while( it != ie ){
        CResourceValuePtr p_rv = *it;
        if( find(Dependencies.begin(),Dependencies.end(),p_rv) != Dependencies.end() ){
            // is not in the list yet
            ResolveDependencies(p_rv,sout,rstatus);
        }
        it++;
    }

    // resolve dynamic resources
    it = Dependencies.begin();
    ie = Dependencies.end();

    while( it != ie ){
        CResourceValuePtr p_rv = *it;
        bool delete_me = false;
        p_rv->ResolveDynamicResource(this,delete_me);
        if( delete_me ){
            RemoveResource(p_rv->Name);
            it = Dependencies.erase(it);
        } else {
            it++;
        }
    }

    // do variable substitution
    it = begin();
    ie = end();

    while( it != ie ){
        CResourceValuePtr p_rv = *it;
        if( p_rv->Value.GetLength() >= 2 ){
            if( p_rv->Value[0] == '$' ){
                CSmallString sres = p_rv->Value.GetSubString(1,p_rv->Value.GetLength()-1);
                CResourceValuePtr p_srv = FindResource(sres);
                if( p_srv ){
                    p_rv->Value = p_srv->Value;
                }
            }
        }
        it++;
    }
}

//------------------------------------------------------------------------------

void CResourceList::ResolveDependencies(CResourceValuePtr& p_rv,std::ostream& sout,bool& rstatus)
{
    // resolve requires
    std::vector<CSmallString>::iterator rit = p_rv->Requires.begin();
    std::vector<CSmallString>::iterator rie = p_rv->Requires.end();

    while( rit != rie ){
        CSmallString reqname = *rit;

        bool found = false;

        std::list<CResourceValuePtr>::iterator     ait = begin();
        std::list<CResourceValuePtr>::iterator     aie = end();

        while( ait != aie ){
            CResourceValuePtr p_arv = *ait;
            CResourceValuePtr p_reqrv;
            if( p_arv->Name == reqname ){
                p_reqrv = p_arv;
            }
            if( p_arv->DoesItProvide(reqname) ){
                p_reqrv = p_arv;
            }
            if( p_reqrv ){
                found = true;
                if( find(Dependencies.begin(),Dependencies.end(),p_reqrv) != Dependencies.end() ){
                    Dependencies.push_back(p_reqrv);
                }
            }
            ait++;
        }

        if( ! found ){
            if( rstatus == true ) sout << endl;
            sout << "<b><red> ERROR: Illegal '" << p_rv->Name << "' resource specification!" << endl;
            sout <<         "        This resources requires '" << reqname << ", which is not specifed or provided by other existing reources'!</red></b>" << endl;
            rstatus = false;
            return;
        }

        rit++;
    }

    // insert this resource
    if( find(Dependencies.begin(),Dependencies.end(),p_rv) != Dependencies.end() ){
        Dependencies.push_back(p_rv);
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

int CResourceList::GetNumOfNodes(void) const
{
    const CResourceValuePtr p_rv = FindResource("nnodes");
    if( p_rv == NULL ) return(1);
    return( p_rv->Value.ToInt() );
}

//------------------------------------------------------------------------------

long long CResourceList::GetMemory(void) const
{
    const CResourceValuePtr p_rv = FindResource("mem");
    if( p_rv == NULL ) return(0);
    return(p_rv->GetSize());
}

//------------------------------------------------------------------------------

const CSmallString CResourceList::GetMemoryString(void) const
{
    const CResourceValuePtr p_rv = FindResource("mem");
    if( p_rv == NULL ) return("n.a.");
    return(p_rv->GetSizeString());
}

//------------------------------------------------------------------------------

const CSmallString CResourceList::GetWorkSizeString(void) const
{
    const CResourceValuePtr p_rv = FindResource("worksize");
    if( p_rv == NULL ) return("n.a.");
    return(p_rv->GetSizeString());
}

//------------------------------------------------------------------------------

const CSmallString CResourceList::GetWallTimeString(void) const
{
    const CResourceValuePtr p_rv = FindResource("walltime");
    if( p_rv == NULL ) return("n.a.");
    return(p_rv->GetValue());
}

//------------------------------------------------------------------------------

const CSmallString CResourceList::GetVariables(void) const
{
    std::list<CResourceValuePtr>::const_iterator     it = begin();
    std::list<CResourceValuePtr>::const_iterator     ie = end();

    CSmallString vars;

    while( it != ie ){
        CSmallString name,value;
        (*it)->GetVariable(name,value);
        if( (it != begin()) && (name != NULL) ) vars << ",";
        if( name != NULL ) vars << name << "=" << value;
        it++;
    }

    return(vars);
}

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

