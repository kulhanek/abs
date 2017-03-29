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
                   size_t& value,bool emit_error=false);
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

#define ATTR_USER_LIST                  "User_List"
#define ATTR_group_list                 "group_list"

#define RESOURCES_WALLTIME              "walltime"
#define RESOURCES_PROCS                 "procs"
#define RESOURCES_NODES                 "nodes"
#define RESOURCES_SELECT                "select"
#define RESOURCES_NCPUS                 "ncpus"

// -----------------------------------------------------------------------------

#endif
