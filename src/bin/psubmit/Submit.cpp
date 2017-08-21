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

#include "Submit.hpp"
#include <ErrorSystem.hpp>
#include <SmallTimeAndDate.hpp>
#include <PluginDatabase.hpp>
#include <NodeList.hpp>
#include <JobList.hpp>
#include <FileSystem.hpp>
#include <BatchServers.hpp>
#include <Host.hpp>
#include <vector>
#include <sys/stat.h>
#include <boost/algorithm/string/split.hpp>
#include <boost/algorithm/string/classification.hpp>
#include <boost/algorithm/string/join.hpp>
#include <boost/algorithm/string/trim.hpp>

using namespace std;
using namespace boost;

//------------------------------------------------------------------------------

MAIN_ENTRY(CSubmit)

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

CSubmit::CSubmit(void)
{
}

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

int CSubmit::Init(int argc,char* argv[])
{
    // encode program options, all check procedures are done inside of CABFIntOpts
        int result = Options.ParseCmdLine(argc,argv);

    // should we exit or was it error?
        if(result != SO_CONTINUE) return(result);

    // attach verbose stream to terminal stream and set desired verbosity level
        vout.Attach(Console);
        if( Options.GetOptSilent() ){
            vout.Verbosity(CVerboseStr::low);
        } else {
            if( Options.GetOptVerbose() ) {
                vout.Verbosity(CVerboseStr::high);
            } else {
                vout.Verbosity(CVerboseStr::medium);
            }
        }

        CSmallTimeAndDate dt;
        dt.GetActualTimeAndDate();

        vout << high;
        vout << endl;
        vout << "# ==============================================================================" << endl;
        vout << "# psubmit (ABS utility) started at " << dt.GetSDateAndTime() << endl;
        vout << "# ==============================================================================" << endl;

        return(SO_CONTINUE);
}

//------------------------------------------------------------------------------

bool CSubmit::Run(void)
{
    if( ABSConfig.IsABSAvailable(vout) == false ){
        ES_TRACE_ERROR("abs not configured or available");
        return(false);
    }

    // init all subsystems
    if( ABSConfig.LoadSystemConfig() == false ){
        ES_ERROR("unable to load ABSConfig config");
        return(false);
    }

    vout << medium;

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

    // execute presubmit hook
    if( ExecPresubmitHook() == false ){
        vout << endl;
        vout << " <b><red>ERROR: Unable to execute the presubmit-hook (or .presubmit-hook) script!</red></b>" << endl;
        vout << " <b><red>       Aborting the job submission ...</red></b>" << endl;
        ES_TRACE_ERROR("presubmit hook failed");
        return(false);
    }

    if( (Options.GetOptNumOfCopies() > 0) && (Options.GetOptResubmitMode() == true) ){
        ES_TRACE_ERROR("unsupported: numofcopies > 0 and repeat == true");
        return(false);
    }

    // submit a single job
    if( Options.GetOptNumOfCopies() == 0 ){
        // single job
        return( SubmitJobFull() );
    }

    // submit job copies
    if( SubmitJobHeader() ){
        vout << endl;
        vout << "Submitting remaining jobs ..." << endl;
        for(int i=2; i <= Options.GetOptNumOfCopies(); i++){
            if( SubmitJobCopy(i) == false ) return(false);
        }
    }

    return(true);
}

//------------------------------------------------------------------------------

