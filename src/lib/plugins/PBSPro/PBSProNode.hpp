#ifndef PBSProNodeH
#define PBSProNodeH
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
#include <Node.hpp>

// -----------------------------------------------------------------------------

/// computational node

class ABS_PACKAGE CPBSProNode : public CNode {
public:
// constructor -----------------------------------------------------------------
        CPBSProNode(void);

// methods ---------------------------------------------------------------------
    /// init node with batch system information
    bool Init(const CSmallString& srv_name,const CSmallString& short_srv_name,struct batch_status* p_node);
};

// -----------------------------------------------------------------------------

#endif
