// =============================================================================
//  ABS - Advanced Batch System
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

#include <ABSCompletion.hpp>
#include <ctype.h>
#include <ErrorSystem.hpp>
#include <XMLParser.hpp>
#include <XMLElement.hpp>
#include <XMLIterator.hpp>
#include <Shell.hpp>
#include <fnmatch.h>
#include <DirectoryEnum.hpp>
#include <string.h>
#include <ABSConfig.hpp>
#include <User.hpp>
#include <QueueList.hpp>
#include <FileSystem.hpp>
#include <AliasList.hpp>
#include <BatchServers.hpp>
#include <PluginDatabase.hpp>
#include <CategoryUUID.hpp>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <boost/algorithm/string/split.hpp>
#include <boost/algorithm/string/classification.hpp>

//------------------------------------------------------------------------------

using namespace std;
using namespace boost;

//------------------------------------------------------------------------------

CABSCompletion Completion;

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

CABSCompletion::CABSCompletion(void)
{
    CGenPosition = 0;
    CWord = 0;
}

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

bool CABSCompletion::InitCompletion(void)
{
    // get completion data --------------------------
    CommandLine = CShell::GetSystemVariable("COMP_LINE");
    CSmallString tmp;
    tmp = CShell::GetSystemVariable("COMP_POINT");
    CGenPosition = tmp.ToInt();

    // extract all words from command line ----------
    tmp = CommandLine;
    char* p_saveptr = NULL;
    char* p_beg = tmp.GetBuffer();
    char* p_word;

    CWord = 0; // command cannot be completed by this program
    p_word = strtok_r(p_beg," ",&p_saveptr);
    while(p_word != NULL) {
        if( (strlen(p_word) >= 1) && (p_word[0] == '-') ){
            // option - ignore
        } else {
            Words.push_back(p_word);
            unsigned int pos = p_word - p_beg;
            if( (pos <= CGenPosition) &&
                    ((pos + strlen(p_word)) >= CGenPosition) ) CWord = Words.size() - 1;
            p_word = strtok_r(NULL," ",&p_saveptr);
        }
    }
    if( CWord == 0 ) CWord = Words.size();

    // get command
    if( Words.size() >= 1 ){
        Command = Words[0];
    }

    // get action
    if( Words.size() >= 2 ){
        Action = Words[1];
    }

    return(true);
}

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

bool CABSCompletion::GetSuggestions(void)
{
    // get suggestions according to command ---------
    if( GetCommand() == "psubmit" ) {
        if( CWord == 1 ){
            AddQueueSuggestions();
            AddAliasSuggestions();
            FilterSuggestions();
            PrintSuggestions();
            return(true);
        }
        if( CWord == 2 ){
            AddJobScriptSuggestions();
            FilterSuggestions();
            PrintSuggestions();
            return(true);
        }
        if( CWord >= 3 ){
            AddResourceSuggestions();
            FilterSuggestions();
            PrintSuggestions();
            return(true);
        }
        // nothing to do
        return(true);
    // ----------------------------------------------
    } else if( GetCommand() == "pcollection" ) {
        // what part should be completed?
        switch(CWord) {
        case 1:
            AddCollectionSuggestions();
            FilterSuggestions();
            PrintSuggestions();
            return(true);
        case 2:
            AddSuggestions("create open close info submit stat prepare rmjob addjob");
            FilterSuggestions();
            PrintSuggestions();
            return(true);
        default:
            // out of command requirements -> no suggestions
            return(true);
        }
    }
    // ----------------------------------------------
    else {
        // unsupported command return
        return(false);
    }
}

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

bool CABSCompletion::AddSuggestions(const CSmallString& list)
{
    CSmallString tmp = list;
    char* p_saveptr = NULL;
    char* p_word;

    p_word = strtok_r(tmp.GetBuffer()," ",&p_saveptr);
    while(p_word != NULL) {
        Suggestions.push_back(p_word);
        p_word = strtok_r(NULL," ",&p_saveptr);
    }

    return(true);
}

//------------------------------------------------------------------------------

CSmallString CABSCompletion::GetCommand(void)
{
    return(Command);
}

