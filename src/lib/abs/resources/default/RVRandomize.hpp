#ifndef RVRandomizeH
#define RVRandomizeH
// =============================================================================
// ABS - Advanced Batch System
// -----------------------------------------------------------------------------
//    Copyright (C) 2022 Petr Kulhanek, kulhanek@chemi.muni.cz
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
#include <ResourceValue.hpp>
#include <iostream>

// -----------------------------------------------------------------------------

class ABS_PACKAGE CRVRandomize : public CResourceValue {
public:
// constructor -----------------------------------------------------------------
        CRVRandomize(void);

// information methods ---------------------------------------------------------
    /// test value if it is in expected range
    virtual void PreTestValue(CResourceList* p_rl,std::ostream& sout,bool& rstatus);

    /// get variable
    virtual void GetVariable(CSmallString& name, CSmallString& value);
};

// -----------------------------------------------------------------------------

#endif
