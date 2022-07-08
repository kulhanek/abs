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
#include <DirectoryEnum.hpp>
#include <FileSystem.hpp>
#include <Shell.hpp>
#include <Host.hpp>
#include <iomanip>

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
    vout << low;

    if( ABSConfig.IsABSAvailable(vout) == false ){
        ES_TRACE_ERROR("abs not configured or available");
        return(false);
    }

    // init all subsystems
    if( ABSConfig.LoadSystemConfig() == false ){
        ES_ERROR("unable to load ABSConfig config");
        return(false);
    }

    // user must be initializaed before ABSConfig.IsUserTicketValid()
    User.InitUser();

    // check if user has valid ticket
    if( ABSConfig.IsUserTicketValid(vout) == false ){
        ES_TRACE_ERROR("user does not have valid ticket");
        return(false);
    }

    Host.InitGlobalSetup();
    Host.InitHostFile();

    if( AliasList.LoadConfig() == false ){
        ES_TRACE_ERROR("unable to load aliases");
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
        if( Jobs.LoadCollection(collname,false) == false ){
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

    CSmallString action = "info";
    if( Options.GetNumberOfProgArgs() >= 2 ){
        action = Options.GetProgArg(1);
    }

// -------------------------------------------------------------------
// create
// -------------------------------------------------------------------
    if( action == "create" ){
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
    } else if( action == "open" ) {
        bool created = false;
        if( Jobs.LoadCollection(Options.GetProgArg(0),false) == false ){
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
    } else if( action == "close" ) {
        if( Jobs.LoadCollection(Options.GetProgArg(0),false) == false ){
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
// prepare
// -------------------------------------------------------------------
    } else if( action == "prepare" ) {
        bool created = false;
        if( Jobs.LoadCollection(Options.GetProgArg(0),false) == false ){
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

        CShell::SetSystemVariable("INF_COLLECTION_NAME",Jobs.GetCollectionName());
        CShell::SetSystemVariable("INF_COLLECTION_PATH",Jobs.GetCollectionPath());
        CShell::SetSystemVariable("INF_COLLECTION_ID",Jobs.GetCollectionID());

        vout << endl;
        vout << "# Searching for jobs with the script name or input file: " << Options.GetProgArg(3) << endl;

        vout << endl;
        vout << "#        Job Input Path                  Comment            " << endl;
        vout << "# ---------------------------- -----------------------------" << endl;

        // find all jobs
        CFileName cwd;
        CFileSystem::GetCurrentDir(cwd);
        CFileName root;
        CFileName job = Options.GetProgArg(3);
        std::vector<CFileName> jobs;

        FindJobs(jobs,cwd,root,job);

        // sort jobs
        sort(jobs.begin(),jobs.end());

        if( jobs.size() != 0 ) {
            // list jobs
            std::vector<CFileName>::iterator it = jobs.begin();
            std::vector<CFileName>::iterator ie = jobs.end();
            while( it != ie ){
                CFileName path = *it;
                bool runtime_files = CJob::AreRuntimeFiles(path);
                if( ! runtime_files ){
                    vout << "  " << path << endl;
                    it++;
                } else {
                    vout << "  " << setw(3) << path << " ignored - it contains runtime files" << endl;
                    it = jobs.erase(it);
                }
            }
        }

        vout << "# ------------------------------------------------" << endl;
        vout << "# Number of found jobs: " << jobs.size() << endl;

        // ask for prepare?
        if( ! Options.GetOptAssumeYes() ){
            vout << endl;
            vout << "Do you want to prepare listed jobs for submission to the batch server (YES/NO)?" << endl;
            vout << "> ";

            string answer;
            cin >> answer;

            CSmallString sanswer(answer.c_str());
            sanswer.ToUpperCase();

            if( sanswer != "YES" ){
                vout << "No jobs were prepared!" << endl;
                ShellProcessor.RollBack();
                return(true);
            }
        } else {
            vout << endl;
        }

        vout << "Listed jobs will be prepared for submission to the batch server!" << endl;

        // setup psubmit command
        std::vector<std::string> args;
        for(int i=2; i < Options.GetNumberOfProgArgs(); i++){
            string arg(Options.GetProgArg(i));
            args.push_back(arg);
        }

        if( jobs.size() != 0 ) {
            // list jobs
            std::vector<CFileName>::iterator it = jobs.begin();
            std::vector<CFileName>::iterator ie = jobs.end();
            while( it != ie ){
                vout << endl;
                CFileName path = *it;
                if( CFileSystem::SetCurrentDir(path) == true ){

                CJob::SubmitJobFull(vout,args,
                                           false,
                                           false,
                                           false,
                                           true,
                                           false,
                                           Options.GetOptVerbose() );
                } else {
                    vout << "<red> >>> ERROR: Unable to change working directory to '" << path << "'!" << endl;
                }
                it++;
            }
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
    } else if( action == "submit" ) {
        if( Jobs.LoadCollection(Options.GetProgArg(0),false) == false ){
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
        Jobs.KeepCollectionJobsForResubmit();

        int njobs = Jobs.GetNumberOfJobs();
        if( njobs <= 0 ){
            vout << endl;
            vout << "<blue> INFO: No jobs that require resubmission were found.</blue>" << endl;
            return(true);
        }

        Jobs.PrintCollectionResubmitJobs(vout);

        // ask for resubmission?
        if( ! Options.GetOptAssumeYes() ){
            vout << endl;
            vout << "Do you want to submit jobs to the batch server (YES/NO)?" << endl;
            vout << "> ";

            string answer;
            cin >> answer;

            CSmallString sanswer(answer.c_str());
            sanswer.ToUpperCase();

            if( sanswer != "YES" ){
                vout << "No jobs were resubmitted!" << endl;
                return(true);
            }
        } else {
            vout << endl;
        }

        vout << "Listed jobs will be resubmitted to the batch server!" << endl;

        // resubmit jobs
        Jobs.CollectionResubmitJobs(vout,Options.GetOptVerbose());

        return(true);
// -------------------------------------------------------------------
// info
// -------------------------------------------------------------------
    } else if( action == "info" ) {
        if( Jobs.LoadCollection(Options.GetProgArg(0),false) == false ){
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
    } else if( action == "stat" ) {
        if( Jobs.LoadCollection(Options.GetProgArg(0),false) == false ){
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
    } else if( action == "kill" ) {
        if( Jobs.LoadCollection(Options.GetProgArg(0),false) == false ){
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
            vout << endl;
            vout << "No queued or running jobs were found to be killed!" << endl;
            return(false);
        }

        // ask for kill?
        if( ! Options.GetOptAssumeYes() ){
            vout << endl;
            vout << "Do you want to kill queued and running jobs (YES/NO)?" << endl;
            vout << "> ";

            string answer;
            cin >> answer;

            CSmallString sanswer(answer.c_str());
            sanswer.ToUpperCase();

            if( sanswer != "YES" ){
                vout << "No jobs were killed!" << endl;
                return(true);
            }
        } else {
            vout << endl;
        }

        vout << "Queued and running jobs will be killed!" << endl;

        // kill jobs
        Jobs.LoadAllInfoFiles();
        Jobs.KillAllJobsWithInfo(vout,true);
        Jobs.SaveAllInfoFiles();

        vout << endl;
        vout << "Done." << endl;

        return(true);
// -------------------------------------------------------------------
// clean
// -------------------------------------------------------------------
    } else if( action == "clean" ) {
        if( Jobs.LoadCollection(Options.GetProgArg(0),false) == false ){
            vout << endl;
            vout << "<b><red> ERROR: Unable to load collection '" << Jobs.GetCollectionName() << "'!</red></b>" << endl;
            return(false);
        }

        // update status of live jobs
        Jobs.UpdateJobStatuses();

        // print info
        Jobs.PrintCollectionInfo(vout,Options.GetOptIncludePath(),Options.GetOptIncludeComment());

        // ask for kill?
        if( ! Options.GetOptAssumeYes() ){
            vout << endl;
            vout << "Do you want to clean collection jobs (YES/NO)?" << endl;
            vout << "> ";

            string answer;
            cin >> answer;

            CSmallString sanswer(answer.c_str());
            sanswer.ToUpperCase();

            if( sanswer != "YES" ){
                vout << "No jobs were cleaned!" << endl;
                return(true);
            }
        } else {
            vout << endl;
        }

        vout << "Jobs will be cleaned!" << endl;

        Jobs.CleanJobs();

        vout << endl;
        vout << "Done." << endl;

        return(true);
// -------------------------------------------------------------------
// addjob
// -------------------------------------------------------------------
    } else if( action == "addjob" ){

        if( Jobs.LoadCollection(Options.GetProgArg(0),false) == false ){
            vout << endl;
            vout << "<b><red> ERROR: Unable to load collection '" << Jobs.GetCollectionName() << "'!</red></b>" << endl;
            return(false);
        }

        // update status of live jobs
        Jobs.UpdateJobStatuses();

        // print info
        Jobs.PrintCollectionInfo(vout,Options.GetOptIncludePath(),Options.GetOptIncludeComment());

        // add job
        if( Jobs.AddJobByPath(Options.GetProgArg(2)) == false ){
            vout << endl;
            vout << "<b><red> ERROR: Unable to add job '" << Options.GetProgArg(2) << "' to collection!</red></b>" << endl;
            return(false);
        }

        vout << endl;
        vout << "Job " << Options.GetProgArg(2) << " was added to collection." << endl;

        // save collection
        if( Jobs.SaveCollection() == false ){
            vout << endl;
            vout << "<b><red> ERROR: Unable to save collection '" << Jobs.GetCollectionName() << "'!</red></b>" << endl;
            return(false);
        }
        return(true);
// -------------------------------------------------------------------
// adddir
// -------------------------------------------------------------------
    } else if( action == "adddir" ){

        if( Jobs.LoadCollection(Options.GetProgArg(0),false) == false ){
            vout << endl;
            vout << "<b><red> ERROR: Unable to load collection '" << Jobs.GetCollectionName() << "'!</red></b>" << endl;
            return(false);
        }

        // update status of live jobs
        Jobs.UpdateJobStatuses();

        // print info
        Jobs.PrintCollectionInfo(vout,Options.GetOptIncludePath(),Options.GetOptIncludeComment());

        // add job dir
        if( Jobs.AddJobDirContainer(Options.GetProgArg(2)) == false ){
            vout << endl;
            vout << "<b><red> ERROR: Unable to add job directory '" << Options.GetProgArg(2) << "' to collection!</red></b>" << endl;
            return(false);
        }

        vout << endl;
        vout << "Job directory '" << Options.GetProgArg(2) << "' was added to collection." << endl;

        // save collection
        if( Jobs.SaveCollection() == false ){
            vout << endl;
            vout << "<b><red> ERROR: Unable to save collection '" << Jobs.GetCollectionName() << "'!</red></b>" << endl;
            return(false);
        }
        return(true);

// -------------------------------------------------------------------
// rmjob
// -------------------------------------------------------------------
    } else if( action == "rmjob" ){

        if( Jobs.LoadCollection(Options.GetProgArg(0),false) == false ){
            vout << endl;
            vout << "<b><red> ERROR: Unable to load collection '" << Jobs.GetCollectionName() << "'!</red></b>" << endl;
            return(false);
        }

        // update status of live jobs
        Jobs.UpdateJobStatuses();

        // print info
        Jobs.PrintCollectionInfo(vout,Options.GetOptIncludePath(),Options.GetOptIncludeComment());

        // ask for rmjob?
        if( ! Options.GetOptAssumeYes() ){
            vout << endl;
            vout << "Do you want to remove specified job from the collection (YES/NO)?" << endl;
            vout << "> ";

            string answer;
            cin >> answer;

            CSmallString sanswer(answer.c_str());
            sanswer.ToUpperCase();

            if( sanswer != "YES" ){
                vout << "No job was removed!" << endl;
                return(true);
            }
        } else {
            vout << endl;
        }

        vout << "Removing job with CID=" << Options.GetProgArg(2).ToInt() << endl;

        // remove job
        if( Jobs.RemoveCollectionJob(Options.GetProgArg(2).ToInt()) == false ){
            vout << endl;
            vout << "<b><red> ERROR: Unable to remove collection job '" << Options.GetProgArg(2).ToInt() << "'!</red></b>" << endl;
            return(false);
        }

        vout << "Done." << endl;

        // save collection
        if( Jobs.SaveCollection() == false ){
            vout << endl;
            vout << "<b><red> ERROR: Unable to save collection '" << Jobs.GetCollectionName() << "'!</red></b>" << endl;
            return(false);
        }
        return(true);
// -------------------------------------------------------------------
// unrecognized
// -------------------------------------------------------------------
    } else {
        vout << endl;
        vout << "<b><red> ERROR: Unsupported  action '" << action << "' was requested!</red></b>" << endl;
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

//------------------------------------------------------------------------------

void CCollection::FindJobs(std::vector<CFileName>& jobs,const CFileName& cwd,const CFileName& root,const CFileName& job)
{
    // find all jobs matching job criteria
    CDirectoryEnum dir;
    CFileName path;
    if( root != NULL ){
        path = cwd/root;
    } else {
        path = cwd;
    }
    dir.SetDirName(path);
    dir.StartFindFile("*");

    CFileName file;
    while( dir.FindFile(file) ){
        if( (file == "..") || (file == ".") ) continue;
        CFileName fdir;
        if( root != NULL ){
            fdir = root / file;
        } else {
            fdir = file;
        }
        if( CFileSystem::IsDirectory(cwd / fdir) ){
            if( CFileSystem::IsFile(cwd / fdir / job) ){
                jobs.push_back(fdir);
            } else {
                FindJobs(jobs,cwd,fdir,job);
            }
        }
    }
    dir.EndFindFile();
}

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

