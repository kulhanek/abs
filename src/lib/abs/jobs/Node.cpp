// =============================================================================
// ABS - Advanced Batch System
// -----------------------------------------------------------------------------
//    Copyright (C) 2011      Petr Kulhanek, kulhanek@chemi.muni.cz
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

#include <Node.hpp>
#include <ErrorSystem.hpp>
#include <iomanip>
#include <boost/algorithm/string/split.hpp>
#include <boost/algorithm/string/join.hpp>
#include <boost/algorithm/string/classification.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/format.hpp>
#include <JobList.hpp>
#include <boost/range/iterator_range.hpp>
#include <boost/algorithm/string/split.hpp>

//------------------------------------------------------------------------------

using namespace std;
using namespace boost;
using namespace boost::algorithm;
using namespace boost::range;

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

CNode::CNode(void)
{
    NCPUs = 0;
    AssignedCPUs = 0;
    NGPUs = 0;
    AssignedGPUs = 0;
    Memory = 0;
    AssignedMemory = 0;

    ScratchLocal = 0;
    ScratchShared = 0;
    ScratchSSD = 0;
}

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

const CSmallString CNode::GetShortServerName(void)
{
    return(ShortServerName);
}

//------------------------------------------------------------------------------

char CNode::GetStateCode(void)
{
    if( IsDown() == true ){
        return('D');
    }

    if( AssignedCPUs == 0 ){
        return('F');
    } else if (NCPUs != AssignedCPUs){
        return('O');
    } else {
        return('E');
    }
}

//------------------------------------------------------------------------------

const std::string CNode::GetNiceSize(size_t size)
{
    // initial size is in kB

    stringstream str;
    char         unit;

    if( size < 1024 ){
        unit = 'K';
    }
    if( size > 1024 ){
        unit = 'M';
        size /= 1024;
    }
    if( size > 1024 ){
        unit = 'G';
        size /= 1024;
    }
    if( size > 1024 ){
        unit = 'T';
        size /= 1024;
    }
    if( size > 1024 ){
        unit = 'P';
        size /= 1024;
    }

    // size is in kB
    str << size << unit;

    return(str.str());
}

//------------------------------------------------------------------------------

void CNode::PrintLineInfo(std::ostream& sout,const std::set<std::string>& gprops,int ncolumns)
{
    if( IsDown() ){
        sout << "<red>";
    }

    sout << left;
    sout << ShortServerName;

    sout << " ";
    CSmallString name = Name;
    if( Name.GetLength() > 12 ){
        name = name.GetSubStringFromTo(0,11);
    }
    sout << setw(12) << name;

    sout << " " << GetStateCode();

    sout << right;
// ------------------
    if( ! IsDown() ){
        if( AssignedCPUs == 0 ){
            sout << "<b><green>";
        } else if (NCPUs != AssignedCPUs){
            sout << "<green>";
        } else {
            sout << "<blue>";
        }
    }
    sout << " " << setw(3) << NCPUs-AssignedCPUs;
// ------------------
    sout << "/" << setw(3) << NCPUs;
// ------------------
    if( ! IsDown() ){
        if( AssignedCPUs == 0 ){
            sout << "</green></b>";
        } else if (NCPUs != AssignedCPUs){
            sout << "</green>";
        } else {
            sout << "</blue>";
        }
    }
// ------------------
    if( ! IsDown() ){
        if( (AssignedGPUs == 0) && (NGPUs > 0) ){
            sout << "<b><green>";
        } else if (NGPUs != AssignedGPUs){
            sout << "<green>";
        } else if(NGPUs > 0) {
            sout << "<blue>";
        }
    }
    sout << " " << setw(2) << NGPUs-AssignedGPUs;
// ------------------
    sout << "/" << setw(2) << NGPUs;
// ------------------
    if( ! IsDown() ){
        if( (AssignedGPUs == 0) && (NGPUs > 0) ){
            sout << "</green></b>";
        } else if (NGPUs != AssignedGPUs){
            sout << "</green>";
        } else if(NGPUs > 0) {
            sout << "</blue>";
        }
    }
// ------------------

    sout << " " << setw(5) << GetNiceSize(Memory-AssignedMemory);
    sout << "/" << setw(5) << GetNiceSize(Memory);

    sout << " " << setw(5) << GetNiceSize(ScratchLocal);
    sout << " " << setw(5) << GetNiceSize(ScratchShared);
    sout << " " << setw(5) << GetNiceSize(ScratchSSD);

    if( IsDown() ){
        sout << "</red>";
    }

    sout << right;
    sout << " ";

    // print extra properties
    const set<string> eprops = GetExtraProperties(gprops);
    set<string>::const_iterator eit = eprops.begin();
    set<string>::const_iterator eie = eprops.end();

// # ---------------------------- ---- ---- ---- ---- -------------------- -----------------------

    int len = 73;
    while( eit != eie ){
        string str = *eit;
        len += str.size();
        eit++;
        if( eit != eie ){
            if( len + 4 > ncolumns ){
                len = 73;
                sout << endl;
                sout << "                                                                        ";
                len += str.size();
            }
        }
        sout << str;
        if( eit != eie ){
            sout << ",";
            len++;
        }
    }
    sout << endl;
    if( Comment != NULL ){
        if( IsDown() ){
            sout << "<red>";
        }
        sout << "               " << left << Comment << endl;
        if( IsDown() ){
            sout << "</red>";
        }
    }
}

//------------------------------------------------------------------------------

