// =============================================================================
// ABS - Advanced Batch System
// -----------------------------------------------------------------------------
//    Copyright (C) 2017 Petr Kulhanek, kulhanek@chemi.muni.cz
//    Copyright (C) 2011-2012 Petr Kulhanek, kulhanek@chemi.muni.cz
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

#include <RVUMask.hpp>
#include <CategoryUUID.hpp>
#include <ABSModule.hpp>
#include <ResourceList.hpp>
#include <pbs_ifl.h>

// -----------------------------------------------------------------------------

CComObject* RVUMaskCB(void* p_data);

CExtUUID        RVUMaskID(
                    "{UMASK:8dafea8a-5090-4439-b6fc-0ac8228ff594}",
                    "umask");

CPluginObject   RVUMaskObject(&ABSPlugin,
                    RVUMaskID,RESOURCES_CAT,
                    RVUMaskCB);

// -----------------------------------------------------------------------------

CComObject* RVUMaskCB(void* p_data)
{
    CComObject* p_object = new CRVUMask();
    return(p_object);
}

//------------------------------------------------------------------------------

using namespace std;

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

CRVUMask::CRVUMask(void)
    : CResourceValue(&RVUMaskObject)
{
}

//------------------------------------------------------------------------------

void CRVUMask::TestValue(CResourceList* p_rl,std::ostream& sout,bool& rstatus)
{
    // FIXME
//    if( Value == "current" ){
//        Value = User.GetUMask();
//    }  else if ( Value == "jobdir" ) {
//        Value = User.GetUMask(); // fallback value
//        struct stat info;
//        CSmallString pwd;
//        if( CFileSystem::GetCurrentDir(pwd) == true ){
//            if( stat(pwd, &info) == 0 ){
//                mode_t mode = info.st_mode;
//                int u = (mode & 0700) >> 6;
//                int g = (mode & 0070) >> 3;
//                int o = mode & 0007;
//                // invert mode
//                u = (~u) & 0x7;
//                g = (~g) & 0x7;
//                o = (~o) & 0x7;
//                stringstream str;
//                str << u << g << o;
//                Value = str.str().c_str();
//            }
//        }
//    }

    if( Value.GetLength() != 3 ){
        if( rstatus == true ) sout << endl;
        sout << "<b><red> ERROR: The user file creation mask (umask) must be composed from three numbers!</red></b>" << endl;
        rstatus = false;
        return;
    }
    for(int i=0; i < 3; i++){
        if( (Value[i] < '0') || (Value[i] > '7') ){
            if( rstatus == true ) sout << endl;
            sout << "<b><red> ERROR: Illegal number '" << Value[i] << "' at possition " << i+ 1 << " in the user file creation mask (umask)!" << endl;
            sout <<         "        Allowed values: 0-7</red></b>" << endl;
            rstatus = false;
            return;
        }
    }
}

//------------------------------------------------------------------------------

void CRVUMask::GetAttribute(CSmallString& name, CSmallString& resource, CSmallString& value)
{
    name = ATTR_umask;
    resource = NULL;
    value = Value;
}

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

