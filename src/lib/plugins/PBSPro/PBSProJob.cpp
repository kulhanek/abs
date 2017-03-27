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

#include "PBSProJob.hpp"
#include <ErrorSystem.hpp>
#include <pbs_ifl.h>
#include <iomanip>
#include "PBSProAttr.hpp"
#include <boost/algorithm/string/split.hpp>
#include <boost/algorithm/string/join.hpp>
#include <boost/algorithm/string/classification.hpp>
#include <boost/format.hpp>

//------------------------------------------------------------------------------

using namespace std;
using namespace boost;
using namespace boost::algorithm;

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

CPBSProJob::CPBSProJob(void)
{

}

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

bool CPBSProJob::Init(const CSmallString& short_srv_name,struct batch_status* p_job)
{
    if( p_job == NULL ){
        ES_ERROR("p_job is NULL");
        return(false);
    }

    // retrieve
    // jobid
    // jobname
    // jobdir
    // owner
    // queue
    // ncpus
    // ngpus
    // nnodes
    // last change time
    // status

    // create used sections
    CreateSection("batch");
    CreateSection("basic");

    // job ID
    string stmp(p_job->name);
    vector<string> items;
    split(items,stmp,is_any_of("."));
    if( items.size() > 0 ){
        SetItem("batch/job","INF_JOB_ID",items[0]);
    } else {
        SetItem("batch/job","INF_JOB_ID","");
    }

    // get attributes
    bool result = true;
    CSmallString tmp;
// -----------------
    tmp = NULL;
    result &= get_attribute(p_job->attribs,ATTR_JOB_STATE,NULL,tmp);
    SetItem("batch/job","INF_JOB_STATE",tmp);

    CSmallString status;
    status = tmp;
    BatchJobStatus = EJS_NONE;
    if( status == "Q" ){
        BatchJobStatus = EJS_SUBMITTED;
    } else if( status == "H" ) {
        BatchJobStatus = EJS_SUBMITTED;
    } else if( status == "R" ) {
        BatchJobStatus = EJS_RUNNING;
    } else if( status == "C" ) {
        BatchJobStatus = EJS_FINISHED;
    } else if( status == "E" ) {
        BatchJobStatus = EJS_FINISHED;
    }

// -----------------
    tmp = NULL;
    result &= get_attribute(p_job->attribs,ATTR_JOB_TITLE,NULL,tmp);
    SetItem("batch/job","INF_JOB_TITLE",tmp);

// -----------------
    tmp = NULL;
    result &= get_attribute(p_job->attribs,ATTR_JOB_OWNER,NULL,tmp);
    stmp = tmp;
    items.clear();
    split(items,stmp,is_any_of("@"));
    if( items.size() > 0 ){
        SetItem("batch/job","INF_JOB_OWNER",items[0]);
    } else {
        SetItem("batch/job","INF_JOB_OWNER","");
    }

// -----------------
    tmp = NULL;
    result &= get_attribute(p_job->attribs,ATTR_JOB_QUEUE,NULL,tmp);
    SetItem("batch/job","INF_JOB_QUEUE",tmp);
    SetItem("specific/resources","INF_QUEUE",tmp);

// job variables
    BatchVariables.clear();
    tmp = NULL;
    result &= get_attribute(p_job->attribs,ATTR_JOB_VARIABLE_LIST,NULL,tmp);
    std::string varlist = std::string(tmp);
    std::vector<string> vars;
    split(vars,varlist,is_any_of(","));

    std::vector<string>::iterator   vit = vars.begin();
    std::vector<string>::iterator   vie = vars.end();
    while( vit != vie ) {
        std::list<string> items;
        split(items,*vit,is_any_of("="));
        if( items.size() >= 2 ){
            std::string key=items.front();
            items.pop_front(); // remove key
            std::string value = join(items,"=");
            BatchVariables[key] = value;
        }
        vit++;
    }

// -----------------
    // FIXME
//    CTorque::DecodeBatchJobComment(p_job->attribs,BatchJobComment);
//    if( (status == "H") && (BatchJobComment == NULL) ){
//        BatchJobComment = "Holded";
//    }
    SetItem("batch/job","INF_JOB_COMMENT",BatchJobComment);

// ------------------
    tmp = NULL;
    // this is optional
    get_attribute(p_job->attribs,ATTR_JOB_OUTPUT_PATH,NULL,tmp);

    SetItem("basic/jobinput","INF_JOB_MACHINE",BatchVariables["INF_JOB_MACHINE"]);
    SetItem("basic/jobinput","INF_JOB_PATH",BatchVariables["INF_JOB_PATH"]);
    SetItem("basic/jobinput","INF_JOB_NAME",BatchVariables["INF_JOB_NAME"]);
    SetItem("basic/external","INF_EXTERNAL_NAME_SUFFIX",BatchVariables["INF_JOB_NAME_SUFFIX"]);

    if( (GetItem("basic/jobinput","INF_JOB_MACHINE") == NULL) ||
        (GetItem("basic/jobinput","INF_JOB_PATH") == NULL) ){
        stmp = tmp;
        items.clear();
        split(items,stmp,is_any_of(":"));
        if( items.size() >= 2 ){
            CSmallString machine = items[0];
            CFileName path = CSmallString(items[1]);
            path = path.GetFileDirectory();
            SetItem("basic/jobinput","INF_JOB_MACHINE",machine);
            SetItem("basic/jobinput","INF_JOB_PATH",path);
        } else {
            SetItem("basic/jobinput","INF_JOB_MACHINE","-unknown-");
            SetItem("basic/jobinput","INF_JOB_PATH","-unknown-");
        }
    }

// ------------------
    tmp = NULL;
    // this is optional - exec host
    SetItem("start/workdir","INF_MAIN_NODE","-unknown-");
    get_attribute(p_job->attribs,ATTR_JOB_EXEC_HOST,NULL,tmp);
    if( tmp != NULL ){
        stmp = tmp;
        items.clear();
        split(items,stmp,is_any_of("+"));
        if( items.size() > 0 ){
            split(items,items[0],is_any_of("/"));
            if( items.size() > 0 ){
                SetItem("start/workdir","INF_MAIN_NODE",items[0]);
            }
        }
    }

// ------------------
    tmp = NULL;
    // this is optional - resources
    int nnodes = 0;
    int ncpus = 0;
    int ngpus = 0;
    get_attribute(p_job->attribs,ATTR_JOB_RESOURCE_LIST,RESOURCES_SELECT,tmp);


    stmp = tmp;
    items.clear();
    split(items,stmp,is_any_of("+"));

    vector<string>::iterator it = items.begin();
    vector<string>::iterator ie = items.end();

    while( it != ie ){
        vector<string> res;
        split(res,*it,is_any_of(":"));

        // first is node or number of nodes
        int lnodes = 1;

        vector<string>::iterator rit = res.begin();
        vector<string>::iterator rie = res.end();

        if( rit != rie ){
            // is it number?
            stringstream str(*rit);
            str >> lnodes;
            if( str.fail() ){
                lnodes = 1;
            }
        }

        // find number of cpus and gpus
        rit++;
        int lncpus=1;
        int lngpus=0;
        while( rit != rie ){
            if( (*rit).find("ppn=") == 0 ){
                vector<string> ritems;
                split(ritems,*rit,is_any_of("="));
                if( ritems.size() >= 2 ){
                    stringstream str(ritems[1]);
                    str >> lncpus;
                }
            }
            if( (*rit).find("ncpus=") == 0 ){
                vector<string> ritems;
                split(ritems,*rit,is_any_of("="));
                if( ritems.size() >= 2 ){
                    stringstream str(ritems[1]);
                    str >> lncpus;
                }
            }
            // FIXME
//            switch( TorqueConfig.GetTorqueMode() ){
//                case ETM_TORQUE:
//                    if( (*rit).find("gpus=") == 0 ){
//                        vector<string> ritems;
//                        split(ritems,*rit,is_any_of("="));
//                        if( ritems.size() >= 2 ){
//                            stringstream str(ritems[1]);
//                            str >> lngpus;
//                        }
//                    }
//                break;
//                case ETM_TORQUE_METAVO:
//                    if( (*rit).find("gpu=") == 0 ){
//                        vector<string> ritems;
//                        split(ritems,*rit,is_any_of("="));
//                        if( ritems.size() >= 2 ){
//                            stringstream str(ritems[1]);
//                            str >> lngpus;
//                        }
//                    }
//                break;
//                case ETM_PBSPRO:
//                    // FIXME
//                break;
//            }
            rit++;
        }
        nnodes += lnodes;
        ncpus += lnodes*lncpus;
        ngpus += lnodes*lngpus;

        it++;
    }

    SetItem("specific/resources","INF_NCPU",ncpus);
    SetItem("specific/resources","INF_NGPU",ngpus);
    SetItem("specific/resources","INF_NNODE",nnodes);

// ------------------
    // this is optional - times
    tmp = NULL;
    get_attribute(p_job->attribs,ATTR_JOB_CREATE_TIME,NULL,tmp);
    SetItem("batch/job","INF_CREATE_TIME",tmp);
    tmp = NULL;
    if( status == "H" ){
        get_attribute(p_job->attribs,ATTR_JOB_HOLD_TIME,NULL,tmp);
        SetItem("batch/job","INF_SUBMIT_TIME",tmp);
    } else {
        get_attribute(p_job->attribs,ATTR_JOB_SUBMIT_TIME,NULL,tmp);
        SetItem("batch/job","INF_SUBMIT_TIME",tmp);
    }
    tmp = NULL;
    // FIXME
//    switch( TorqueConfig.GetTorqueMode() ){
//        case ETM_TORQUE:
//        case ETM_TORQUE_METAVO:
//            get_attribute(p_job->attribs,ATTR_JOB_START_TIME,NULL,tmp);
//        break;
//        case ETM_PBSPRO:
            get_attribute(p_job->attribs,ATTR_JOB_START_TIME_2,NULL,tmp);
//        break;
//    }

    SetItem("batch/job","INF_START_TIME",tmp);
    tmp = NULL;
    get_attribute(p_job->attribs,ATTR_JOB_FINISH_TIME,NULL,tmp);
    SetItem("batch/job","INF_FINISH_TIME",tmp);

// ------------------
    if( ! result ){
        CSmallString error;
        error << "unable to get attribute(s) of job '" << p_job->name << "'";
        ES_TRACE_ERROR(error);
    }

    return(result);
}

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

