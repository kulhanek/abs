// =============================================================================
// ABS - Advanced Batch System
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

#include <PluginDatabase.hpp>
#include "BasicTicketCheckModule.hpp"

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

bool BasicTicketCheckPluginCB(EModuleRequest request,void* p_data);

// -----------------------------------------------------------------------------

CExtUUID        BasicTicketCheckPluginID(
                    "{BASIC_TICKET_CHECK:992ee2c3-c72e-428b-be40-dd98c67c1ec6}");

CPluginModule   BasicTicketCheckPlugin(
                    BasicTicketCheckPluginID,
                    BasicTicketCheckPluginCB);

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

bool BasicTicketCheckPluginCB(EModuleRequest request,void* p_data)
{
    switch(request) {
        case EMR_INITIALIZE:
            break;
        case EMR_FINALIZE:
            break;
    };

    return(true);
}

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================
