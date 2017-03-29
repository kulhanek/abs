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
#include <iomanip>
#include <XMLElement.hpp>
#include <QueueList.hpp>
#include <boost/algorithm/string/split.hpp>
#include <boost/algorithm/string/classification.hpp>
#include <NodeList.hpp>
#include <ABSConfig.hpp>
#include <BatchServers.hpp>

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
               const CSmallString& resources)
{
    SystemAlias = false;
    Name = name;
    Destination = queue;
    Resources.Parse(resources);
}

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

bool CAlias::TestAlias(std::ostream& sout)
{
    bool result = true;

    CSmallString srv_name,srv_short,queue;
    if( BatchServers.DecodeQueueName(Destination,srv_name,srv_short,queue) == false ){
        ES_TRACE_ERROR("unable to decode alias destination (queue[@server])");
        // something was wrong - exit
        return(false);
    }

    // list queues
    CBatchServerPtr srv_ptr = BatchServers.FindBatchServer(srv_name);
    if( srv_ptr ){
        srv_ptr->GetQueues(QueueList);
    } else {
        ES_TRACE_ERROR("unable to init batch server");
    }

    // does queue exist?
    CQueuePtr p_queue = QueueList.FindQueue(queue);
    if( p_queue == NULL ){
        ES_TRACE_ERROR("queue does not exist or is not allowed");
        if( result == true ) sout << endl;
        sout << "<b><red> ERROR: The '" << queue << "' queue does not exist or is not accessible to the user on the server '" << srv_name << "'!</red></b>" << endl;
        result = false;
    }

    // are all resources valid?
    Resources.SetBatchServerName(srv_name);
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
    p_ele->SetAttribute("res",Resources.ToString(false));

}

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

