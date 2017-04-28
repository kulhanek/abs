// =============================================================================
// ABS - Advanced Batch System
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

#include "Collection.hpp"
#include <ErrorSystem.hpp>
#include <SmallTimeAndDate.hpp>
#include <ABSConfig.hpp>
#include <AMSGlobalConfig.hpp>
#include <ShellProcessor.hpp>

using namespace std;

//------------------------------------------------------------------------------

MAIN_ENTRY(CCollection)

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

CCollection::CCollection(void)
{
}

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

int CCollection::Init(int argc,char* argv[])
{
// encode program options, all check procedures are done inside of CABFIntOpts
    int result = Options.ParseCmdLine(argc,argv);

// should we exit or was it error?
    if(result != SO_CONTINUE) return(result);

// attach verbose stream to terminal stream and set desired verbosity level
    // all output to stderr
    Console.Attach(stderr);

    vout.Attach(Console);
    if( Options.GetOptVerbose() ) {
        vout.Verbosity(CVerboseStr::high);
    } else {
        vout.Verbosity(CVerboseStr::low);
    }

    CSmallTimeAndDate dt;
    dt.GetActualTimeAndDate();

    vout << high;
    vout << endl;
    vout << "# ==============================================================================" << endl;
    vout << "# pcollection (ABS utility) started at " << dt.GetSDateAndTime() << endl;
    vout << "# ==============================================================================" << endl;

    return(SO_CONTINUE);
}

//------------------------------------------------------------------------------

