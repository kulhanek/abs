#ifndef AlphafoldJobTypeH
#define AlphafoldJobTypeH
// =============================================================================
//  ABS - Advanced Batch System
// -----------------------------------------------------------------------------
//    Copyright (C) 2021 Petr Kulhanek (kulhanek@chemi.muni.cz)
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

class CAlphafoldJobType : public CJobType {
public:
// constructor -----------------------------------------------------------------
        CAlphafoldJobType(void);

// executive methods -----------------------------------------------------------
    /// determine if the job is job of given type
    virtual ERetStatus DetectJobType(CJob& job,bool& detected,std::ostream& sout);

    /// check job input file
    virtual bool CheckInputFile(CJob& job,std::ostream& sout);
};

// -----------------------------------------------------------------------------

#endif
