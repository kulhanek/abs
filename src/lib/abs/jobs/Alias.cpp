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

#include <Alias.hpp>
#include <ErrorSystem.hpp>
#include <pbs_ifl.h>
#include <iomanip>
#include <TorqueAttr.hpp>
#include <XMLElement.hpp>
#include <QueueList.hpp>
#include <TorqueConfig.hpp>
#include <boost/algorithm/string/split.hpp>
#include <boost/algorithm/string/classification.hpp>
#include <GlobalConfig.hpp>
#include <NodeList.hpp>

//------------------------------------------------------------------------------

using namespace std;
using namespace boost;
using namespace boost::algorithm;

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

CAlias::CAlias(void)
{
    SystemAlias = false;
}

//------------------------------------------------------------------------------

CAlias::CAlias(const CSmallString& name,const CSmallString& queue,
               const CSmallString& sync,const CSmallString& resources)
{
    SystemAlias = false;
    Name = name;
    Destination = queue;
    SyncMode = sync;
    Resources.Parse(resources);
}

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

bool CAlias::TestAlias(std::ostream& sout)
{
    bool result = true;

    CSmallString dest = Destination;
    CSmallString host;
    // override destination
    string          sdest = string(Destination);
    vector<string>  items;
    split(items,sdest,is_any_of("@"));
    if( items.size() > 1 ){
        host = items[0];
        dest = items[1];
    } else if( items.size() == 1 ) {
        dest = items[0];
    }
    if( (host == "local") || (host == "localhost") ){
        host = GlobalConfig.GetHostName();
    }

    // does queue exist?
    CQueuePtr p_queue = QueueList.FindQueue(dest);
    if( p_queue == NULL ){
        ES_TRACE_ERROR("queue does not exist or is not allowed");
        if( result == true ) sout << endl;
        sout << "<b><red> ERROR: The '" << dest << "' queue does not exist or is not accessible to the user!</red></b>" << endl;
        result = false;
    }

    // test if node is available and is suitable for given queue
    if( host != NULL ){
        if( NodeList.FindNode(host) == NULL ){
            if( result == true ) sout << endl;
            sout << "<b><red> ERROR: The '" << host << "' node does not exist!</red></b>" << endl;
            result = false;
        }
        if( result == true ){
            if( NodeList.FindNode(host,p_queue) == NULL ){
                if( result == true ) sout << endl;
                sout << "<b><red> ERROR: The '" << host << "' node does not have required propery of  '" << dest << "' queue!</red></b>" << endl;
                result = false;
            }
        }
    }

    // is sync mode supported?
    CSmallString allowed_sync_modes="sync";
    TorqueConfig.GetSystemConfigItem("INF_SUPPORTED_SYNCMODES",allowed_sync_modes);

    vector<string>  sync_modes;
    string sl = string(allowed_sync_modes);
    split(sync_modes,sl,is_any_of(":"));

    if( find(sync_modes.begin(),sync_modes.end(),string(SyncMode)) == sync_modes.end() ){
        ES_TRACE_ERROR("syncmode is not supported");
        if( result == true ) sout << endl;
        sout << "<b><red> ERROR: The '" << SyncMode << "' syncmode is not supported on the active site!</red></b>" << endl;
        result = false;
    }

    // are all resources valid?
    Resources.TestResources(sout,result);

    return(result);
}

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

void CAlias::PrintLineInfo(std::ostream& sout)
{
    sout << left;
    sout << setw(1);
    if( SystemAlias ){
        sout << "S";
    } else {
        sout << "U";
    }
    sout << " " << setw(14) << Name;
    sout << " " << setw(16) << Destination;
    sout << " " << setw(12) << SyncMode;
    sout << left;
    sout << " " << Resources.ToString(true);
    sout << endl;
}

//------------------------------------------------------------------------------

bool CAlias::IsSystemAlias(void) const
{
    return(SystemAlias);
}

//------------------------------------------------------------------------------

const CSmallString& CAlias::GetName(void) const
{
    return(Name);
}

//------------------------------------------------------------------------------

const CSmallString& CAlias::GetDestination(void) const
{
    return(Destination);
}

//------------------------------------------------------------------------------

const CSmallString& CAlias::GetSyncMode(void) const
{
    return(SyncMode);
}

//------------------------------------------------------------------------------

const CResourceList& CAlias::GetResources(void) const
{
    return(Resources);
}

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

bool CAlias::Load(CXMLElement* p_ele,bool system)
{
    if( p_ele == NULL ){
        ES_ERROR("p_ele is NULL");
        return(false);
    }
    if( p_ele->GetName() != "alias" ){
        ES_ERROR("p_ele is not <alias>");
        return(false);
    }

    SystemAlias = system;

    bool result = true;
    result &= p_ele->GetAttribute("name",Name);
    result &= p_ele->GetAttribute("dest",Destination);
    result &= p_ele->GetAttribute("sync",SyncMode);
    CSmallString res;
    result &= p_ele->GetAttribute("res",res);
    if( result ){
        Resources.Parse(res);
    }

    if( result == false ){
        ES_ERROR("unable to get alias attribute(s)");
        return(false);
    }

    return(true);
}

//------------------------------------------------------------------------------

void CAlias::Save(CXMLElement* p_ele)
{
    if( p_ele == NULL ){
        LOGIC_ERROR("p_ele is NULL");
    }
    if( p_ele->GetName() != "alias" ){
        LOGIC_ERROR("p_ele is not <alias>");
    }

    p_ele->SetAttribute("name",Name);
    p_ele->SetAttribute("dest",Destination);
    p_ele->SetAttribute("sync",SyncMode);
    p_ele->SetAttribute("res",Resources.ToString(false));

}

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

