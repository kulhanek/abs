#ifndef GaussianJobTypeH
#define GaussianJobTypeH
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

#include "BasicJobTypesMainHeader.hpp"
#include <JobType.hpp>

// -----------------------------------------------------------------------------

class CGaussianJobType : public CJobType {
public:
// constructor -----------------------------------------------------------------
        CGaussianJobType(void);

// executive methods -----------------------------------------------------------
    /// determine if the job is job of given type
    virtual ERetStatus DetectJobType(CJob& job,bool& detected,std::ostream& sout);

    /// check job input file
    virtual bool CheckInputFile(CJob& job,std::ostream& sout);

    /// update number of ncpus in job input file
    bool UpdateNProcShared(CJob& job,const CSmallString& name,int nprocshared);

    /// get number of ncpus specified in the job input file
    int GetNProcShared(CJob& job,const CSmallString& name);

    /// update memory - in MB
    bool UpdateMemory(CJob& job,const CSmallString& name,long long mem);

    /// get memory from the file - in MB
    long long GetMemory(CJob& job,const CSmallString& name);
};

// -----------------------------------------------------------------------------

#endif