bool CSubmit::SubmitJobFull(void)
{
    CJobPtr Job = JobList.CreateNewJob();

    // start job preparation
    Job->CreateHeader();

    // basic section
    Job->CreateBasicSection();
    Job->SetExternalOptions();

    CSmallString qa,jn,rs;
    qa = Options.GetProgArg(0);
    jn = Options.GetProgArg(1);

    // generate list of resources
    vector<string> argres;
    vector<string> allres;
    for(int i=2; i < Options.GetNumberOfProgArgs(); i++){
        string stm(Options.GetProgArg(i));
        split(argres,stm,is_any_of(","),boost::token_compress_on);
        allres.insert(allres.end(),argres.begin(),argres.end());
    }
    rs = join(allres,",");

    if( Job->SetArguments(qa,jn,rs) == false ) return(false);
    if( Job->CheckRuntimeFiles(vout,Options.GetOptIgnoreRuntimeFiles()) == false ){
        CJobList jobs;
        jobs.InitByInfoFiles(".",false);
        if( jobs.GetNumberOfJobs() > 0 ){
            jobs.SortByPrepareDateAndTime();
            jobs.UpdateJobStatuses();
            vout << endl;
            vout << ">>> List of jobs from info files ..." << endl;
            jobs.PrintInfosCompact(vout,false,true);
        }

        vout << endl;
        vout << " <b><red>ERROR: Infinity runtime files were detected in the job input directory!</red></b>" << endl;
        vout << "" << endl;
        vout << "<b><red>        The presence of the runtime files indicates that another job</red></b>" << endl;
        vout << "<b><red>        has been started in this directory. Multiple job submission </red></b>" << endl;
        vout << "<b><red>        within the same directory is not permitted by the Infinity system.</red></b>" << endl;

        if( (jobs.GetNumberOfJobsFromBatchSys(EJS_SUBMITTED) > 0) || (jobs.GetNumberOfJobsFromBatchSys(EJS_RUNNING) > 0) ){
            vout << "" << endl;
            vout << "<blue>        At least one job is waiting or running in the batch system (state: Q, R),</blue>" << endl;
            vout << "<blue>        please use the pkill command to remove this job from the batch system first!</blue>" << endl;
        } else {
            if( (jobs.GetNumberOfJobs(EJS_INCONSISTENT) > 0) || (jobs.GetNumberOfJobs(EJS_ERROR) > 0) ){
                vout << "" << endl;
                vout << "<blue>        At least one job is in inconsistent or error state (state: IN, ER),</blue>" << endl;
                vout << "<blue>        thus it is potentially dangerous to blindly remove the runtime files via:</blue>" << endl;
                vout << "<blue>        premovertf</blue>" << endl;
            } else {
                vout << "" << endl;
                vout << "        If you really want to submit the job, you have to remove runtime" << endl;
                vout << "        files from the previous run. Please, be very sure that the previous job has " << endl;
                vout << "        been already terminated otherwise undefined behaviour can occur!" << endl;
                vout << "" << endl;
                vout << "        Type following to remove runtime files:" << endl;
                vout << "        <b>premovertf</b>" << endl;
            }
        }
        ES_TRACE_ERROR("runtime files detected");
        return(false);
    }

    ERetStatus retstat = Job->JobInput(vout);
    if( retstat == ERS_FAILED ){
        ES_TRACE_ERROR("unable to set job input");
        return(false);
    }
    if( retstat == ERS_TERMINATE ){
        return(true);
    }

    vout << endl;
    Job->PrintBasicV3(vout);

    // resources
    if( Job->DecodeResources(vout,Options.GetOptExpertMode()) == false ){
        ES_TRACE_ERROR("unable to decode resources");
        return(false);
    }
    Job->PrintResourcesV3(vout);

    // last job check
    if( Job->LastJobCheck(vout) == false ){
        ES_TRACE_ERROR("job submission was canceled by last check procedure");
        return(false);
    }

    // submit job
    if( Job->ShouldSubmitJob(vout,Options.GetOptAssumeYes()) == false ){
        ES_TRACE_ERROR("job submission was canceled by an user");
        return(false);
    }

    if( Options.GetOptResubmitMode() ){

        CSmallString src = ABSConfig.GetSystemConfigItem("INF_RETRY_COUNT");
        CSmallString srt = ABSConfig.GetSystemConfigItem("INF_RETRY_TIME");
        int rc = 3;
        if( src != NULL ){
            rc = src.ToInt();
        }
        int rt = 600;
        if( srt != NULL ){
            rt = srt.ToInt();
        }

        bool success = false;
        for(int i = 0; i <= rc; i++){
            if( Job->SubmitJob(vout,false,Options.GetOptVerbose()) == true ){
                success = true;
                break;
            }
            vout << endl;
            vout << "ERROR: The job submission was not sucessfull! I will retry in " << rt << " seconds." << endl;
            vout << endl;
            sleep(rt);
        }
        if( ! success ){
            ES_TRACE_ERROR("unable to submit job");
            return(false);
        }
    } else {
        if( Job->SubmitJob(vout,false,Options.GetOptVerbose()) == false ){
            ES_TRACE_ERROR("unable to submit job");
            return(false);
        }
    }

    // save job info
    if( Job->SaveInfoFileWithPerms() == false ){
        ES_ERROR("unable to save job info file");
        return(false);
    }

    if( Job->SaveJobKey() == false ){
        ES_ERROR("unable to save job key file");
        return(false);
    }

    return(true);
}

