#ifndef UpdateInfoFileH
#define UpdateInfoFileH
// =============================================================================
// ABS - Advanced Batch System
// -----------------------------------------------------------------------------
//    Copyright (C) 2012      Petr Kulhanek, kulhanek@chemi.muni.cz
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

#include "UpdateInfoFileOptions.hpp"
#include <Job.hpp>

// -----------------------------------------------------------------------------

class CUpdateInfoFile{
public:
// constructor -----------------------------------------------------------------
        CUpdateInfoFile(void);

// main methods ----------------------------------------------------------------
    //! init options
    int Init(int argc,char* argv[]);

    //! main part of program
    bool Run(void);

    //! finalize program
    void Finalize(void);

// section of private data -----------------------------------------------------
    private:
    CUpdateInfoFileOptions  Options;
    CJob                    Job;

    bool ArchiveRuntimeFiles(void);
};

// -----------------------------------------------------------------------------

#endif
