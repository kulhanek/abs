// =============================================================================
// ABS - Advanced Batch System
// -----------------------------------------------------------------------------
//    Copyright (C) 2012      Petr Kulhanek, kulhanek@chemi.muni.cz
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

#include "UpdateInfoFile.hpp"
#include <ErrorSystem.hpp>
#include <TerminalStr.hpp>
#include <JobList.hpp>
#include <SiteController.hpp>
#include <AMSRegistry.hpp>

//------------------------------------------------------------------------------

MAIN_ENTRY(CUpdateInfoFile)

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

CUpdateInfoFile::CUpdateInfoFile(void)
{
}

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

int CUpdateInfoFile::Init(int argc,char* argv[])
{
// encode program options
    int result = Options.ParseCmdLine(argc,argv);

// attach verbose stream to terminal stream and set desired verbosity level
    vout.Attach(Console);
    if( Options.GetOptVerbose() ) {
        vout.Verbosity(CVerboseStr::high);
    } else {
        vout.Verbosity(CVerboseStr::medium);
    }

    return(result);
}

//------------------------------------------------------------------------------

bool CUpdateInfoFile::Run(void)
{
    SiteController.InitSiteControllerConfig();

// init AMS registry
    AMSRegistry.LoadRegistry(vout);

    if( Options.GetArgAction() == "archive" ){
        return( ArchiveRuntimeFiles() );
    }
    if( Options.GetArgAction() == "clean" ){
        return( CleanRuntimeFiles() );
    }

    CSmallString info_file_name;

    if( Job.GetJobInfoFileName(info_file_name) == false ){
        ES_ERROR("unable to get the name of info file");
        return(false);
    }

    if( Job.LoadInfoFile(info_file_name) == false ){
        ES_ERROR("unable to load info file");
        return(false);
    }

 // actions --------------------------------------
    if( Options.GetArgAction() == "start" ){
        if( Job.WriteStart() == false ){
            ES_ERROR("unable to write start section");
            return(false);
        }
    } else if( Options.GetArgAction() == "cliready" ){
        Job.WriteCLITerminalReady(Options.GetOptAgent());
    } else if( Options.GetArgAction() == "guiready" ){
        Job.WriteGUITerminalReady(Options.GetOptAgent(),Options.GetOptVNCID());
    }
    else if( Options.GetArgAction() == "stop" ){
        if( Job.WriteStop() == false ){
            ES_ERROR("unable to write stop section");
            return(false);
        }
    }
    else if( Options.GetArgAction() == "incstage" ){
        Job.IncrementRecycleStage();

    }
    else{
        ES_ERROR("not implemented");
        return(false);
    }

    // save info file -------------------------------
    if( Job.SaveInfoFile(info_file_name) == false ){
        ES_ERROR("unable to save info file");
        return(false);
    }

    return(true);
}

//------------------------------------------------------------------------------

bool CUpdateInfoFile::ArchiveRuntimeFiles(void)
{
    CJobList jobs;

    // get all jobs in job input directory
    jobs.InitByInfoFiles(".",false);

    // keep finished or terminated jobs
    jobs.KeepForClean();

    // sort by prepare date
    jobs.SortByPrepareDateAndTime();

    CTerminalStr    console;
    CVerboseStr     vout;
    vout.Attach(console);

    vout << std::endl;
    if( jobs.GetNumberOfJobs() == 0 ){
        vout << "No job runtime files to archive!" << std::endl;
        return(true);
    }

    vout << "Jobs for archiving (format: " << Options.GetOptFormat() << "):" << std::endl;
    jobs.PrintInfosCompact(vout,false,false);

    // archive
    jobs.ArchiveRuntimeFiles(Options.GetOptFormat());

    return(true);
}

//------------------------------------------------------------------------------

bool CUpdateInfoFile::CleanRuntimeFiles(void)
{
    CJobList jobs;

    // get all jobs in job input directory
    jobs.InitByInfoFiles(".",false);

    // keep finished or terminated jobs
    jobs.KeepForClean();

    // sort by prepare date
    jobs.SortByPrepareDateAndTime();

    CTerminalStr    console;
    CVerboseStr     vout;
    vout.Attach(console);

    if( jobs.GetNumberOfJobs() == 0 ){
        vout << "No job runtime files to clean ..." << std::endl;
        return(true);
    }

    vout << "Jobs for cleaning ..." << std::endl;
    jobs.PrintInfosCompact(vout,false,false);

    // clean
    jobs.CleanRuntimeFiles();

    return(true);
}

//------------------------------------------------------------------------------

void CUpdateInfoFile::Finalize(void)
{
    if( Options.GetOptVerbose() || ErrorSystem.IsError() ) {
        ErrorSystem.PrintErrors(stderr);
    }
}

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

