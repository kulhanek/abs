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
    FreeCPUs = 0;
    MaxCPUsPerNode = 0;
    NGPUs = 0;
    FreeGPUs = 0;
}

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

//bool CNode::Init(struct batch_status* p_node)
//{
//    if( p_node == NULL ){
//        ES_ERROR("p_node is NULL");
//        return(false);
//    }

//    Name = p_node->name;

//    CSmallString queue;
//    int          used_NGPUS = 0;
//    int          used_NCPUs = 0;

//    // get attributes
//    bool result = true;
//    result &= get_attribute(p_node->attribs,ATTR_NODE_STATE,NULL,State);
//    switch( ABSConfig.GetTorqueMode() ){
//        case ETM_TORQUE:
//            result &= get_attribute(p_node->attribs,ATTR_NODE_NP,NULL,NCPUs);
//            result &= get_attribute(p_node->attribs,ATTR_NODE_JOBS,NULL,Jobs,false);
//            result &= get_attribute(p_node->attribs,ATTR_NODE_PROPERTIES,NULL,Properties);
//            result &= get_attribute(p_node->attribs,ATTR_NODE_TYPE,NULL,Type,false);
//            result &= get_attribute(p_node->attribs,ATTR_NODE_QUEUE,NULL,queue,false);
//            result &= get_attribute(p_node->attribs,ATTR_NODE_GPUS,NULL,NGPUs,false);
//        break;
//        case ETM_TORQUE_METAVO:
//            result &= get_attribute(p_node->attribs,ATTR_NODE_NP,NULL,NCPUs);
//            result &= get_attribute(p_node->attribs,ATTR_NODE_JOBS,NULL,Jobs,false);
//            result &= get_attribute(p_node->attribs,ATTR_NODE_PROPERTIES,NULL,Properties);
//            result &= get_attribute(p_node->attribs,ATTR_NODE_TYPE,NULL,Type,false);
//            result &= get_attribute(p_node->attribs,ATTR_NODE_QUEUE,NULL,queue,false);
//            result &= get_attribute(p_node->attribs,ATTR_NODE_RES_TOTAL,ATTR_NODE_RES_GPU,NGPUs,false);
//            result &= get_attribute(p_node->attribs,ATTR_NODE_RES_USED,ATTR_NODE_RES_GPU,used_NGPUS,false);
//        break;
//        case ETM_PBSPRO:
//            result &= get_attribute(p_node->attribs,ATTR_NODE_RES_AVAIL,ATTR_NODE_RES_NCPUS,NCPUs);
//            result &= get_attribute(p_node->attribs,ATTR_NODE_RES_ASSIGNED,ATTR_NODE_RES_NCPUS,used_NCPUs);
//            result &= get_attribute(p_node->attribs,ATTR_NODE_JOBS,NULL,Jobs,false);
//            result &= get_attribute(p_node->attribs,ATTR_NODE_TYPE,NULL,Type,false);
//        break;
//    }

//    if( ! result ){
//        CSmallString error;
//        error << "unable to get attribute(s) of node '" << Name << "'";
//        ES_TRACE_ERROR(error);
//    }

//    // if queue is set -> put the name to State
//    if( queue != NULL ){
//        if( State.GetLength() > 0 ) State += ",";
//        State += queue;
//    }

//    // split data
//    std::string tmp = string(Jobs);

//// complex example
//// jobs = 9/373089.sokar.ncbr.muni.cz,14/373184.sokar.ncbr.muni.cz,1/373745.sokar.ncbr.muni.cz,0,2-8,10-13,15-23/373771.sokar.ncbr.muni.cz

//    std::vector< iterator_range<string::iterator> > find_jobs;
//    find_all( find_jobs, tmp, string(ABSConfig.GetServerName()) );

//    std::vector< iterator_range<string::iterator> >::iterator jit = find_jobs.begin();
//    std::vector< iterator_range<string::iterator> >::iterator jie = find_jobs.end();

//    std::vector<std::string> jobs;

//    string::iterator sit = tmp.begin();
//    string::iterator sie;
//    while( jit != jie ){
//      sie = boost::end(*jit);
//      string item(sit,sie);
//      jobs.push_back(item);
//      sie++;
//      sit = sie;
//      jit++;
//    }

//    std::set<std::string>    job_ids;

//    // fix condensed cpus in torque > 5.0?
//    std::vector<std::string>::iterator it = jobs.begin();
//    std::vector<std::string>::iterator ie = jobs.end();
//    while( it != ie ){
//        std::string job = *it;
//        std::vector<std::string> jdscr;
//        split(jdscr,job,is_any_of("/"));
//        if( jdscr.size() == 2 ){
//            job_ids.insert(jdscr[1]);

//            std::vector<std::string> pcdscr;
//            split(pcdscr,jdscr[0],is_any_of(","));

