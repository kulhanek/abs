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
#include <GlobalConfig.hpp>
#include <NodeList.hpp>
#include <JobList.hpp>
#include <FileSystem.hpp>

using namespace std;

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
    // init all subsystems
    if( TorqueConfig.LoadSystemConfig() == false ){
        ES_ERROR("unable to load torque config");
        return(false);
    }

    PluginDatabase.SetPluginPath(GlobalConfig.GetPluginsDir());
    if( PluginDatabase.LoadPlugins(GlobalConfig.GetPluginsConfigDir()) == false ){
        ES_ERROR("unable to load plugins");
        return(false);
    }

    vout << medium;

    // check if user has valid ticket
    if( TorqueConfig.IsUserTicketValid(vout) == false ){
        ES_TRACE_ERROR("user does not have valid ticket");
        return(false);
    }

    // we need ticket here
    if( Torque.Init() == false ){
        ES_ERROR("unable to init torque");
        return(false);
    }

    if( User.InitUser() == false ){
        ES_ERROR("unable to init current user");
        return(false);
    }

    if( Torque.GetQueues(QueueList) == false ){
        ES_ERROR("unable to get list of queues");
        return(false);
    }

    QueueList.RemoveDisabledQueues();
    QueueList.RemoveStoppedQueues();
    QueueList.RemoveInaccesibleQueues(User);
    QueueList.RemoveNonexecutiveQueues();
    QueueList.SortByName();

    if( Torque.GetNodes(NodeList) == false ){
        ES_ERROR("unable to load nodes");
        return(false);
    }

    // get unique properties
    //NodeList.PrepareUniqueProperties();

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
    CJobPtr Job = CJobPtr(new CJob);

    // start job preparation
    Job->CreateHeader();

    // basic section
    Job->CreateBasicSection();
    Job->SetExternalOptions();

    CSmallString qa,jn,rs,sm;
    qa = Options.GetProgArg(0);
    jn = Options.GetProgArg(1);
    if( Options.GetNumberOfProgArgs() >= 3 ) rs = Options.GetProgArg(2);
    if( Options.GetNumberOfProgArgs() >= 4 ) sm = Options.GetProgArg(3);

    if( Job->SetArguments(qa,jn,rs,sm) == false ) return(false);
    if( Job->CheckRuntimeFiles(vout,TorqueConfig,Options.GetOptIgnoreRuntimeFiles()) == false ){
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
        vout << "<b><red>        The presence of runtime files indicates that another job</red></b>" << endl;
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
                vout << "<blue>        thus it is potentially dangerous to blindly remove runtime files via:</blue>" << endl;
                vout << "<blue>        rm -f *.info *.infex *.infout *.stdout *.nodes *.gpus *.infkey ___JOB_IS_RUNNING___</blue>" << endl;
            } else {
                vout << "" << endl;
                vout << "        If you really want to submit the job, you have to remove runtime" << endl;
                vout << "        files of previous one. Please, be very sure that previous job has " << endl;
                vout << "        been already terminated otherwise undefined behaviour can occur!" << endl;
                vout << "" << endl;
                vout << "        Type following to remove runtime files:" << endl;
                vout << "        <b>rm -f *.info *.infex *.infout *.stdout *.nodes *.gpus *.infkey ___JOB_IS_RUNNING___</b>" << endl;
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
    Job->PrintBasic(vout);

    // resources
    if( Job->DecodeTorqueResources(vout) == false ){
        ES_TRACE_ERROR("unable to decode resources");
        return(false);
    }
    Job->PrintResources(vout);

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
    if( Job->SubmitJob(Job,vout,false) == false ){
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

bool CSubmit::SubmitJobHeader(void)
{
    CJobPtr Job = CJobPtr(new CJob);

    // start job preparation
    Job->CreateHeader();

    // basic section
    Job->CreateBasicSection();
    Job->SetExternalOptions();

    CSmallString qa,jn,rs,sm;
    qa = Options.GetProgArg(0);
    jn = Options.GetProgArg(1);
    if( Options.GetNumberOfProgArgs() >= 3 ) rs = Options.GetProgArg(2);
    if( Options.GetNumberOfProgArgs() >= 4 ) sm = Options.GetProgArg(3);

    if( Job->SetArguments(qa,jn,rs,sm) == false ) return(false);

    // set output number
    if( Job->SetOutputNumber(vout,1) == false ) return(false);

    if( Job->CheckRuntimeFiles(vout,TorqueConfig,Options.GetOptIgnoreRuntimeFiles()) == false ){
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
        vout << "        <b>rm -f *.info *.infex *.infout *.stdout *.nodes *.gpus *.infkey ___JOB_IS_RUNNING___</b>" << endl;

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
    Job->PrintBasic(vout);

    // resources
    if( Job->DecodeTorqueResources(vout) == false ){
        ES_TRACE_ERROR("unable to decode resources");
        return(false);
    }
    Job->PrintResources(vout);

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
    if( Job->SubmitJob(Job,vout,false) == false ){
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
    CJobPtr Job = CJobPtr(new CJob);

    // start job preparation
    Job->CreateHeader();

    // basic section
    Job->CreateBasicSection();
    Job->SetExternalOptions();

    CSmallString qa,jn,rs,sm;
    qa = Options.GetProgArg(0);
    jn = Options.GetProgArg(1);
    if( Options.GetNumberOfProgArgs() >= 3 ) rs = Options.GetProgArg(2);
    if( Options.GetNumberOfProgArgs() >= 4 ) sm = Options.GetProgArg(3);

    if( Job->SetArguments(qa,jn,rs,sm) == false ) return(false);

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
    if( Job->DecodeTorqueResources(stmp) == false ){
        ES_TRACE_ERROR("unable to decode resources");
        return(false);
    }
    // last job check
    if( Job->LastJobCheck(stmp) == false ){
        ES_TRACE_ERROR("job submission was canceled by last check procedure");
        return(false);
    }

    // submit job
    if( Job->SubmitJob(Job,vout,true) == false ){
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
    int mode = CFileSystem::GetPosixMode(presubmit_hook);
    int fmode = (mode | 0100 ) & 0777;
    CFileSystem::SetPosixMode(presubmit_hook,fmode);

    CFileName pwd = CJob::GetJobInputPath();
    presubmit_hook =  pwd / presubmit_hook;

    // execute script
    bool result = system(presubmit_hook) == 0;

    // dissable execution flag for the executable
    fmode = (mode & ~0111 ) & 0777;
    CFileSystem::SetPosixMode(presubmit_hook,fmode);

    return(result);
}

//------------------------------------------------------------------------------

bool CSubmit::Finalize(void)
{
    // unload plugins
    PluginDatabase.UnloadPlugins();

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

