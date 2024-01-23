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

#include <JobStat.hpp>
#include <ErrorSystem.hpp>
#include <string.h>

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

CJobStatDatagram::CJobStatDatagram(void)
{
    memset(Magic,0,sizeof(Magic));
    memset(Control,0,sizeof(Control));
    memset(Site,0,sizeof(Site));
    memset(User,0,sizeof(User));
    memset(HostName,0,sizeof(HostName));
    memset(HostGroup,0,sizeof(HostGroup));
    memset(BatchServer,0,sizeof(BatchServer));
    memset(Queue,0,sizeof(Queue));
    memset(NCPUs,0,sizeof(NCPUs));
    memset(CPUMem,0,sizeof(CPUMem));
    memset(NGPUs,0,sizeof(NGPUs));
    memset(GPUMem,0,sizeof(GPUMem));
    memset(NumOfNodes,0,sizeof(NumOfNodes));
    memset(WallTime,0,sizeof(WallTime));
    memset(Flags,0,sizeof(Flags));
    memset(Time,0,sizeof(Time));
}

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

void CJobStatDatagram::Finish(void)
{
    // magic header
    strncpy(Magic,"ABS4",4);

    // controll sum
    int control_sum = 0;
    for(unsigned int i=0; i < sizeof(Magic); i++) control_sum += Magic[i];
    for(unsigned int i=0; i < sizeof(Site); i++) control_sum += Site[i];
    for(unsigned int i=0; i < sizeof(User); i++) control_sum += User[i];
    for(unsigned int i=0; i < sizeof(HostName); i++) control_sum += HostName[i];
    for(unsigned int i=0; i < sizeof(HostGroup); i++) control_sum += HostGroup[i];
    for(unsigned int i=0; i < sizeof(BatchServer); i++) control_sum += BatchServer[i];
    for(unsigned int i=0; i < sizeof(Queue); i++) control_sum += Queue[i];
    for(unsigned int i=0; i < sizeof(NCPUs); i++) control_sum += NCPUs[i];
    for(unsigned int i=0; i < sizeof(CPUMem); i++) control_sum += CPUMem[i];
    for(unsigned int i=0; i < sizeof(NGPUs); i++) control_sum += NGPUs[i];
    for(unsigned int i=0; i < sizeof(GPUMem); i++) control_sum += GPUMem[i];
    for(unsigned int i=0; i < sizeof(NumOfNodes); i++) control_sum += NumOfNodes[i];
    for(unsigned int i=0; i < sizeof(WallTime); i++) control_sum += WallTime[i];
    for(unsigned int i=0; i < sizeof(Flags); i++) control_sum += Flags[i];
    for(unsigned int i=0; i < sizeof(Time); i++) control_sum += Time[i];

    Control[0] = (unsigned char) ((control_sum >> 24) & 0xFF);
    Control[1] = (unsigned char) ((control_sum >> 16) & 0xFF);
    Control[2] = (unsigned char) ((control_sum >>  8) & 0xFF);
    Control[3] = (unsigned char) ((control_sum      ) & 0xFF);
}

//------------------------------------------------------------------------------

void CJobStatDatagram::SetSite(const CSmallString& site)
{
    // -1 for \0 termination character
    strncpy(Site,site,sizeof(Site)-1);
}

//------------------------------------------------------------------------------

void CJobStatDatagram::SetUser(const CSmallString& name)
{
    // -1 for \0 termination character
    strncpy(User,name,sizeof(User)-1);
}


//------------------------------------------------------------------------------

void CJobStatDatagram::SetHostName(const CSmallString& name)
{
    strncpy(HostName,name,sizeof(HostName)-1);
}

//------------------------------------------------------------------------------

void CJobStatDatagram::SetHostGroup(const CSmallString& name)
{
    strncpy(HostGroup,name,sizeof(HostGroup)-1);
}

//------------------------------------------------------------------------------

void CJobStatDatagram::SetBatchServer(const CSmallString& name)
{
    strncpy(BatchServer,name,sizeof(BatchServer)-1);
}

//------------------------------------------------------------------------------

void CJobStatDatagram::SetQueue(const CSmallString& name)
{
    strncpy(Queue,name,sizeof(Queue)-1);
}

//------------------------------------------------------------------------------

