// =============================================================================
//  ABS - Advanced Batch System
// -----------------------------------------------------------------------------
//    Copyright (C) 2012 Petr Kulhanek (kulhanek@chemi.muni.cz)
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

#include <JobType.hpp>

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

CJobType::CJobType(CPluginObject* p_objectinfo)
    : CComObject(p_objectinfo)
{
}

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

ERetStatus CJobType::DetectJobType(CJob& job,bool& detected,std::ostream& sout)
{
    // by default - the job is not of this type
    detected = false;
    return(ERS_OK);
}

//------------------------------------------------------------------------------

bool CJobType::CheckInputFile(CJob& job,std::ostream& sout)
{
    // by default - the job input file is correct
    return(true);
}

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