//------------------------------------------------------------------------------

CSmallString CABSCompletion::GetAction(void)
{
    return(Action);
}

//------------------------------------------------------------------------------

bool CABSCompletion::AddQueueSuggestions(void)
{
    // is cache valid?
    CSmallString cname = ABSConfig.GetUserSiteConfigDir() / "queues";
    struct stat cstat;
    if( stat(cname,&cstat) != 0 ){
        InitQueuesCache(cname);
    } else {
        time_t ct = time(NULL);
        double dt = difftime(ct,cstat.st_mtim.tv_sec);
        if( dt > 86400 ){
            InitQueuesCache(cname); // expired cache
        }
    }

    // open cache
    ifstream ifs;
    ifs.open(cname);
    if( ! ifs ){
        CSmallString error;
        error << "unable to open cache file '" << cname << "'";
        ES_TRACE_ERROR(error);
        return(false);
    }

    unsigned int qp = WhatQueuePart();

    // load cache by lines
    std::string line;
    while( getline(ifs,line) ){
        vector<string> items;
        split(items,line,is_any_of("@"));
        if( items.size() == 1 ){
            Suggestions.push_back(line);
        } else if ( items.size() == 2 ) {
            MultipleServers = true;
            switch(qp){
                case 0:
                    Suggestions.push_back(items[0]);
                break;
                case 1:
                    Suggestions.push_back(line);
                break;
            }
        }
    }

    return(true);
}

//------------------------------------------------------------------------------

bool CABSCompletion::InitQueuesCache(const CSmallString& cname)
{
    CFileSystem::CreateDir(ABSConfig.GetUserSiteConfigDir());

    ofstream ofs;
    ofs.open(cname);
    if( ! ofs ){
        CSmallString error;
        error << "unable to open cache file '" << cname << "'";
        ES_TRACE_ERROR(error);
        return(false);
    }

    // init all subsystems
    if( ABSConfig.LoadSystemConfig() == false ){
        ES_TRACE_ERROR("unable to load ABSConfig config");
        return(false);
    }

    // user must be initializaed before ABSConfig.IsUserTicketValid()
    User.InitUser();

    // check if user has valid ticket
    std::stringstream vout;
    if( ABSConfig.IsUserTicketValid(vout) == false ){
        ES_TRACE_ERROR("user does not have valid ticket");
        return(false);
    }

    BatchServers.GetQueues();
    QueueList.RemoveDisabledQueues();
    QueueList.RemoveStoppedQueues();
    QueueList.RemoveInaccesibleQueues(User);
    QueueList.RemoveNonexecutiveQueues();
    QueueList.RemoveOnlyRoutable();

    QueueList.SortByName();

    std::list<CQueuePtr>::iterator it = QueueList.begin();
    std::list<CQueuePtr>::iterator ie = QueueList.end();

    while( it != ie ){
        if( BatchServers.GetNumberOfServers() > 1 ){
            ofs << (*it)->GetNameIncludingShortServerName() << endl;
        } else {
            ofs << (*it)->GetName() << endl;
        }
        it++;
    }

    return(true);
}

//------------------------------------------------------------------------------

unsigned int CABSCompletion::WhatQueuePart(void)
{
    unsigned int numsem = 0;

    if( CWord < Words.size() ) {
        for(unsigned int i=0; i < Words[CWord].GetLength(); i++) {
            if( Words[CWord][i] == '@' ) numsem++;
        }
    }

    return(numsem);
}

//------------------------------------------------------------------------------

bool CABSCompletion::AddAliasSuggestions(void)
{
    if( AliasList.LoadConfig() == false ){
        ES_TRACE_ERROR("unable to load aliases");
        return(false);
    }

    AliasList.GetAliasesSuggestions(Suggestions);

    return(true);
}

//------------------------------------------------------------------------------