void CJobStatDatagram::SetNCPUs(int ncpus)
{
    NCPUs[0] = (unsigned char) ((ncpus >> 24) & 0xFF);
    NCPUs[1] = (unsigned char) ((ncpus >> 16) & 0xFF);
    NCPUs[2] = (unsigned char) ((ncpus >>  8) & 0xFF);
    NCPUs[3] = (unsigned char) ((ncpus      ) & 0xFF);
}

//------------------------------------------------------------------------------

void CJobStatDatagram::SetCPUMem(int mem)
{
    CPUMem[0] = (unsigned char) ((mem >> 24) & 0xFF);
    CPUMem[1] = (unsigned char) ((mem >> 16) & 0xFF);
    CPUMem[2] = (unsigned char) ((mem >>  8) & 0xFF);
    CPUMem[3] = (unsigned char) ((mem      ) & 0xFF);
}

//------------------------------------------------------------------------------

void CJobStatDatagram::SetNGPUs(int ngpus)
{
    NGPUs[0] = (unsigned char) ((ngpus >> 24) & 0xFF);
    NGPUs[1] = (unsigned char) ((ngpus >> 16) & 0xFF);
    NGPUs[2] = (unsigned char) ((ngpus >>  8) & 0xFF);
    NGPUs[3] = (unsigned char) ((ngpus      ) & 0xFF);
}

//------------------------------------------------------------------------------

void CJobStatDatagram::SetGPUMem(int mem)
{
    GPUMem[0] = (unsigned char) ((mem >> 24) & 0xFF);
    GPUMem[1] = (unsigned char) ((mem >> 16) & 0xFF);
    GPUMem[2] = (unsigned char) ((mem >>  8) & 0xFF);
    GPUMem[3] = (unsigned char) ((mem      ) & 0xFF);
}

//------------------------------------------------------------------------------

void CJobStatDatagram::SetNumOfNodes(int nnodes)
{
    NumOfNodes[0] = (unsigned char) ((nnodes >> 24) & 0xFF);
    NumOfNodes[1] = (unsigned char) ((nnodes >> 16) & 0xFF);
    NumOfNodes[2] = (unsigned char) ((nnodes >>  8) & 0xFF);
    NumOfNodes[3] = (unsigned char) ((nnodes      ) & 0xFF);
}

//------------------------------------------------------------------------------

void CJobStatDatagram::SetWallTime(int wt)
{
    WallTime[0] = (unsigned char) ((wt >> 24) & 0xFF);
    WallTime[1] = (unsigned char) ((wt >> 16) & 0xFF);
    WallTime[2] = (unsigned char) ((wt >>  8) & 0xFF);
    WallTime[3] = (unsigned char) ((wt      ) & 0xFF);
}

//------------------------------------------------------------------------------

void CJobStatDatagram::SetFlags(int flags)
{
    Flags[0] = (unsigned char) ((flags >> 24) & 0xFF);
    Flags[1] = (unsigned char) ((flags >> 16) & 0xFF);
    Flags[2] = (unsigned char) ((flags >>  8) & 0xFF);
    Flags[3] = (unsigned char) ((flags      ) & 0xFF);
}

//------------------------------------------------------------------------------

void CJobStatDatagram::SetTimeAndDate(const CSmallTimeAndDate& dt)
{
    int seconds = dt.GetSecondsFromBeginning();

    Time[0] = (unsigned char) ((seconds >> 24) & 0xFF);
    Time[1] = (unsigned char) ((seconds >> 16) & 0xFF);
    Time[2] = (unsigned char) ((seconds >>  8) & 0xFF);
    Time[3] = (unsigned char) ((seconds      ) & 0xFF);
}

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

