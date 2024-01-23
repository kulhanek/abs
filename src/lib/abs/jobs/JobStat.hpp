#ifndef JobStatH
#define JobStatH
// =============================================================================
//  AMS - Advanced Module System
// -----------------------------------------------------------------------------
//     Copyright (C) 2024 Petr Kulhanek (kulhanek@chemi.muni.cz)
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

#include <AMSMainHeader.hpp>
#include <XMLDocument.hpp>
#include <SmallTimeAndDate.hpp>

//------------------------------------------------------------------------------

struct CJobStatDatagram {
    // constructor and destructors ------------------------------------------------
    CJobStatDatagram(void);

    // set methods ----------------------------------------------------------------
    void Finish(void);

    void SetSite(const CSmallString& site);
    void SetUser(const CSmallString& user);
    void SetHostName(const CSmallString& name);
    void SetHostGroup(const CSmallString& name);
    void SetBatchServer(const CSmallString& name);
    void SetQueue(const CSmallString& name);
    void SetNCPUs(int ncpus);
    void SetCPUMem(int mem);
    void SetNGPUs(int ngpus);
    void SetGPUMem(int mem);
    void SetNumOfNodes(int nnodes);
    void SetWallTime(int wt);
    void SetFlags(int flags);
    void SetTimeAndDate(const CSmallTimeAndDate& dt);

    // get methods ----------------------------------------------------------------
    bool IsValid(void);

    const CSmallString      GetSite(void) const;
    const CSmallString      GetUser(void) const;
    const CSmallString      GetHostName(void) const;
    const CSmallString      GetHostGroup(void) const;
    const CSmallString      GetBatchServer(void) const;
    const CSmallString      GetQueue(void) const;
    int                     GetNCPUs(void) const;
    int                     GetCPUMem(void) const;
    int                     GetNGPUs(void) const;
    int                     GetGPUMem(void) const;
    int                     GetNumOfNodes(void) const;
    int                     GetWallTime(void) const;
    int                     GetFlags(void) const;
    const CSmallTimeAndDate GetTimeAndDate(void) const;

    // section of private data ----------------------------------------------------
private:
    char            Magic[4];               // magic word
    unsigned char   Control[4];             // control sum
    char            Site[64];              // site name
    char            User[64];              // user name
    char            HostName[64];           // hostname
    char            HostGroup[64];           // host group
    char            BatchServer[64];           // batch server
    char            Queue[64];           // queue
    unsigned char   NCPUs[4];               // ncpus
    unsigned char   CPUMem[4];       // ncpus on node
    unsigned char   NGPUs[4];               // ngpus
    unsigned char   GPUMem[4];       // ngpus on node
    unsigned char   NumOfNodes[4];          // number of nodes
    unsigned char   WallTime[4];                // time in seconds from 00:00:00 UTC, January 1, 1970
    unsigned char   Flags[4];               // 32 bit flags
    unsigned char   Time[4];                // time in seconds from 00:00:00 UTC, January 1, 1970
};

//------------------------------------------------------------------------------

#endif