bool CABSCompletion::AddJobScriptSuggestions(void)
{
    // list all files in the current directory
    CDirectoryEnum dir_enum(".");

    dir_enum.StartFindFile("*");

    CFileName job_name;

    while( dir_enum.FindFile(job_name) ){
        if( CFileSystem::IsDirectory(job_name) ) continue;
        if( job_name.GetLength() > 0 ){
            // do not show hidden files
            if( job_name[0] == '.' ) continue;
        }
        Suggestions.push_back(job_name);
    }
    dir_enum.EndFindFile();

    return(true);
}

//------------------------------------------------------------------------------

bool CABSCompletion::AddResourceSuggestions(void)
{
    // init all subsystems
    if( ABSConfig.LoadSystemConfig() == false ){
        ES_TRACE_ERROR("unable to load ABSConfig config");
        return(false);
    }

    CSimpleIteratorC<CPluginObject> I(PluginDatabase.GetObjectList());
    CPluginObject* p_pobj;
    while( (p_pobj = I.Current()) ){
        if( p_pobj->GetCategoryUUID() == RESOURCES_CAT ){
            CSmallString res_name = p_pobj->GetObjectUUID().GetName();
            // FIXME
            if( res_name != "pbsprogeneric" ){
                Suggestions.push_back(res_name);
            }
            /*
            CComObject* p_obj = p_pobj->CreateObject(NULL);
            CResourceValue* p_res = dynamic_cast<CResourceValue*>(p_obj);
            */
        }
        I++;
    }
    return(true);
}

//------------------------------------------------------------------------------

bool CABSCompletion::AddCollectionSuggestions(void)
{
    // list all files in the current directory
    CDirectoryEnum dir_enum(".");

    dir_enum.StartFindFile("*.cofi");

    CFileName job_name;

    while( dir_enum.FindFile(job_name) ){
        if( CFileSystem::IsDirectory(job_name) ) continue;
        Suggestions.push_back(job_name.GetFileNameWithoutExt());
    }
    dir_enum.EndFindFile();

    return(true);
}

//------------------------------------------------------------------------------

bool CABSCompletion::FilterSuggestions(void)
{
    // build filter ---------------------------------
    CSmallString filter;

    if( CWord < Words.size() ) {
        filter = Words[CWord] + "*";
    } else {
        filter = "*";
    }

    // filter suggestions ---------------------------
    std::list<CSmallString>::iterator it = Suggestions.begin();
    std::list<CSmallString>::iterator ie = Suggestions.end();
    while( it != ie ) {
        CSmallString sg = *it;
        if( fnmatch(filter,sg,0) != 0 ) {
            // does not match - remove suggestion
            it = Suggestions.erase(it);
        } else {
            it++;
        }
    }

//    it = Suggestions.begin();

//    // keep only the last word after "@"
//    while( it != ie ) {
//        CSmallString tmp = *it;
//        char* p_saveptr = NULL;
//        char* p_word;

//        p_word = strtok_r(tmp.GetBuffer(),"@",&p_saveptr);
//        while(p_word != NULL) {
//            *it = p_word;
//            p_word = strtok_r(NULL,"@",&p_saveptr);
//        }
//        it++;
//    }

    return(true);
}

//------------------------------------------------------------------------------

bool CABSCompletion::PrintSuggestions(void)
{
    std::list<CSmallString>::iterator it = Suggestions.begin();
    std::list<CSmallString>::iterator ie = Suggestions.end();
    while( it != ie ) {
        if( CWord == 1 ) {
            unsigned int qp = WhatQueuePart();
            if( (Suggestions.size() == 1) && (qp == 0) && MultipleServers ){
                // print only suggestion and move to the next part of queue
                cout << *it << "@" << endl;
            } else if (Suggestions.size() == 1) {
                // print only suggestion and move to the next argument
                cout << *it << " " << endl;
            } else {
                cout << *it << endl;
            }
        } else if( (Suggestions.size() == 1) && (CWord < 3) ) {
            // print only suggestion and move to the next argument
            cout << *it << " " << endl;
        } else if( (Suggestions.size() == 1) && (CWord >= 3) ) {
            // print only suggestion and move to the resource value
            cout << *it << "=" << endl;
        } else {
            // print only suggestion
            cout << *it << endl;
        }
        it++;
    }

    return(true);
}

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