//------------------------------------------------------------------------------

bool CSubmit::SubmitJobHeader(void)
{
    CJobPtr Job = JobList.CreateNewJob();

    // start job preparation
    Job->CreateHeader();

    // basic section
    Job->CreateBasicSection();
    Job->SetExternalOptions();

    CSmallString qa,jn,rs;
    qa = Options.GetProgArg(0);
    jn = Options.GetProgArg(1);

    // generate list of resources
    vector<string> argres;
    vector<string> allres;
    for(int i=2; i < Options.GetNumberOfProgArgs(); i++){
        string stm(Options.GetProgArg(i));
        split(argres,stm,is_any_of(","),boost::token_compress_on);
        allres.insert(allres.end(),argres.begin(),argres.end());
    }
    rs = join(allres,",");

    if( Job->SetArguments(qa,jn,rs) == false ) return(false);

    // set output number
    if( Job->SetOutputNumber(vout,1) == false ) return(false);

    if( Job->CheckRuntimeFiles(vout,Options.GetOptIgnoreRuntimeFiles()) == false ){
        CJobList jobs;
        jobs.InitByInfoFiles(".",false);
        if( jobs.GetNumberOfJobs() > 0 ){
            jobs.SortByPrepareDateAndTime();
            jobs.UpdateJobStatuses();
            vout << endl;
            vout << ">>> List of jobs from info files ..." << endl;
            jobs.PrintInfosCompact(vout,false,true);
        }

        vout << endl;
        vout << " <b><red>ERROR: Infinity runtime files were detected in the job input directory!</red></b>" << endl;
        vout << "" << endl;
        vout << "        The presence of runtime files indicates that another job" << endl;
        vout << "        has been started in this directory. Multiple job submission " << endl;
        vout << "        within the same directory is not permitted by the Infinity system." << endl;
        vout << "" << endl;
        vout << "        If you really want to submit the job, you have to remove runtime" << endl;
        vout << "        files of previous one. Please, be very sure that previous job has " << endl;
        vout << "        been already terminated otherwise undefined behaviour can occur!" << endl;
        vout << "" << endl;
        vout << "        Type following to remove runtime files:" << endl;
        vout << "        <b>premovertf</b>" << endl;

        ES_TRACE_ERROR("runtime files detected");
        return(false);
    }

    ERetStatus retstat = Job->JobInput(vout);
    if( retstat == ERS_FAILED ){
        ES_TRACE_ERROR("unable to set job input");
        return(false);
    }
    if( retstat == ERS_TERMINATE ){
        return(true);
    }
    vout << endl;
    Job->PrintBasicV3(vout);

    // resources
    if( Job->DecodeResources(vout,Options.GetOptExpertMode()) == false ){
        ES_TRACE_ERROR("unable to decode resources");
        return(false);
    }
    Job->PrintResourcesV3(vout);

    // last job check
    if( Job->LastJobCheck(vout) == false ){
        ES_TRACE_ERROR("job submission was canceled by last check procedure");
        return(false);
    }

    // submit job
    if( Job->ShouldSubmitJob(vout,Options.GetOptAssumeYes()) == false ){
        ES_TRACE_ERROR("job submission was canceled by an user");
        return(false);
    }
    if( Job->SubmitJob(vout,false,Options.GetOptVerbose()) == false ){
        ES_TRACE_ERROR("unable to submit job");
        return(false);
    }

    // save job info
    if( Job->SaveInfoFileWithPerms() == false ){
        ES_ERROR("unable to save job info file");
        return(false);
    }

    if( Job->SaveJobKey() == false ){
        ES_ERROR("unable to save job key file");
        return(false);
    }

    return(true);
}

