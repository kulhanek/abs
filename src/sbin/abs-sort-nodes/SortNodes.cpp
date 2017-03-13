// =============================================================================
// ABS - Advanced Batch System
// -----------------------------------------------------------------------------
//    Copyright (C) 2013      Petr Kulhanek, kulhanek@chemi.muni.cz
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

#include "SortNodes.hpp"
#include <ErrorSystem.hpp>
#include <TerminalStr.hpp>
#include <fstream>

//------------------------------------------------------------------------------

MAIN_ENTRY(CSortNodes)

using namespace std;

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

CSortNodes::CSortNodes(void)
{
}

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

int CSortNodes::Init(int argc,char* argv[])
{
    // encode program options
    int result = Options.ParseCmdLine(argc,argv);
    return(result);
}

//------------------------------------------------------------------------------

bool CompareNamesB(const string& s1,const string& s2)
{
    string n1 = s1;
    string n2 = s2;
    int    i1 = 0;
    int    i2 = 0;

    std::size_t found = s1.find_first_of("0123456789");
    if( (found != string::npos) && (found > 0) ){
        n1 = s1.substr(0,found);
        stringstream str(s1.substr(found));
        str >> i1;
    }

    found = s2.find_first_of("0123456789");
    if( (found != string::npos) && (found > 0) ){
        n2 = s2.substr(0,found);
        stringstream str(s2.substr(found));
        str >> i2;
    }

    if( n1 == n2 ){
        return( i1 <= i2 );
    }

    return( n1 <= n2 );
}

//------------------------------------------------------------------------------

bool CSortNodes::Run(void)
{
    // read node file -------------------------------
    ifstream ifs;

    ifs.open(Options.GetArgNodeFile());
    if( ! ifs ){
        CSmallString error;
        error << "unable to open node file " << Options.GetArgNodeFile();
        ES_ERROR(error);
    }

    string line;
    while( getline(ifs,line) ){
        Nodes.push_back(line);
    }
    ifs.close();

    // print main node
    list<string>::iterator it = Nodes.begin();
    list<string>::iterator ie = Nodes.end();

    while( it != ie ){
        if( *it == string(Options.GetArgMainNode()) ){
            cout << *it << endl;
            it = Nodes.erase(it);
        } else {
            it++;
        }
    }

    // sort the list
    Nodes.sort(CompareNamesB);

    // print the rest of the list
    it = Nodes.begin();
    while( it != ie ){
        cout << *it << endl;
        it++;
    }

    return(true);
}

//------------------------------------------------------------------------------

void CSortNodes::Finalize(void)
{
    if( Options.GetOptVerbose() || ErrorSystem.IsError() ) {
        ErrorSystem.PrintErrors(stderr);
    }
}

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