bool CCollection::Run(void)
{
    // init all subsystems
    if( ABSConfig.LoadSystemConfig() == false ){
        ES_ERROR("unable to load ABSConfig config");
        return(false);
    }

    vout << low;

    // user must be initializaed before ABSConfig.IsUserTicketValid()
    User.InitUser();

    // check if user has valid ticket
    if( ABSConfig.IsUserTicketValid(vout) == false ){
        ES_TRACE_ERROR("user does not have valid ticket");
        return(false);
    }

    // if no arguments try to print info about collection
    if( Options.GetNumberOfProgArgs() == 0 ){
        CSmallString collname;
        if( Jobs.TryToFindCollection(collname) == false ){
            vout << endl;
            vout << "<b><red> ERROR: No or more than one collection is present in the current directory!</red></b>" << endl;
            return(false);
        }
        if( Jobs.LoadCollection(collname) == false ){
            vout << endl;
            vout << "<b><red> ERROR: Unable to load collection '" << collname << "'!</red></b>" << endl;
            return(false);
        }
        // update status of live jobs
        Jobs.UpdateJobStatuses();

        // print info about collection
        Jobs.PrintCollectionInfo(vout,Options.GetOptIncludePath(),Options.GetOptIncludeComment());
        return(true);
    }

    if( Options.GetNumberOfProgArgs() == 1 ){
        vout << endl;
        vout << "<b><red> ERROR: Incorrect number of arguments!</red></b>" << endl;
        return(false);
    }

// -------------------------------------------------------------------
// create
// -------------------------------------------------------------------
    if( Options.GetProgArg(1) == "create" ){
        if( Jobs.CreateCollection(Options.GetProgArg(0),vout) == false ){
            ES_TRACE_ERROR("unable to create collection");
            return(false);
        }

        // print info
        Jobs.PrintCollectionInfo(vout,Options.GetOptIncludePath(),Options.GetOptIncludeComment());

        // save collection
        if( Jobs.SaveCollection() == false ){
            vout << endl;
            vout << "<b><red> ERROR: Unable to save collection '" << Jobs.GetCollectionName() << "'!</red></b>" << endl;
            return(false);
        }
        return(true);
// -------------------------------------------------------------------
// open
// -------------------------------------------------------------------
    } else if( Options.GetProgArg(1) == "open" ) {
        bool created = false;
        if( Jobs.LoadCollection(Options.GetProgArg(0)) == false ){
            vout << endl;
            vout << "<blue> INFO: The collection does not exist, creating new one ...</blue>" << endl;
            if( Jobs.CreateCollection(Options.GetProgArg(0),vout) == false ){
                ES_TRACE_ERROR("unable to create collection");
                return(false);
            }
            created = true;
        }
        // update status of live jobs
        Jobs.UpdateJobStatuses();

        // print info
        Jobs.PrintCollectionInfo(vout,Options.GetOptIncludePath(),Options.GetOptIncludeComment());

        // save collection if it was created
        if( created ){
            if( Jobs.SaveCollection() == false ){
                vout << endl;
                vout << "<b><red> ERROR: Unable to save collection '" << Jobs.GetCollectionName() << "'!</red></b>" << endl;
                return(false);
            }
        }

        if( Jobs.GetCollectionSiteName() != AMSGlobalConfig.GetActiveSiteName() ){
            vout << endl;
            vout << "<b><red> ERROR: The collection site '" << Jobs.GetCollectionSiteName();
            vout << " does not match with the active site '" << AMSGlobalConfig.GetActiveSiteName() << "'!</red></b>" << endl;
            return(false);
        }

        vout << endl;
        vout << "<blue> INFO: Collection was opened.</blue>" << endl;

        ShellProcessor.SetVariable("INF_COLLECTION_NAME",Jobs.GetCollectionName());
        ShellProcessor.SetVariable("INF_COLLECTION_PATH",Jobs.GetCollectionPath());
        ShellProcessor.SetVariable("INF_COLLECTION_ID",Jobs.GetCollectionID());
        return(true);
// -------------------------------------------------------------------
// close
// -------------------------------------------------------------------
    } else if( Options.GetProgArg(1) == "close" ) {
        if( Jobs.LoadCollection(Options.GetProgArg(0)) == false ){
            vout << endl;
            vout << "<b><red> ERROR: Unable to load collection '" << Jobs.GetCollectionName() << "'!</red></b>" << endl;
            return(false);
        }

        // update status of live jobs
        Jobs.UpdateJobStatuses();

        // print info
        Jobs.PrintCollectionInfo(vout,Options.GetOptIncludePath(),Options.GetOptIncludeComment());

        if( Jobs.GetCollectionSiteName() != AMSGlobalConfig.GetActiveSiteName() ){
            vout << endl;
            vout << "<b><red> ERROR: The collection site '" << Jobs.GetCollectionSiteName();
            vout << " does not match with the active site '" << AMSGlobalConfig.GetActiveSiteName() << "'!</red></b>" << endl;
            return(false);
        }

        vout << endl;
        vout << "<blue> INFO: Collection was closed.</blue>" << endl;
        ShellProcessor.UnsetVariable("INF_COLLECTION_NAME");
        ShellProcessor.UnsetVariable("INF_COLLECTION_PATH");
        ShellProcessor.UnsetVariable("INF_COLLECTION_ID");
        return(true);
// -------------------------------------------------------------------
// submit
// -------------------------------------------------------------------
    } else if( Options.GetProgArg(1) == "submit" ) {
        if( Jobs.LoadCollection(Options.GetProgArg(0)) == false ){
            vout << endl;
            vout << "<b><red> ERROR: Unable to load collection '" << Jobs.GetCollectionName() << "'!</red></b>" << endl;
            return(false);
        }

        // update status of live jobs
        Jobs.UpdateJobStatuses();

        // print info
        Jobs.PrintCollectionInfo(vout,Options.GetOptIncludePath(),Options.GetOptIncludeComment());

        if( Jobs.GetCollectionSiteName() != AMSGlobalConfig.GetActiveSiteName() ){
            vout << endl;
            vout << "<b><red> ERROR: The collection site '" << Jobs.GetCollectionSiteName();
            vout << " does not match with the active site '" << AMSGlobalConfig.GetActiveSiteName() << "'!</red></b>" << endl;
            return(false);
        }

        // are there jobs for resubmission?
        int njobs = Jobs.NumOfJobsToBeResubmitted();
        if( njobs <= 0 ){
            vout << endl;
            vout << "<blue> INFO: No jobs that require resubmission were found.</blue>" << endl;
            return(true);
        }

        Jobs.PrintCollectionResubmitJobs(vout);

        // ask for resubmission?
        if( ! Options.GetOptAssumeYes() ){
            vout << endl;
            vout << "Do you want to submit jobs to the Torque server (YES/NO)?" << endl;
            vout << "> ";

            string answer;
            cin >> answer;

            CSmallString sanswer(answer.c_str());
            sanswer.ToUpperCase();

            if( sanswer != "YES" ) return(true);
            vout << "Listed jobs will be resubmitted to the Torque server!" << endl;
        }

        // resubmit jobs
        Jobs.CollectionResubmitJobs(vout);

        return(true);
// -------------------------------------------------------------------
// info
// -------------------------------------------------------------------
    } else if( Options.GetProgArg(1) == "info" ) {
        if( Jobs.LoadCollection(Options.GetProgArg(0)) == false ){
            vout << endl;
            vout << "<b><red> ERROR: Unable to load collection '" << Jobs.GetCollectionName() << "'!</red></b>" << endl;
            return(false);
        }

        // update status of live jobs
        Jobs.UpdateJobStatuses();

        // print info
        Jobs.PrintCollectionInfo(vout,Options.GetOptIncludePath(),Options.GetOptIncludeComment());
        return(true);

// -------------------------------------------------------------------
// stat
// -------------------------------------------------------------------
    } else if( Options.GetProgArg(1) == "stat" ) {
        if( Jobs.LoadCollection(Options.GetProgArg(0)) == false ){
            vout << endl;
            vout << "<b><red> ERROR: Unable to load collection '" << Jobs.GetCollectionName() << "'!</red></b>" << endl;
            return(false);
        }

        // update status of live jobs
        Jobs.UpdateJobStatuses();

        // print info
        Jobs.PrintCollectionInfo(vout,Options.GetOptIncludePath(),Options.GetOptIncludeComment());

        // print statistics
        Jobs.PrintCollectionStat(vout,Options.GetOptCompact(),Options.GetOptIncludePath(),Options.GetOptIncludeComment());
        return(true);

// -------------------------------------------------------------------
// kill
// -------------------------------------------------------------------
    } else if( Options.GetProgArg(1) == "kill" ) {
        if( Jobs.LoadCollection(Options.GetProgArg(0)) == false ){
            vout << endl;
            vout << "<b><red> ERROR: Unable to load collection '" << Jobs.GetCollectionName() << "'!</red></b>" << endl;
            return(false);
        }

        // update status of live jobs
        Jobs.UpdateJobStatuses();

        // print info
        Jobs.PrintCollectionInfo(vout,Options.GetOptIncludePath(),Options.GetOptIncludeComment());

        // keep only running jobs
        Jobs.KeepOnlyLiveJobs();

        if( Jobs.GetNumberOfJobs() == 0 ){
            vout << "No queued or running jobs to be killed!" << endl;
            return(false);
        }

        // ask for resubmission?
        if( ! Options.GetOptAssumeYes() ){
            vout << endl;
            vout << "Do you want to kill queued and running jobs (YES/NO)?" << endl;
            vout << "> ";

            string answer;
            cin >> answer;

            CSmallString sanswer(answer.c_str());
            sanswer.ToUpperCase();

            if( sanswer != "YES" ) return(true);
            vout << "Queued and running jobs will be killed!" << endl;
        }

        // kill jobs
        Jobs.LoadAllInfoFiles();
        Jobs.KillAllJobsWithInfo(vout,true);
        Jobs.SaveAllInfoFiles();

        return(true);
// -------------------------------------------------------------------
// unrecognized
// -------------------------------------------------------------------
    } else {
        vout << endl;
        vout << "<b><red> ERROR: Unsupported  action '" << Options.GetProgArg(1) << "' was requested!</red></b>" << endl;
    }

    return(false);
}

//------------------------------------------------------------------------------

bool CCollection::Finalize(void)
{
    if( ! ErrorSystem.IsError() ){
        ShellProcessor.SetExitCode(0);
    } else {
        ShellProcessor.RollBack();
        ShellProcessor.SetExitCode(1);
    }

    CSmallTimeAndDate dt;
    dt.GetActualTimeAndDate();

    vout << endl;

    vout << high;
    vout << endl;
    vout << "# ==============================================================================" << endl;
    vout << "# pcollection terminated at " << dt.GetSDateAndTime() << endl;
    vout << "# ==============================================================================" << endl;

    if( ErrorSystem.IsError() || Options.GetOptVerbose() ){
        vout << low;
        ErrorSystem.PrintErrors(vout);
    }

    vout << endl;

    ShellProcessor.BuildEnvironment();

    return(true);
}

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