bool CJobStatDatagram::IsValid(void)
{
    // magic header
    if( strncmp(Magic,"ABS4",4) != 0 ) {
        return(false);
    }

    // controll sum
    int control_sum = 0;
    for(unsigned int i=0; i < sizeof(Magic); i++) control_sum += Magic[i];
    for(unsigned int i=0; i < sizeof(Site); i++) control_sum += Site[i];
    for(unsigned int i=0; i < sizeof(User); i++) control_sum += User[i];
    for(unsigned int i=0; i < sizeof(HostName); i++) control_sum += HostName[i];
    for(unsigned int i=0; i < sizeof(HostGroup); i++) control_sum += HostGroup[i];
    for(unsigned int i=0; i < sizeof(BatchServer); i++) control_sum += BatchServer[i];
    for(unsigned int i=0; i < sizeof(Queue); i++) control_sum += Queue[i];
    for(unsigned int i=0; i < sizeof(NCPUs); i++) control_sum += NCPUs[i];
    for(unsigned int i=0; i < sizeof(CPUMem); i++) control_sum += CPUMem[i];
    for(unsigned int i=0; i < sizeof(NGPUs); i++) control_sum += NGPUs[i];
    for(unsigned int i=0; i < sizeof(GPUMem); i++) control_sum += GPUMem[i];
    for(unsigned int i=0; i < sizeof(NumOfNodes); i++) control_sum += NumOfNodes[i];
    for(unsigned int i=0; i < sizeof(WallTime); i++) control_sum += WallTime[i];
    for(unsigned int i=0; i < sizeof(Flags); i++) control_sum += Flags[i];
    for(unsigned int i=0; i < sizeof(Time); i++) control_sum += Time[i];

    int rec_control_sum;
    rec_control_sum = (Control[0] << 24) + (Control[1] << 16)
                      + (Control[2] << 8) + Control[3];

    if( rec_control_sum != control_sum ) {
        CSmallString error;
        error << "sum: " << CSmallString(rec_control_sum) << " rec: " << CSmallString(control_sum);
        ES_ERROR(error);
        return(false);
    }

    return(true);
}

//------------------------------------------------------------------------------

const CSmallString CJobStatDatagram::GetSite(void) const
{
    return(Site);
}

//------------------------------------------------------------------------------

const CSmallString CJobStatDatagram::GetUser(void) const
{
    return(User);
}

//------------------------------------------------------------------------------

const CSmallString CJobStatDatagram::GetHostName(void) const
{
    return(HostName);
}

//------------------------------------------------------------------------------

const CSmallString CJobStatDatagram::GetHostGroup(void) const
{
    return(HostGroup);
}

//------------------------------------------------------------------------------

const CSmallString CJobStatDatagram::GetBatchServer(void) const
{
    return(BatchServer);
}

//------------------------------------------------------------------------------

const CSmallString CJobStatDatagram::GetQueue(void) const
{
    return(Queue);
}

//------------------------------------------------------------------------------

int CJobStatDatagram::GetNCPUs(void) const
{
    return( (NCPUs[0] << 24) + (NCPUs[1] << 16) + (NCPUs[2] << 8) + NCPUs[3] );
}

//------------------------------------------------------------------------------

int CJobStatDatagram::GetCPUMem(void) const
{
    return( (CPUMem[0] << 24) + (CPUMem[1] << 16)
            + (CPUMem[2] << 8) + CPUMem[3] );
}

//------------------------------------------------------------------------------

int CJobStatDatagram::GetNGPUs(void) const
{
    return( (NGPUs[0] << 24) + (NGPUs[1] << 16) + (NGPUs[2] << 8) + NGPUs[3] );
}

//------------------------------------------------------------------------------

int CJobStatDatagram::GetGPUMem(void) const
{
    return( (GPUMem[0] << 24) + (GPUMem[1] << 16)
            + (GPUMem[2] << 8) + GPUMem[3] );
}

//------------------------------------------------------------------------------

int CJobStatDatagram::GetNumOfNodes(void) const
{
    return( (NumOfNodes[0] << 24) + (NumOfNodes[1] << 16)
          + (NumOfNodes[2] << 8) + NumOfNodes[3] );
}

//------------------------------------------------------------------------------

int CJobStatDatagram::GetWallTime(void) const
{
    return( (WallTime[0] << 24) + (WallTime[1] << 16)
          + (WallTime[2] << 8) + WallTime[3] );
}

//------------------------------------------------------------------------------

int CJobStatDatagram::GetFlags(void) const
{
    return( (Flags[0] << 24) + (Flags[1] << 16) + (Flags[2] << 8) + Flags[3] );
}

//------------------------------------------------------------------------------

const CSmallTimeAndDate CJobStatDatagram::GetTimeAndDate(void) const
{
    int seconds_from_beginning = (Time[0] << 24) + (Time[1] << 16) + (Time[2] << 8) + Time[3];

    CSmallTimeAndDate dt(seconds_from_beginning);
    return(dt);
}

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================
