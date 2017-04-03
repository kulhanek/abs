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

    ShortServerName = short_srv_name;

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
    CSmallString tmp;
// -----------------
    tmp = NULL;
    get_attribute(p_job->attribs,ATTR_state,NULL,tmp);
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
    } else if( status == "F" ) {
        BatchJobStatus = EJS_FINISHED;
    } else if( status == "E" ) {
        BatchJobStatus = EJS_FINISHED;
    }

// -----------------
    tmp = NULL;
    get_attribute(p_job->attribs,ATTR_name,NULL,tmp);
    SetItem("batch/job","INF_JOB_TITLE",tmp);

// -----------------
    tmp = NULL;
    get_attribute(p_job->attribs,ATTR_owner,NULL,tmp);
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
    get_attribute(p_job->attribs,ATTR_queue,NULL,tmp);
    SetItem("batch/job","INF_JOB_QUEUE",tmp);
    SetItem("specific/resources","INF_QUEUE",tmp);

// job variables
    BatchVariables.clear();
    tmp = NULL;
    get_attribute(p_job->attribs,ATTR_v,NULL,tmp);
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
    DecodeBatchJobComment(p_job->attribs,BatchJobComment);
    if( (status == "H") && (BatchJobComment == NULL) ){
        BatchJobComment = "Holded";
    }
    SetItem("batch/job","INF_JOB_COMMENT",BatchJobComment);

// ------------------
    tmp = NULL;
    // this is optional
    get_attribute(p_job->attribs,ATTR_o,NULL,tmp);

    SetItem("basic/jobinput","INF_INPUT_MACHINE",BatchVariables["INF_INPUT_MACHINE"]);
    SetItem("basic/jobinput","INF_INPUT_DIR",BatchVariables["INF_INPUT_DIR"]);
    SetItem("basic/jobinput","INF_JOB_NAME",BatchVariables["INF_JOB_NAME"]);
    SetItem("basic/external","INF_EXTERNAL_NAME_SUFFIX",BatchVariables["INF_JOB_NAME_SUFFIX"]);

    if( (GetItem("basic/jobinput","INF_INPUT_MACHINE") == NULL) ||
        (GetItem("basic/jobinput","INF_INPUT_DIR") == NULL) ){
        stmp = tmp;
        items.clear();
        split(items,stmp,is_any_of(":"));
        if( items.size() >= 2 ){
            CSmallString machine = items[0];
            CFileName path = CSmallString(items[1]);
            path = path.GetFileDirectory();
            SetItem("basic/jobinput","INF_INPUT_MACHINE",machine);
            SetItem("basic/jobinput","INF_INPUT_DIR",path);
        } else {
            SetItem("basic/jobinput","INF_INPUT_MACHINE","-unknown-");
            SetItem("basic/jobinput","INF_INPUT_DIR","-unknown-");
        }
    }

// ------------------
    tmp = NULL;
    // this is optional - exec host
    SetItem("start/workdir","INF_MAIN_NODE","-unknown-");
    get_attribute(p_job->attribs,ATTR_exechost2,NULL,tmp);
    if( tmp != NULL ){
        stmp = tmp;
        items.clear();
        split(items,stmp,is_any_of(":"));
        if( items.size() > 0 ){
            SetItem("start/workdir","INF_MAIN_NODE",items[0]);
        }
    }

// ------------------
    // this is optional - resources
    tmp = NULL;
    get_attribute(p_job->attribs,ATTR_l,"ncpus",tmp);
    if( tmp == NULL ) tmp = "0";
    SetItem("specific/resources","INF_NCPUS",tmp);
    tmp = NULL;
    get_attribute(p_job->attribs,ATTR_l,"ngpus",tmp);
    if( tmp == NULL ) tmp = "0";
    SetItem("specific/resources","INF_NGPUS",tmp);
    tmp = NULL;
    get_attribute(p_job->attribs,ATTR_l,"nodect",tmp);
    if( tmp == NULL ) tmp = "1";
    SetItem("specific/resources","INF_NNODES",tmp);

// ------------------
    // this is optional - times
    tmp = NULL;
    get_attribute(p_job->attribs,ATTR_ctime,NULL,tmp);
    SetItem("batch/job","INF_CREATE_TIME",tmp);
    tmp = NULL;
    if( status == "H" ){
        get_attribute(p_job->attribs,ATTR_qtime,NULL,tmp);
        SetItem("batch/job","INF_SUBMIT_TIME",tmp);
    } else {
        get_attribute(p_job->attribs,ATTR_etime,NULL,tmp);
        SetItem("batch/job","INF_SUBMIT_TIME",tmp);
    }

    tmp = NULL;
    get_attribute(p_job->attribs,ATTR_stime,NULL,tmp);
    SetItem("batch/job","INF_START_TIME",tmp);

    tmp = NULL;
    get_attribute(p_job->attribs,ATTR_mtime,NULL,tmp);
    SetItem("batch/job","INF_FINISH_TIME",tmp);

    CSmallTime time;
    get_attribute(p_job->attribs,ATTR_l,"walltime",time);
    SetItem("batch/job","INF_WALL_TIME",time.GetSecondsFromBeginning());

    return(true);
}

//------------------------------------------------------------------------------

void CPBSProJob::DecodeBatchJobComment(struct attrl* p_item,CSmallString& comment)
{
    comment = "";

    while( p_item != NULL ){
        // job comment
        if( strcmp(p_item->name,ATTR_comment) == 0 ){
            if( comment == NULL ) {
                comment = p_item->value;
            }
        }

// FIXME
//        if( strcmp(p_item->name,ATTR_JOB_PLANNED_START) == 0 ){
//            CSmallString tmp = p_item->value;

//            CSmallTimeAndDate   date(tmp.ToLInt());
//            CSmallTimeAndDate   cdate;
//            CSmallTime          ddiff;
//            cdate.GetActualTimeAndDate();
//            ddiff = date - cdate;
//            if( ddiff > 0 ){
//                comment = "planned start within " + ddiff.GetSTimeAndDay();
//            }
//        }

//        if( strcmp(p_item->name,ATTR_JOB_PLANNED_NODES) == 0 ){
//            vector<string> nodes;
//            string         list(p_item->value);
//            split(nodes,list,is_any_of(" ,"),boost::token_compress_on);
//            if( nodes.size() >= 1 ){
//                comment += " (" + nodes[0];
//                if( nodes.size() > 1 ) comment += ",+";
//                comment += ")";
//            }
//        }

        p_item = p_item->next;
    }
}

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

