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

#include "KerberosTicketChecker.hpp"
#include "BasicTicketCheckModule.hpp"
#include <CategoryUUID.hpp>
#include <ErrorSystem.hpp>
#include <User.hpp>
#include <ABSConfig.hpp>
#include <string>
#include <vector>
#include <boost/algorithm/string/split.hpp>
#include <boost/algorithm/string/classification.hpp>

//------------------------------------------------------------------------------

using namespace std;
using namespace boost;

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

CComObject* KerberosTicketCheckerCB(void* p_data);

CExtUUID        KerberosTicketCheckerID(
                    "{KERBEROS_TICKET_CHECKER:5e84dfcb-e7fa-49ad-96a1-1cb5e265b4ff}",
                    "Check Kerberos ticket");

CPluginObject   KerberosTicketCheckerObject(&BasicTicketCheckPlugin,
                    KerberosTicketCheckerID,TICKET_CHECKER_CAT,
                    KerberosTicketCheckerCB);

// -----------------------------------------------------------------------------

CComObject* KerberosTicketCheckerCB(void* p_data)
{
    CComObject* p_object = new CKerberosTicketChecker();
    return(p_object);
}

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

CKerberosTicketChecker::CKerberosTicketChecker(void)
    : CTicketChecker(&KerberosTicketCheckerObject)
{
}

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

bool CKerberosTicketChecker::IsTicketValid(std::ostream& sout)
{
    // run klist command and check if tickets are valid
    FILE* p_stdout = popen("klist -s","r");
    if( p_stdout == NULL ){
        ES_ERROR("unable to popen klist");
        return(false);
    }

    // close the stream
    int retcode = pclose(p_stdout);
    if( retcode != 0 ){
        sout << endl;
        sout <<  "<b><red> ERROR: Kerberos tickets are expired!</red></b>" << endl;
        sout <<          "        Obtain new tickets by the <b>kinit</b> command." << endl;
        sout << endl;
        sout << "<b><blue> HELP:  " << ABSConfig.GetDocURL("kinit") << "</blue></b>" << endl;        return(false);
    }

    // check the principal name
    bool principal_ok = false;
    p_stdout = popen("klist","r");
        if( p_stdout == NULL ){
            ES_ERROR("unable to popen klist");
            return(false);
        }

    CSmallString line1;
    line1.ReadLineFromFile(p_stdout);

    CSmallString line2;
    line2.ReadLineFromFile(p_stdout);

    vector<string>  keys;
    string          sline2(line2);
    split(keys,sline2,is_any_of(" \t\n"),token_compress_on);

    string principal;
    if( keys.size() > 0 ){
        // get the last key on the second line
        principal = keys[keys.size()-1];
        vector<string> items;
        split(items,principal,is_any_of("@"));
        if( items.size() >= 1 ){
            if( User.GetName() == CSmallString(items[0]) ) principal_ok = true;
        }
    }

    pclose(p_stdout);

    if( principal_ok == false ){
        sout << endl;
        sout <<  "<b><red> ERROR: The Kerberos principal '" << principal << "' does not match with the logged user!</red></b>" << endl;
        sout <<          "        Obtain the correct Kerberos tickets by the <b>kinit " << User.GetName() << "</b> command." << endl;
        sout << endl;
        sout << "<b><blue> HELP:  " << ABSConfig.GetDocURL("kinit") << "</blue></b>" << endl;
        return(false);
    }

    return(true);
}

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