//------------------------------------------------------------------------------

bool CSubmit::SubmitJobCopy(int i)
{
    CJobPtr Job = JobList.CreateNewJob();

    // start job preparation
    Job->CreateHeader();

    // basic section
    Job->CreateBasicSection();
    Job->SetExternalOptions();

    CSmallString qa,jn,rs,sm;
    qa = Options.GetProgArg(0);
    jn = Options.GetProgArg(1);
    if( Options.GetNumberOfProgArgs() >= 3 ) rs = Options.GetProgArg(2);

    if( Job->SetArguments(qa,jn,rs) == false ) return(false);

    // set output number
    if( Job->SetOutputNumber(vout,i) == false ) return(false);

    stringstream stmp;

    ERetStatus retstat = Job->JobInput(stmp);
    if( retstat == ERS_FAILED ){
        ES_TRACE_ERROR("unable to set job input");
        return(false);
    }
    if( retstat == ERS_TERMINATE ){
        return(true);
    }
    // resources
    if( Job->DecodeResources(stmp,Options.GetOptExpertMode()) == false ){
        ES_TRACE_ERROR("unable to decode resources");
        return(false);
    }
    // last job check
    if( Job->LastJobCheck(stmp) == false ){
        ES_TRACE_ERROR("job submission was canceled by last check procedure");
        return(false);
    }

    // submit job
    if( Job->SubmitJob(vout,true,Options.GetOptVerbose()) == false ){
        ES_TRACE_ERROR("unable to submit job");
        return(false);
    }

    // save job info
    if( Job->SaveInfoFileWithPerms() == false ){
        ES_ERROR("unable to save job info file");
        return(false);
    }

    if( Job->SaveJobKey() == false ){
        ES_ERROR("unable to save job key file");
        return(false);
    }

    return(true);
}

//------------------------------------------------------------------------------

bool CSubmit::ExecPresubmitHook(void)
{
    CFileName presubmit_hook;

    if( CFileSystem::IsFile("presubmit-hook") ){
        presubmit_hook = "presubmit-hook";
    } else if( CFileSystem::IsFile(".presubmit-hook") ) {
        presubmit_hook = ".presubmit-hook";
    }

    if( presubmit_hook == NULL ) return(true);  // nothing to do

    // make the script executable
    mode_t mode = CFileSystem::GetPosixMode(presubmit_hook);
    mode_t fmode = (mode | 0100 ) & 0777;
    chmod(presubmit_hook,fmode);

    CFileName pwd = CJob::GetJobInputPath();
    presubmit_hook =  pwd / presubmit_hook;

    // execute script
    bool result = system(presubmit_hook) == 0;

    // dissable execution flag for the executable
    fmode = (mode & ~0111 ) & 0777;
    chmod(presubmit_hook,fmode);

    return(result);
}

//------------------------------------------------------------------------------

bool CSubmit::Finalize(void)
{
    CSmallTimeAndDate dt;
    dt.GetActualTimeAndDate();

    vout << endl;
    vout << high;
    vout << "# ==============================================================================" << endl;
    vout << "# psubmit terminated at " << dt.GetSDateAndTime() << endl;
    vout << "# ==============================================================================" << endl;

    if( Options.GetOptVerbose() || ErrorSystem.IsError() ){
        vout << low;
        ErrorSystem.PrintErrors(vout);
    }

    vout << endl;

    return(true);
}

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

