#ifndef PBSProAttrH
#define PBSProAttrH
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

#include <ABSMainHeader.hpp>
#include <SmallString.hpp>
#include <SmallTime.hpp>
#include <vector>
#include <pbs_ifl.h>

// -----------------------------------------------------------------------------

bool get_attribute(struct attrl* p_first,const char* p_name,const char* p_res,
                   bool& value,bool emit_error=false);
bool get_attribute(struct attrl* p_first,const char* p_name,const char* p_res,
                   int& value,bool emit_error=false);
bool get_attribute(struct attrl* p_first,const char* p_name,const char* p_res,
                   CSmallString& value,bool emit_error=false);
bool get_attribute(struct attrl* p_first,const char* p_name,const char* p_res,
                   CSmallTime& value,bool emit_error=false);
bool get_attribute(struct attrl* p_first,const char* p_name,const char* p_res,
                   std::vector<CSmallString>& values,const char* p_delim=",",bool emit_error=false);
void set_attribute(struct attropl* &p_prev,const char* p_name,const char* p_res,const char* p_value);
void set_attribute(struct attropl* &p_prev,const char* p_name,const char* p_res,
                   const char* p_value,enum batch_op op);
void set_attribute(struct attrl* &p_prev,const char* p_name,const char* p_res,const char* p_value);

// -----------------------------------------------------------------------------

struct attrl* FindAttr(struct attrl* p_list,const char* p_name,const char* p_res,
                       bool emit_error);

// -----------------------------------------------------------------------------

#define ATTR_QUEUE_TYPE                 "queue_type"
#define ATTR_QUEUE_PRIORITY             "Priority"
#define ATTR_QUEUE_ACL_USER_ENABLED     "acl_user_enable"
#define ATTR_QUEUE_ACL_USERS            "acl_users"
#define ATTR_QUEUE_ACL_GROUP_ENABLED    "acl_group_enable"
#define ATTR_QUEUE_ACL_GROUPS           "acl_groups"
#define ATTR_QUEUE_MAX_RUNNING          "max_running"
#define ATTR_QUEUE_MAX_USER_RUNNING     "max_user_run"
#define ATTR_QUEUE_TOTAL_JOBS           "total_jobs"
#define ATTR_QUEUE_STARTED              "started"
#define ATTR_QUEUE_ENABLED              "enabled"
#define ATTR_QUEUE_REQUIRED_PROPERTY    "required_property"
#define ATTR_QUEUE_RESOURCES_MAX        "resources_max"
#define ATTR_QUEUE_STATE_COUNT          "state_count"
#define ATTR_QUEUE_ROUTE_DESTINATIONS   "route_destinations"

#define ATTR_NODE_STATE                 "state"
#define ATTR_NODE_NP                    "np"
#define ATTR_NODE_JOBS                  "jobs"
#define ATTR_NODE_PROPERTIES            "properties"
#define ATTR_NODE_TYPE                  "ntype"
#define ATTR_NODE_QUEUE                 "queue"
#define ATTR_NODE_RES_TOTAL             "resources_total"
#define ATTR_NODE_RES_USED              "resources_used"
#define ATTR_NODE_RES_GPU               "gpu"
#define ATTR_NODE_GPUS                  "gpus"
#define ATTR_NODE_RES_AVAIL             "resources_available"
#define ATTR_NODE_RES_ASSIGNED          "resources_assigned"
#define ATTR_NODE_RES_NCPUS             "ncpus"

#define ATTR_JOB_TITLE                  "Job_Name"
#define ATTR_JOB_OWNER                  "Job_Owner"
#define ATTR_JOB_QUEUE                  "queue"
#define ATTR_JOB_STATE                  "job_state"
#define ATTR_JOB_COMMENT                "comment"
#define ATTR_JOB_OUTPUT_PATH            "Output_Path"
#define ATTR_JOB_EXEC_HOST              "exec_host"
#define ATTR_JOB_RESOURCE_LIST          "Resource_List"
#define ATTR_JOB_VARIABLE_LIST          "Variable_List"
#define ATTR_JOB_CREATE_TIME            "ctime"
#define ATTR_JOB_SUBMIT_TIME            "etime"
#define ATTR_JOB_START_TIME             "start_time"
#define ATTR_JOB_START_TIME_2           "stime"
#define ATTR_JOB_FINISH_TIME            "mtime"
#define ATTR_JOB_HOLD_TIME              "qtime"
#define ATTR_JOB_ACCOUNT_NAME           "Account_Name"
#define ATTR_JOB_PLANNED_START          "planned_start"
#define ATTR_JOB_PLANNED_NODES          "planned_nodes"

#define ATTR_USER_LIST                  "User_List"
#define ATTR_group_list                 "group_list"

#define RESOURCES_WALLTIME              "walltime"
#define RESOURCES_PROCS                 "procs"
#define RESOURCES_NODES                 "nodes"
#define RESOURCES_SELECT                "select"
#define RESOURCES_NCPUS                 "ncpus"




// -----------------------------------------------------------------------------

#endif
