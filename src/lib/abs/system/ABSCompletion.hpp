#ifndef CompletionH
#define CompletionH
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

#include <ABSMainHeader.hpp>
#include <XMLDocument.hpp>
#include <SmallString.hpp>
#include <vector>

//------------------------------------------------------------------------------

class ABS_PACKAGE CABSCompletion {
public:
    // constructor and destructors ------------------------------------------------
    CABSCompletion(void);

    // input methods --------------------------------------------------------------
    //! load completion configuration file and command line
    bool InitCompletion(void);

    // executive methods ----------------------------------------------------------
    //! return suggestions for completion
    bool GetSuggestions(void);

    // section of private date ----------------------------------------------------
private:
    CSmallString                CommandLine;
    unsigned int                CGenPosition;
    std::vector<CSmallString>   Words;              // command line splitted into words
    unsigned int                CWord;              // word to be completed
    std::vector<CSmallString>   Suggestions;        // suggestions

    // add initial suggestions (for actions)
    bool AddSuggestions(const CSmallString& list);

    // return command name or empty string
    CSmallString GetCommand(void);

    // return action name or empty string
    CSmallString GetAction(void);

    // add queue suggestions
    bool AddQueueSuggestions(void);

    // add alias suggestions
    bool AddAliasSuggestions(void);

    // add job script suggestions
    bool AddJobScriptSuggestions(void);

    // list collection files
    bool AddCollectionSuggestions(void);

    // does command line contain any option?
    bool AnyOption(void);

    // filter suggestions
    bool FilterSuggestions(void);

    // print suggestions
    bool PrintSuggestions(void);
};

//------------------------------------------------------------------------------

extern  CABSCompletion Completion;

//------------------------------------------------------------------------------

#endif