void CNode::PrintJobsInfo(std::ostream& sout,const std::string& jobid)
{
    std::set<std::string>::iterator it = JobList.begin();
    std::set<std::string>::iterator ie = JobList.end();

    while( it != ie ){
        std::string job = *it;
        int    nslots = JobSlots[job].size();
        std::string sslots = join(JobSlots[job],"/");
     // sout << "# ------------ ";
        if( jobid.empty() ){
            sout << "  > <b><blue>";
            sout << job << "(ncpus=" << nslots << ",slots=" << sslots <<")</blue></b>" << std::endl;
        } else if( jobid == job){
            sout << "++> <b><blue>";
            sout << job << "(ncpus=" << nslots << ",slots=" << sslots <<")</blue></b>" << std::endl;
        } else {
            sout << "  > <blue>";
            sout << job << "(ncpus=" << nslots << ",slots=" << sslots <<")</blue>" << std::endl;
        }

        it++;
    }
}

//------------------------------------------------------------------------------

const CSmallString& CNode::GetName(void) const
{
    return(Name);
}

//------------------------------------------------------------------------------

const CSmallString& CNode::GetHost(void) const
{
    return(MOM);
}

//------------------------------------------------------------------------------

int CNode::GetNumOfCPUs(void) const
{
    return(NCPUs);
}

//------------------------------------------------------------------------------

int CNode::GetNumOfFreeCPUs(void) const
{
    return(NCPUs-AssignedCPUs);
}

//------------------------------------------------------------------------------

int CNode::GetNumOfGPUs(void) const
{
    return(NGPUs);
}

//------------------------------------------------------------------------------

int CNode::GetNumOfFreeGPUs(void) const
{
    return(NGPUs-AssignedGPUs);
}

//------------------------------------------------------------------------------

long long CNode::GetMemory(void) const
{
    return(Memory);
}

//------------------------------------------------------------------------------

long long CNode::GetFreeMemory(void) const
{
    return(Memory-AssignedMemory);
}

//------------------------------------------------------------------------------

const std::vector<std::string>& CNode::GetPropertyList(void) const
{
    return(PropList);
}

//------------------------------------------------------------------------------

const std::vector<std::string>& CNode::GetAllProps(void) const
{
    return(AllPropList);
}

//------------------------------------------------------------------------------

bool CNode::HasProperty(const std::string& prop)
{
    vector<string>::iterator mit = PropList.begin();
    vector<string>::iterator mie = PropList.end();

    while( mit != mie ){
        if( *mit == prop ) return(true);
        mit++;
    }

    return(false);
}

//------------------------------------------------------------------------------

bool CNode::HasAllProperties(const std::vector<std::string>& props)
{
    vector<string>::const_iterator mit = props.begin();
    vector<string>::const_iterator mie = props.end();

    while( mit != mie ){
        if( HasProperty(*mit) == false ) return(false);
        mit++;
    }

    return(true);
}

//------------------------------------------------------------------------------

bool CNode::HasAnyProperties(const std::vector<std::string>& props)
{
    vector<string>::const_iterator mit = props.begin();
    vector<string>::const_iterator mie = props.end();

    while( mit != mie ){
        if( HasProperty(*mit) == true ) return(true);
        mit++;
    }

    return(props.size() == 0);
}

//------------------------------------------------------------------------------

bool CNode::IsInAnyQueueWithServer(const std::vector<std::string>& qlist)
{
    vector<string>::const_iterator mit = qlist.begin();
    vector<string>::const_iterator mie = qlist.end();

    while( mit != mie ){
        if( IsInQueueWithServer(*mit) == true ) return(true);
        mit++;
    }

    return(qlist.size() == 0);
}

//------------------------------------------------------------------------------

bool CNode::IsInQueueWithServer(const std::string& queue)
{
    vector<string>::iterator mit = QueueList.begin();
    vector<string>::iterator mie = QueueList.end();

    while( mit != mie ){
        std::string nqueue = *mit;
        nqueue += ".";
        nqueue += ShortServerName;
        if( nqueue == queue ) return(true);
        mit++;
    }

    return(false);
}

//------------------------------------------------------------------------------

bool CNode::IsInOwnerList(const std::string& user)
{
    if( NodeOwner == NULL ) return(true); // no owner - keep

    vector<string>::iterator mit = OwnerList.begin();
    vector<string>::iterator mie = OwnerList.end();

    while( mit != mie ){
        std::string nuser = *mit;
        if( nuser == "everybody" ) return(true); // hardcoded "everybody" :-(
        if( nuser == user ) return(true);
        mit++;
    }

    return(false);
}

//------------------------------------------------------------------------------

const std::set<std::string> CNode::GetExtraProperties(const std::set<std::string>& gprops)
{
    set<string> extra;
    vector<string>::iterator mit = PropList.begin();
    vector<string>::iterator mie = PropList.end();

    while( mit != mie ){
        if( gprops.find(*mit) == gprops.end() ){
            extra.insert(*mit);
        }
        mit++;
    }
    return(extra);
}

//------------------------------------------------------------------------------

const CSmallString& CNode::GetType(void) const
{
    return(Type);
}

//------------------------------------------------------------------------------

const std::vector<std::string> CNode::GetStateList(void) const
{
    std::vector<std::string>    states;
    std::string                 tmp = string(State);
    if( ! tmp.empty() ) split(states,tmp,is_any_of(","));
    return(states);
}

//------------------------------------------------------------------------------

bool CNode::IsDown(void) const
{
    return( (State.FindSubString("down") != -1) ||
            (State.FindSubString("maintenance") != -1) ||
            (State.FindSubString("reserved") != -1) ||
            (State.FindSubString("private") != -1)  ||
            (State.FindSubString("xentest") != -1)  ||
            (State.FindSubString("offline") != -1) ||
            (State.FindSubString("state-unknown") != -1) );
}

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

