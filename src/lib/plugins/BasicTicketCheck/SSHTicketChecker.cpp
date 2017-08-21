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

#include "SSHTicketChecker.hpp"
#include "BasicTicketCheckModule.hpp"
#include <CategoryUUID.hpp>
#include <FileName.hpp>
#include <fstream>
#include <Shell.hpp>
#include <ABSConfig.hpp>

//------------------------------------------------------------------------------

using namespace std;

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

CComObject* SSHTicketCheckerCB(void* p_data);

CExtUUID        SSHTicketCheckerID(
                    "{SSH_TICKET_CHECKER:40b2034b-b2db-446e-8940-3a9281dc296b}",
                    "Check passwordless authentication by SSH keys");

CPluginObject   SSHTicketCheckerObject(&BasicTicketCheckPlugin,
                    SSHTicketCheckerID,TICKET_CHECKER_CAT,
                    SSHTicketCheckerCB);

// -----------------------------------------------------------------------------

CComObject* SSHTicketCheckerCB(void* p_data)
{
    CComObject* p_object = new CSSHTicketChecker();
    return(p_object);
}

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

CSSHTicketChecker::CSSHTicketChecker(void)
    : CTicketChecker(&SSHTicketCheckerObject)
{
}

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

bool CSSHTicketChecker::IsTicketValid(std::ostream& sout)
{
    CSmallString public_key_name = "id_rsa.pub";
    // FIXME - config from plugin database
    // ABSConfig.GetSystemConfigItem(SSHTicketCheckerID,"INF_PUB_KEY_NAME",public_key_name);

    // get full paths
    CFileName authorized_keys_name;
    public_key_name = CFileName(CShell::GetSystemVariable("HOME")) / ".ssh" / public_key_name;
    authorized_keys_name = CFileName(CShell::GetSystemVariable("HOME")) / ".ssh" / "authorized_keys";

    ifstream ifs;

    // open public key and read it
    ifs.open(public_key_name);

    if( ! ifs ){
        sout << endl;        
        sout <<  "<b><red> ERROR: Passwordless SSH athentication does not probably work!</red></b>" << endl;
        sout <<  "<b><red>        Unable to open the public SSH key '" << public_key_name << "'!</red></b>" << endl;
        sout << endl;
        sout << "<b><blue> HELP:  " << ABSConfig.GetDocURL("passwordless") << "</blue></b>" << endl;
        sout << endl;
        return(false);
    }

    string pub_key;
    getline(ifs,pub_key);

    ifs.close();

    // open authorized keys
    ifs.open(authorized_keys_name);

    if( ! ifs ){
        sout << endl;        
        sout <<  "<b><red> ERROR: Passwordless SSH athentication does not probably work!</red></b>" << endl;
        sout <<  "<b><red>        Unable to open the file with authorized keys '" << authorized_keys_name << "'!</red></b>" << endl;
        sout << endl;
        sout << "<b><blue> HELP:  " << ABSConfig.GetDocURL("passwordless") << "</blue></b>" << endl;
        sout << endl;
        return(false);
    }

    string line;
    while( getline(ifs,line) ){
        if( line == pub_key ){
            return(true);
        }
    }

    sout << endl;    
    sout <<  "<b><red> ERROR: Passwordless SSH athentication does not probably work!</red></b>" << endl;
    sout <<  "<b><red>        The public SSH key must be a member of authorized keys!</red></b>" << endl;
    sout << endl;
    sout << "<b><blue> HELP:  " << ABSConfig.GetDocURL("passwordless") << "</blue></b>" << endl;
    sout << endl;

    return(false);
}

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

