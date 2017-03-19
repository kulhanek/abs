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

#include <Resources.hpp>
#include <ErrorSystem.hpp>
#include <iomanip>
#include <ctype.h>
#include <Shell.hpp>
#include <string.h>

using namespace std;

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

CResources::CResources(void)
{
     MaxCPUsPerNode = 1;
     NCPUs = 1;
     HostName = CShell::GetSystemVariable("HOSTNAME");
}

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

// local:ncpus[,mem=xxx,key=xxx]
// node:ncpus[,mem=xxx,key=xxx]
// ncpus[:prop[,mem=xxx,key=xxx]]
// prop[,mem=xxx,key=xxx]]

bool CResources::Parse(const CSmallString& resources)
{
    if( resources.GetLength() <= 0 ){
        ES_ERROR("no resources");
        return(false);
    }

    CSmallString loc_copy(resources);
    CSmallString exts;

    // strip-down extensions
    int loc;
    loc = loc_copy.IndexOf(',');
    if( loc >= 0 ){
        exts = loc_copy.GetSubStringFromTo(loc+1,loc_copy.GetLength()-1);
        loc_copy = loc_copy.GetSubStringFromTo(0,loc-1);
    }

    char* p_ext;
    char* p_ext_buff = NULL;
    p_ext = strtok_r(exts.GetBuffer(),",",&p_ext_buff);
    while( p_ext != NULL ){
        Extensions.push_back(p_ext);
        p_ext = strtok_r(NULL,",",&p_ext_buff);
    }

    // find first :
    CSmallString tok1,tok2;

    loc = loc_copy.IndexOf(':');
    if( loc >= 0 ){
        tok1 = loc_copy.GetSubStringFromTo(0,loc-1);
        if( tok1.GetLength() <= 0 ){
            ES_ERROR("first token to the ':' delimiter has zero length");
            return(false);
        }
        tok2 = loc_copy.GetSubStringFromTo(loc+1,loc_copy.GetLength()-1);

        if( isdigit(tok1[0]) ){
            tok1.ToInt(NCPUs);
            Properties = tok2;
        } else {
            NodeName = tok1;
            if( tok2.ToInt(NCPUs) == false ){
                ES_ERROR("second token after ':' delimiter has to be a number");
                return(false);
            }
        }

    } else {
        tok1 = loc_copy;
        if( isdigit(tok1[0]) ){
            if( tok1.ToInt(NCPUs) == false ){
                ES_ERROR("first token has to be a number");
                return(false);
            }
        } else {
            Properties = tok1;
        }
    }

    return(true);
}

//------------------------------------------------------------------------------

bool CResources::MergeWithUser(const CResources& resources,std::ostream& sout)
{
    return(true);
}

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

bool CResources::IsLocal(void)
{
    return( NodeName == "local" );
}

//------------------------------------------------------------------------------

int CResources::GetMaxCPUsPerNode(void)
{
    return( MaxCPUsPerNode );
}

//------------------------------------------------------------------------------

void CResources::SetMaxCPUsPerNode(int set)
{
    MaxCPUsPerNode = set;
}

//------------------------------------------------------------------------------

int CResources::GetNumOfCPUs(void)
{
    return( NCPUs );
}

//------------------------------------------------------------------------------

int CResources::GetNumOfNodes(void)
{
    if( MaxCPUsPerNode <= 0 ) return(1);

    int nodes = NCPUs / MaxCPUsPerNode;
    if( NCPUs % MaxCPUsPerNode > 0 ){
        nodes++;
    }
    return( nodes );
}

//------------------------------------------------------------------------------

const CSmallString CResources::GetNodeName(void)
{
    if( NodeName == "local" ){
        return(HostName);
    } else {
        return(NodeName);
    }
}

//------------------------------------------------------------------------------

const CSmallString CResources::GetProperties(void)
{
    return(Properties);
}

//------------------------------------------------------------------------------

const CSmallString CResources::GetTorqueResources(void)
{
    if( MaxCPUsPerNode <= 0 ){
        ES_ERROR("MaxCPUsPerNode is less than or equal to zero");
        return(false);
    }

    CSmallString resources;
    resources = "nodes=";
    if( NodeName != NULL ){
        resources += GetNodeName();
    } else {
        resources << GetNumOfNodes();
    }

    // number of CPUs
    resources += ":ppn=";
    if( NCPUs < MaxCPUsPerNode ){
        resources += CSmallString(NCPUs);
    } else {
        resources += CSmallString(MaxCPUsPerNode);
    }

    // do we have properties?
    if( Properties != NULL ){
        resources += ":";
        resources += Properties;
    }

    // add extensions
    for(unsigned int i=0; i < Extensions.size(); i++){
        resources += ",";
        resources += Extensions[i];
    }

    return(resources);
}

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

