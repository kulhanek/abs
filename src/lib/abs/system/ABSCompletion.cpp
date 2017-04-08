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
        Words.push_back(p_word);
        unsigned int pos = p_word - p_beg;
        if( (pos <= CGenPosition) &&
                ((pos + strlen(p_word)) >= CGenPosition) ) CWord = Words.size() - 1;
        p_word = strtok_r(NULL," ",&p_saveptr);
    }
    if( CWord == 0 ) CWord = Words.size();

    return(true);
}

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

bool CABSCompletion::GetSuggestions(void)
{
    // if any option then do not provide any suggestion
    if( AnyOption() == true ) return(true);

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
            AddSuggestions("create open close info submit stat");
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
    if( Words.size() >= 1 ) return(Words[0]);
    return("");
}

//------------------------------------------------------------------------------

CSmallString CABSCompletion::GetAction(void)
{
    if( Words.size() >= 2 ) return(Words[1]);
    return("");
}

//------------------------------------------------------------------------------

bool CABSCompletion::AddQueueSuggestions(void)
{
    // init all subsystems
    if( ABSConfig.LoadSystemConfig() == false ){
        ES_TRACE_ERROR("unable to load ABSConfig config");
        return(false);
    }

    // check if user has valid ticket
    std::stringstream vout;
    if( ABSConfig.IsUserTicketValid(vout) == false ){
        ES_TRACE_ERROR("user does not have valid ticket");
        return(false);
    }

    User.InitUser();

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
            Suggestions.push_back((*it)->GetNameIncludingShortServerName());
        } else {
            Suggestions.push_back((*it)->GetName());
        }
        it++;
    }

    return(true);
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
    CSimpleIteratorC<CPluginObject> I(PluginDatabase.GetObjectList());
    CPluginObject* p_pobj;
    while( (p_pobj = I.Current()) ){
        if( p_pobj->GetCategoryUUID() == RESOURCES_CAT ){
            CSmallString res_name = p_pobj->GetObjectUUID().GetName();
            Suggestions.push_back(res_name+"=");
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

bool CABSCompletion::AnyOption(void)
{
    for(unsigned int i=0; i < Words.size(); i++) {
        // all words are non-empty (due to usage of strtok)
        if( Words[i][0] == '-' ) return(true);
    }
    return(false);
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
    for(unsigned int i=0; i < Suggestions.size(); i++) {
        if( fnmatch(filter,Suggestions[i],0) != 0 ) {
            // does not match - remove suggestion
            Suggestions[i] = NULL;
        }
    }

    return(true);
}

//------------------------------------------------------------------------------

bool CABSCompletion::PrintSuggestions(void)
{
    // count number of suggestions
    int scount = 0;
    for(unsigned int i=0; i < Suggestions.size(); i++) {
        if( Suggestions[i] != NULL) scount++;
    }

    // print suggestions
    for(unsigned int i=0; i < Suggestions.size(); i++) {
        if( scount == 1 ) {
            if( Suggestions[i] != NULL) printf("%s \n",(const char*)Suggestions[i]);
        } else {
            // print only suggestion
            if( Suggestions[i] != NULL) printf("%s\n",(const char*)Suggestions[i]);
        }
    }
    return(true);
}

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

