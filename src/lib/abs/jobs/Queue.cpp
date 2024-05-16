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

#include <Queue.hpp>
#include <ErrorSystem.hpp>
#include <boost/algorithm/string/split.hpp>
#include <boost/algorithm/string/classification.hpp>
#include <boost/algorithm/string/join.hpp>
#include <iomanip>

//------------------------------------------------------------------------------

using namespace std;
using namespace boost;
using namespace boost::algorithm;

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

CQueue::CQueue(void)
{
    Enabled = true;
    Started = true;
    Priority = 0;
    TotalJobs = 0;
    QueuedJobs = 0;
    RunningJobs = 0;
    OnlyRoutable = false;
}

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

const CSmallString CQueue::GetShortServerName(void)
{
    return(ShortServerName);
}

//------------------------------------------------------------------------------

bool CQueue::IsStarted(void) const
{
    return(Started);
}

//------------------------------------------------------------------------------

bool CQueue::IsEnabled(void) const
{
    return(Enabled);
}

//------------------------------------------------------------------------------

bool CQueue::IsRouteQueue(void) const
{
    return( (RouteDestinations.size() != 0) && (GetType() == "Route") );
}

//------------------------------------------------------------------------------

bool CQueue::IsOnlyRoutable(void) const
{
    return(OnlyRoutable);
}

//------------------------------------------------------------------------------

bool CQueue::IsRouteDestination(const CSmallString& name)
{
    size_t n = std::count(RouteDestinations.begin(),RouteDestinations.end(),name);
    return( n != 0 );
}

//------------------------------------------------------------------------------

const CSmallString& CQueue::GetName(void) const
{
    return(Name);
}

//------------------------------------------------------------------------------

const CSmallString CQueue::GetNameIncludingShortServerName(void) const
{
    CSmallString name;
    name << Name << "@" << ShortServerName;
    return(name);
}

//------------------------------------------------------------------------------

const CSmallString& CQueue::GetChunkQueueName(void) const
{
    return(ChunkQueue);
}

//------------------------------------------------------------------------------

const CSmallString& CQueue::GetType(void) const
{
    return(Type);
}

//------------------------------------------------------------------------------

bool CQueue::IsAllowed(CUser& user) const
{
    // any ACL?
    if( ACLUsers.empty() && ACLGroups.empty() ) return(true);

    // check users
    for(unsigned int i = 0; i < ACLUsers.size(); i++){
        if( user.GetName() == ACLUsers[i] ) return(true);
    }

    // check groups
    for(unsigned int i = 0; i < ACLGroups.size(); i++){
        if( user.IsInPosixGroup(ACLGroups[i]) ) return(true);
    }

    return(false);
}

//------------------------------------------------------------------------------

void CQueue::PrintLineInfo(std::ostream& sout,bool route_queue)
{
    if( ! OnlyRoutable ){
        sout << "<b><green>";
    }

    sout << left;
    if( route_queue ){
        sout << "  ";
        sout << setw(1) << ShortServerName;
    } else {
        sout << setw(1) << ShortServerName;
        sout << " ";
    }

    sout << left;
    if( route_queue ){
        sout << " > " << setw(16) << Name;
    } else {
        sout << setw(20) << Name;
    }

    sout << right;
    sout << " " << setw(5) << Priority;
    sout << " " << setw(5) << TotalJobs;
    sout << " " << setw(5) << QueuedJobs;
    sout << " " << setw(5) << RunningJobs;
    sout << " " << setw(5) << TotalJobs - (RunningJobs+QueuedJobs);
    sout << " " << setw(13) << MaxWallTime.GetSTimeAndDay();

    if( ! OnlyRoutable ){
        sout << "</green></b>";
    }

    sout << " ";    
    PrintQueueComment(sout);
}

//------------------------------------------------------------------------------

void CQueue::PrintQueueComment(std::ostream& sout)
{
    string          svalue = string(Comment);
    vector<string>  langs;
    vector<string>  items;
    int             nrow, ncolumns = 80;
    CTerminal::GetSize(nrow,ncolumns);

    // split to EN | CS comments
    split(langs,svalue,is_any_of("|"));

    if( langs.size() == 0 ){ // no comments
        sout << endl;
        return;
    }

    // split EN comment into words
    split(items,langs[0],is_any_of(" "),token_compress_on);

    vector<string>::iterator it = items.begin();
    vector<string>::iterator ie = items.end();

    unsigned int title_len = 67;
    unsigned len = title_len;

    while( it != ie ){
        string sres = *it;
        sout << sres;
        len += sres.size();
        len++;
        it++;
        if( it != ie ){
            string sres = *it;
            int tlen = len;
            tlen += sres.size();
            tlen++;
            if( tlen > ncolumns ){
                sout << endl;
                for(unsigned int i=0; i < title_len; i++){
                    sout << " ";
                }
                len = title_len;
            } else {
                sout << " ";
                len += 1;
            }
        }
    }
    sout << endl;
}

//------------------------------------------------------------------------------

const CSmallTime& CQueue::GetMaxWallTime(void) const
{
    return(MaxWallTime);
}

//------------------------------------------------------------------------------

const CSmallTime& CQueue::GetDefaultWallTime(void) const
{
    return(DefaultWallTime);
}

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

