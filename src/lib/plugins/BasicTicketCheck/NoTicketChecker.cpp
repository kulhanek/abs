// =============================================================================
//  ABS - Advanced Batch System
// -----------------------------------------------------------------------------
//    Copyright (C) 2015 Petr Kulhanek (kulhanek@chemi.muni.cz)
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

#include "NoTicketChecker.hpp"
#include "BasicTicketCheckModule.hpp"
#include <CategoryUUID.hpp>
#include <FileName.hpp>
#include <fstream>
#include <Shell.hpp>
#include <TorqueConfig.hpp>

//------------------------------------------------------------------------------

using namespace std;

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

CComObject* NoTicketCheckerCB(void* p_data);

CExtUUID        NoTicketCheckerID(
                    "{NO_TICKET_CHECKER:f77bb2f1-bda5-4bb1-958b-4534070baff0}",
                    "Skip ticket validation.");

CPluginObject   NoTicketCheckerObject(&BasicTicketCheckPlugin,
                    NoTicketCheckerID,TICKET_CHECKER_CAT,
                    NoTicketCheckerCB);

// -----------------------------------------------------------------------------

CComObject* NoTicketCheckerCB(void* p_data)
{
    CComObject* p_object = new CNoTicketChecker();
    return(p_object);
}

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

CNoTicketChecker::CNoTicketChecker(void)
    : CTicketChecker(&NoTicketCheckerObject)
{
}

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

bool CNoTicketChecker::IsTicketValid(std::ostream& sout)
{
    return(true);
}

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

