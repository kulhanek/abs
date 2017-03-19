#ifndef InfoFileH
#define InfoFileH
// =============================================================================
//  ABS - Advanced Batch System - Uniform Job Management for Scientific Calculations
// -----------------------------------------------------------------------------
//    Copyright (C) 2001-2008 Petr Kulhanek, kulhanek@chemi.muni.cz
//
//     This library is free software; you can redistribute it and/or
//     modify it under the terms of the GNU Lesser General Public
//     License as published by the Free Software Foundation; either
//     version 2.1 of the License, or (at your option) any later version.
//
//     This library is distributed in the hope that it will be useful,
//     but WITHOUT ANY WARRANTY; without even the implied warranty of
//     MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
//     Lesser General Public License for more details.
//
//     You should have received a copy of the GNU Lesser General Public
//     License along with this library; if not, write to the Free Software
//     Foundation, Inc., 51 Franklin Street, Fifth Floor,
//     Boston, MA  02110-1301  USA
// =============================================================================

#include <InfinityMainHeader.h>
#include <XMLDocument.hpp>

// -----------------------------------------------------------------------------

class CXMLElement;

// -----------------------------------------------------------------------------

class INFINITY_PACKAGE CInfoFile {
    public:
 // constructor ----------------------------------------------------------------
        __hipolyspec CInfoFile(void);

 // main methods ---------------------------------------------------------------
    //! load info file
    bool __hipolyspec LoadInfoFile(const CSmallString& name);

    //! save info file
    bool __hipolyspec SaveInfoFile(const CSmallString& name);

    //! was info file modified?
    bool __hipolyspec IsFileModified(void);

    //! job info file
    static bool __hipolyspec GetJobInfoFileName(CSmallString& info_file_name);

 // actions --------------------------------------------------------------------
    bool __hipolyspec WriteHeader(void);
    bool __hipolyspec WriteBasic(void);
    bool __hipolyspec WriteBackgroundSubmit(void);
    bool __hipolyspec WriteSubmit(void);
    bool __hipolyspec WriteStart(void);
    bool __hipolyspec WriteStop(void);
    bool __hipolyspec WriteKill(void);
    bool __hipolyspec WriteAbort(void);

    bool __hipolyspec WriteSpecificPBS(void);
    bool __hipolyspec WriteSpecificGLiteWMS(void);

 // actions --------------------------------------------------------------------
    bool               __hipolyspec SetItem(const CSmallString& path,
                                    const CSmallString& name,
                                    const CSmallString& value,bool verbose=false);
    bool               __hipolyspec PrintItem(const CSmallString& path,
                                    const CSmallString& name);
    const CSmallString __hipolyspec GetItem(const CSmallString& path,
                                    const CSmallString& name);
    const CSmallString __hipolyspec GetDriver(void);
    const CSmallString __hipolyspec GetStamp(void);
    const CSmallString __hipolyspec GetStampName(void);

    bool __hipolyspec IsSection(const CSmallString& path);
    bool __hipolyspec PrintTime(const CSmallString& path);
    bool __hipolyspec PrintWaitTime(const CSmallString& path);
    bool __hipolyspec PrintTimeDiff(const CSmallString& path1,const CSmallString& path2);
    bool __hipolyspec PrintTimeDiffInSeconds(const CSmallString& path1,const CSmallString& path2);
    bool __hipolyspec PrintVer(void);
    bool __hipolyspec PrintMode(void);
    bool __hipolyspec PrintSubMode(void);
    bool __hipolyspec PrintDriver(void);
    bool __hipolyspec PrintStamp(void);
    bool __hipolyspec PrintStampName(void);
    bool __hipolyspec MergeSections(const CSmallString& outsec,
                                    const CSmallString& source,
                                    const CSmallString& insec);
    bool __hipolyspec ListNodes(void);
    bool __hipolyspec RestoreEnvironment(void);

    bool __hipolyspec PrintBasic(void);
    bool __hipolyspec PrintResources(void);
    bool __hipolyspec PrintExec(void);

    bool __hipolyspec PrintCoInfo(const CSmallString& real_status);

 // section of private data ----------------------------------------------------
    private:
    CXMLDocument    InfoFile;
    bool            Modified;

    // helper methods
    CXMLElement* __hipolyspec GetElementByPath(const CSmallString& p_path,bool create);
    CXMLElement* __hipolyspec FindItem(CXMLElement* p_root,const char* p_name);
    bool         __hipolyspec WriteInfo(const CSmallString& category,
                                     const CSmallString& name);
    bool         __hipolyspec CreateSection(const CSmallString& section);
    bool         __hipolyspec RestoreEnvironmentForElement(CXMLElement* p_ele);
    };

// -----------------------------------------------------------------------------

#endif