//            for(size_t i=0; i < pcdscr.size(); i++ ){
//                std::vector<std::string> cdscr;
//                split(cdscr,pcdscr[i],is_any_of("-"));
//                if( cdscr.size() == 1 ){
//                    stringstream str;
//                    str << format("%s/%s") % cdscr[0] % jdscr[1];
//                    JobList.push_back(str.str());
//                } else if ( cdscr.size() == 2 ) {
//                    int s = atoi( cdscr[0].c_str() );
//                    int e = atoi( cdscr[1].c_str() );
//                    for(int i = s; i <= e; i++){
//                        stringstream str;
//                        str << format("%d/%s") % i % jdscr[1];
//                        JobList.push_back(str.str());
//                    }

//                } // something wrong?
//            }
//        }
//        it++;
//    }

//    // get number of GPUs
//    switch( ABSConfig.GetTorqueMode() ){
//    case ETM_TORQUE:{
//            if( NGPUs == 0 ) break;
//            std::set<std::string>::iterator it = job_ids.begin();
//            std::set<std::string>::iterator ie = job_ids.end();
//            while( it != ie ){
//                CSmallString jobid(*it);
//                CJobPtr job = Torque.GetJob(jobid);
//                if( job ){
//                    used_NGPUS += job->GetNumOfGPUs();
//                }
//                it++;
//            }
//        }
//        break;
//    default:
//        // nothing to do
//        break;
//    }

//    tmp = string(Properties);
//    if( ! tmp.empty() ) split(PropList,tmp,is_any_of(","));

//    sort(PropList.begin(),PropList.end());

//    switch( ABSConfig.GetTorqueMode() ){
//        case ETM_TORQUE:
//        case ETM_TORQUE_METAVO:
//            // get number of free cpus
//            FreeCPUs = NCPUs - JobList.size();
//            if( FreeCPUs < 0 ) FreeCPUs = 0;
//        break;
//        case ETM_PBSPRO:
//            // nothing to do
//            FreeCPUs = NCPUs - used_NCPUs;
//        break;
//    }

//    // get number of free gpus
//    FreeGPUs = NGPUs - used_NGPUS;

//    // update state
//    std::list<std::string>  states;
//                            tmp = string(State);
//    if( ! tmp.empty() ) split(states,tmp,is_any_of(","));

//    // remove free, job-exclusive  (wrong in MetaCentrum)
//    states.remove("free");
//    states.remove("job-exclusive");

//    // update state
//    if( IsDown() == false ){
//        if( (NCPUs == FreeCPUs) && (FreeCPUs > 0) && (NGPUs == FreeGPUs) ) {
//            states.push_back("free");
//        } else if( FreeCPUs == 0 ) {
//            states.push_back("fully-occupied");
//        } else {
//            states.push_back("partially-free");
//        }
//    }

//    State = join(states, ",");

//    return(result);
//}

//------------------------------------------------------------------------------

void CNode::PrintLineInfo(std::ostream& sout,const std::set<std::string>& gprops,int ncolumns)
{
    sout << left;

    CSmallString name = Name;
    if( Name.GetLength() > 27 ){
        name = name.GetSubStringFromTo(0,26);
    }
    sout << setw(27) << name;

    sout << right;
    stringstream str;

    if( MaxCPUsPerNode > 0 ){
        str << NCPUs << "/" << MaxCPUsPerNode;
    } else {
        str << NCPUs;
    }
    sout << " " << setw(7) << str.str();
    sout << " " << setw(4) << FreeCPUs;
    sout << " " << setw(4) << NGPUs;
    sout << " " << setw(4) << FreeGPUs;
    sout << left;

    if( IsDown() == false ){
        if( (NCPUs == FreeCPUs) && (FreeCPUs > 0) && (NGPUs == FreeGPUs) ){
            sout << " <b><green>" << setw(20) << State << "</green></b>";
        } else if( FreeCPUs == 0 ) {
            sout << " <blue>" << setw(20) << State<< "</blue>";
        } else {
            sout << " <green>" << setw(20) << State<< "</green>";
        }
    } else {
        sout << " <red>" << setw(20) << State << "</red>";
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
}

//------------------------------------------------------------------------------

const CSmallString& CNode::GetName(void) const
{
    return(Name);
}

//------------------------------------------------------------------------------

int CNode::GetNumOfCPUs(void) const
{
    return(NCPUs);
}

//------------------------------------------------------------------------------

int CNode::GetNumOfFreeCPUs(void) const
{
    return(FreeCPUs);
}

//------------------------------------------------------------------------------

int CNode::GetNumOfGPUs(void) const
{
    return(NGPUs);
}

//------------------------------------------------------------------------------

int CNode::GetNumOfFreeGPUs(void) const
{
    return(FreeGPUs);
}

//------------------------------------------------------------------------------

const std::vector<std::string>& CNode::GetPropertyList(void) const
{
    return(PropList);
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
            (State.FindSubString("offline") != -1) );
}

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

