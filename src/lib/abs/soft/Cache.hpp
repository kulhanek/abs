#ifndef CacheH
#define CacheH
// =============================================================================
// ABS - Advanced Batch System
// -----------------------------------------------------------------------------
//     Copyright (C) 2012 Petr Kulhanek (kulhanek@chemi.muni.cz)
//     Copyright (C) 2011 Petr Kulhanek (kulhanek@chemi.muni.cz)
//     Copyright (C) 2004,2005,2008,2010 Petr Kulhanek (kulhanek@chemi.muni.cz)
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

#include <ABSMainHeader.hpp>
#include <SmallString.hpp>
#include <XMLDocument.hpp>
#include <FileName.hpp>
#include <VerboseStr.hpp>
#include <vector>
#include <list>

//-----------------------------------------------------------------------------

/* ======= MODULE BLOCK STRUCTURE ========
<module name=""
        enabled="true/false"
        export="true/false"
        >
    <build ver="" arch="" para="">
        <variable name="" value=""
                  operation="append/preppend/set"
                  priority="normal/modaction"/>
        <script name="" type="child" priority="normal/modaction"/>
        <dependencies>
            <conflict module=""/>
            <depend module=""/>
        </dependencies>
    </build>
    <dependencies>
        <conflict module=""/>
        <depend module=""/>
    </dependencies>
    <default ver="" arch="" para=""/>
</module>
*/

//-----------------------------------------------------------------------------

class ABS_PACKAGE CCache {
public:
// constructor and destructors -------------------------------------------------
    CCache(void);

// input/output methods --------------------------------------------------------
    /// init whole module cache
    bool LoadCache(void);

    /// get root element of cache
    CXMLElement* GetRootElementOfCache(void);

// information methods ---------------------------------------------------------
    /// return module specification for module with name
    CXMLElement* GetModule(const CSmallString& name);

    /// check module version
    bool CheckModuleVersion(CXMLElement* p_module,const CSmallString& ver);

    /// check module version and architecture
    bool CheckModuleArchitecture(CXMLElement* p_module,
                            const CSmallString& ver,
                            const CSmallString& arch);

    /// check module version, architecture, and parallel mode
    bool CheckModuleMode(CXMLElement* p_module,
                            const CSmallString& ver,
                            const CSmallString& arch,
                            const CSmallString& mode);

    /// does it need GPU resources
    bool DoesItNeedGPU(const CSmallString& name);

// section of private data -----------------------------------------------------
private:
    CXMLDocument    Cache;
};

//-----------------------------------------------------------------------------

extern CCache   Cache;

//-----------------------------------------------------------------------------

#endif
