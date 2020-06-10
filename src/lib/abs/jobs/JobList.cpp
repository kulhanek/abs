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

#include <JobList.hpp>
#include <DirectoryEnum.hpp>
#include <FileSystem.hpp>
#include <fnmatch.h>
#include <iomanip>
#include <ErrorSystem.hpp>
#include <UUID.hpp>
#include <XMLPrinter.hpp>
#include <XMLElement.hpp>
#include <XMLParser.hpp>
#include <CommonParser.hpp>
#include <ErrorSystem.hpp>
#include <boost/algorithm/string/split.hpp>
#include <boost/algorithm/string/classification.hpp>
#include <boost/algorithm/string/join.hpp>
#include <Shell.hpp>
#include <AMSGlobalConfig.hpp>
#include <ABSConfig.hpp>
#include <boost/filesystem.hpp>
#include <TerminalStr.hpp>

//------------------------------------------------------------------------------

using namespace std;
using namespace boost;
using namespace boost::algorithm;
namespace fs = boost::filesystem;

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

CJobList::CJobList(void)
{

}

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

void CJobList::InitByInfoFiles(const CFileName& dir,bool recursive)
{
    CDirectoryEnum direnum;
    direnum.SetDirName(dir);

    direnum.StartFindFile("*");

    CFileName file;

    while( direnum.FindFile(file) == true ){
        if( file == "." ) continue;
        if( file == ".." ) continue;

        CFileName suname = dir / file;
        if( recursive && CFileSystem::IsDirectory(suname) ){
            InitByInfoFiles(suname,recursive);
            continue;
        }

        if( fnmatch("*.info",file,0) == 0 ) {
            AddJob(suname);
        }
    }

    direnum.EndFindFile();
}

//------------------------------------------------------------------------------

void CJobList::InitFromRunningJob(void)
{
    // "$INF_WORK_DIR/${INF_WHOLE_NAME}.info"
    CFileName dir = CShell::GetSystemVariable("INF_WORK_DIR");
    CFileName name = CShell::GetSystemVariable("INF_WHOLE_NAME");
    CFileName info_name = dir / name + ".info";
    AddJob(info_name);
}

//------------------------------------------------------------------------------

bool CJobList::AddJob(const CFileName& info_name)
{
    // add info file
    CJobPtr p_job = CJobPtr(new CJob);
    if( p_job->LoadInfoFile(info_name) == false ){
        return(false);
    }
    push_back(p_job);
    return(true);
}

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

void CJobList::UpdateJobStatuses(void)
{
    list<CJobPtr>::iterator it = begin();
    list<CJobPtr>::iterator ie = end();

    while( it != ie ){
        CJobPtr p_job = *it;
        switch( p_job->GetJobInfoStatus() ){
            case EJS_SUBMITTED:
            case EJS_RUNNING:
                // get batch job status
                p_job->UpdateJobStatus();
                break;
            default:
                // nothing to do
                break;
        }

        it++;
    }
}

//------------------------------------------------------------------------------

void CJobList::KeepOnlyLiveJobs(bool submitted)
{
    list<CJobPtr>::iterator it = begin();
    list<CJobPtr>::iterator ie = end();

    while( it != ie ){
        CJobPtr p_job = *it;
        switch( p_job->GetJobBatchStatus() ){
            case EJS_SUBMITTED:
                if( submitted ){
                    // keep
                    it++;
                } else {
                    // delete
                    it = erase(it);
                }
                continue;
            case EJS_RUNNING:
                // keep
                it++;
                continue;
            default:
                // delete
                it = erase(it);
                continue;
        }
    }
}

//------------------------------------------------------------------------------

void CJobList::KeepUserJobs(const CSmallString& name)
{
    list<CJobPtr>::iterator it = begin();
    list<CJobPtr>::iterator ie = end();

    while( it != ie ){
        CJobPtr p_job = *it;
        if( p_job->GetBatchUserOwner() == name ){
            // keep
            it++;
            continue;
        } else {
            // delete
            it = erase(it);
            continue;
        }
    }
}

//------------------------------------------------------------------------------

void CJobList::KeepOnlyFinishedJobs(void)
{
    list<CJobPtr>::iterator it = begin();
    list<CJobPtr>::iterator ie = end();

    while( it != ie ){
        CJobPtr p_job = *it;
        if( p_job->GetJobBatchStatus() == EJS_FINISHED ){
            // keep
            it++;
        } else {
            // delete
            it = erase(it);
        }
    }
}

//------------------------------------------------------------------------------

void CJobList::KeepForClean(void)
{
    list<CJobPtr>::iterator it = begin();
    list<CJobPtr>::iterator ie = end();

    while( it != ie ){
        CJobPtr p_job = *it;
        if( (p_job->GetJobInfoStatus() == EJS_PREPARED) ||
            (p_job->GetJobInfoStatus() == EJS_FINISHED) ||
            (p_job->GetJobInfoStatus() == EJS_KILLED) ){
            // keep
            it++;
        } else {
            // delete
            it = erase(it);
        }
    }
}

//------------------------------------------------------------------------------

void CJobList::KeepOnlyMovedJobs(void)
{
    list<CJobPtr>::iterator it = begin();
    list<CJobPtr>::iterator ie = end();

    while( it != ie ){
        CJobPtr p_job = *it;
        switch( p_job->GetJobBatchStatus() ){
            case EJS_MOVED:
                // keep
                it++;
                continue;
            default:
                // delete
                it = erase(it);
                continue;
        }
    }
}

//------------------------------------------------------------------------------

void CJobList::KeepJobsByMask(struct SExpression* p_mask)
{
    list<CJobPtr>::iterator it = begin();
    list<CJobPtr>::iterator et = end();

    while( it != et ){
        CJobPtr p_job = *it;
        if( IsJobSelected(p_job,p_mask) == false ){
            it = erase(it);
        } else {
            it++;
        }
    }
}

//------------------------------------------------------------------------------

bool CJobList::IsJobSelected(CJobPtr p_job,struct SExpression* p_expr)
{
    if(p_expr->Selection != NULL) {
        return( IsJobSelected(p_job,p_expr->Selection) );
    } else {
        bool left;
        bool right;
        switch(p_expr->Operator) {
            case O_AND:
                left = IsJobSelected(p_job,p_expr->LeftExpression);
                right = IsJobSelected(p_job,p_expr->RightExpression);
                return( left && right );
            case O_OR:
                left = IsJobSelected(p_job,p_expr->LeftExpression);
                right = IsJobSelected(p_job,p_expr->RightExpression);
                return( left || right );
            case O_NOT:
                right = IsJobSelected(p_job,p_expr->RightExpression);
                return( ! right );

            default:
                ES_ERROR("<- unknown operator");
                return(false);
        };
    }
}

//------------------------------------------------------------------------------

bool CJobList::IsJobSelected(CJobPtr p_job,struct SSelection* p_sel)
{
    switch(p_sel->Type){
        case T_ID:
            switch(p_sel->Operator){
                case O_LT:
                    return( p_job->GetJobID().ToInt() < p_sel->IValue );
                case O_LE:
                    return( p_job->GetJobID().ToInt() <= p_sel->IValue );
                case O_GT:
                    return( p_job->GetJobID().ToInt() > p_sel->IValue );
                case O_GE:
                    return( p_job->GetJobID().ToInt() >= p_sel->IValue );
                case O_EQ:
                    return( p_job->GetJobID().ToInt() == p_sel->IValue );
                case O_NE:
                    return( p_job->GetJobID().ToInt() != p_sel->IValue );
                default:
                    ES_ERROR("<- unknown selection operator");
                    return(false);
            }
        case T_NCPUS:
            switch(p_sel->Operator){
                case O_LT:
                    return( p_job->GetNumOfCPUs() < p_sel->IValue );
                case O_LE:
                    return( p_job->GetNumOfCPUs() <= p_sel->IValue );
                case O_GT:
                    return( p_job->GetNumOfCPUs() > p_sel->IValue );
                case O_GE:
                    return( p_job->GetNumOfCPUs() >= p_sel->IValue );
                case O_EQ:
                    return( p_job->GetNumOfCPUs() == p_sel->IValue );
                case O_NE:
                    return( p_job->GetNumOfCPUs() != p_sel->IValue );
                default:
                    ES_ERROR("<- unknown selection operator");
                    return(false);
            }
        case T_NGPUS:
            switch(p_sel->Operator){
                case O_LT:
                    return( p_job->GetNumOfGPUs() < p_sel->IValue );
                case O_LE:
                    return( p_job->GetNumOfGPUs() <= p_sel->IValue );
                case O_GT:
                    return( p_job->GetNumOfGPUs() > p_sel->IValue );
                case O_GE:
                    return( p_job->GetNumOfGPUs() >= p_sel->IValue );
                case O_EQ:
                    return( p_job->GetNumOfGPUs() == p_sel->IValue );
                case O_NE:
                    return( p_job->GetNumOfGPUs() != p_sel->IValue );
                default:
                    ES_ERROR("<- unknown selection operator");
                    return(false);
            }
        case T_NNODES:
            switch(p_sel->Operator){
                case O_LT:
                    return( p_job->GetNumOfNodes() < p_sel->IValue );
                case O_LE:
                    return( p_job->GetNumOfNodes() <= p_sel->IValue );
                case O_GT:
                    return( p_job->GetNumOfNodes() > p_sel->IValue );
                case O_GE:
                    return( p_job->GetNumOfNodes() >= p_sel->IValue );
                case O_EQ:
                    return( p_job->GetNumOfNodes() == p_sel->IValue );
                case O_NE:
                    return( p_job->GetNumOfNodes() != p_sel->IValue );
                default:
                    ES_ERROR("<- unknown selection operator");
                    return(false);
            }
        case T_TITLE:
            switch(p_sel->Operator){
                case O_EQ:
                    return( fnmatch(p_sel->SValue,p_job->GetJobTitle(),0) == 0 );
                case O_NE:
                    return( fnmatch(p_sel->SValue,p_job->GetJobTitle(),0) != 0 );
                default:
                    ES_ERROR("<- unknown selection operator");
                    return(false);
            }
        case T_QUEUE:
            switch(p_sel->Operator){
                case O_EQ:
                    return( fnmatch(p_sel->SValue,p_job->GetQueue(),0) == 0 );
                case O_NE:
                    return( fnmatch(p_sel->SValue,p_job->GetQueue(),0) != 0 );
                default:
                    ES_ERROR("<- unknown selection operator");
                    return(false);
            }
        case T_USER:
            switch(p_sel->Operator){
                case O_EQ:
                    return( fnmatch(p_sel->SValue,p_job->GetBatchUserOwner(),0) == 0 );
                case O_NE:
                    return( fnmatch(p_sel->SValue,p_job->GetBatchUserOwner(),0) != 0 );
                default:
                    ES_ERROR("<- unknown selection operator");
                    return(false);
            }
        case T_PATH:
            switch(p_sel->Operator){
                case O_EQ:
                    return( fnmatch(p_sel->SValue,p_job->GetInputDir(),0) == 0 );
                case O_NE:
                    return( fnmatch(p_sel->SValue,p_job->GetInputDir(),0) != 0 );
                default:
                    ES_ERROR("<- unknown selection operator");
                    return(false);
            }
        case T_ST:{
                CSmallString sstr = p_job->GetJobBatchStatusString();
                switch(p_sel->Operator){
                    case O_EQ:
                        return( fnmatch(p_sel->SValue,sstr,0) == 0 );
                    case O_NE:
                        return( fnmatch(p_sel->SValue,sstr,0) != 0 );
                    default:
                        ES_ERROR("<- unknown selection operator");
                        return(false);
                }
            }
        case T_BS:{
                CSmallString sstr = p_job->GetShortServerName();
                switch(p_sel->Operator){
                    case O_EQ:
                        return( fnmatch(p_sel->SValue,sstr,0) == 0 );
                    case O_NE:
                        return( fnmatch(p_sel->SValue,sstr,0) != 0 );
                    default:
                        ES_ERROR("<- unknown selection operator");
                        return(false);
                }
            }
        default:
            ES_ERROR("<- unsupported selector");
            return(false);
    }

}

//------------------------------------------------------------------------------

bool CJobList::KillAllJobs(void)
{
    list<CJobPtr>::iterator it = begin();
    list<CJobPtr>::iterator ie = end();

    bool result = true;

    while( it != ie ){
        result &= (*it)->KillJob();
        it++;
    }

    return(result);
}

//------------------------------------------------------------------------------

bool CJobList::KillJobSoftly(std::ostream& sout)
{
    if( size() != 1 ){
        sout << "<b><red>ERROR: None or more than one job is suitable for the pkill command!</red></b>" << endl;
        sout << "<b><red>       In the second case, you must explicitly specified the info file of the job!</red></b>" << endl;
        sout << endl;
        return(false);
    }

    CJobPtr p_job = *begin();

    if( p_job->IsInteractiveJob() ){
        if( p_job->IsTerminalReady() == false ){
            sout << "<b><red>ERROR: The job is running but the job terminal is not ready yet.</red></b>" << endl;
            sout << "<b><red>       The pkill command cannot be used in the soft mode because the information about the job terminal is currently unknown.</red></b>" << endl;
            sout << "<b><red>       Please wait a while and try again.</red></b>" << endl;
            sout << endl;
            return(true);
        }
    } else {
        // do we have start/workdir
        if( ( p_job->GetItem("start/workdir","INF_MAIN_NODE",true) == NULL ) ||
            ( p_job->GetItem("start/workdir","INF_WORK_DIR",true) == NULL ) ) {
            sout << "<b><red>ERROR: The job is running but the job working directory is not ready yet.</red></b>" << endl;
            sout << "<b><red>       The pkill command cannot be used in the soft mode because the information about the job working directory is currently unknown.</red></b>" << endl;
            sout << "<b><red>       Please wait a while and try again.</red></b>" << endl;
            sout << endl;
            return(true);
        }
    }

    if( p_job->PrepareSoftKillEnv() == false ){
        sout << "<b><red>ERROR: Unable to prepare environment for the pkill command as some items are missing in the info file!</red></b>" << endl;
        sout << endl;
        return(false);
    }
    return(true);
}

//------------------------------------------------------------------------------

bool CJobList::KillAllJobsWithInfo(std::ostream& sout,bool force)
{
    sout << endl;
    sout << "#    Job ID        Status                                       " << endl;
    sout << "# ------------ -------------------------------------------------" << endl;

    list<CJobPtr>::iterator it = begin();
    list<CJobPtr>::iterator ie = end();

    bool result = true;

    while( it != ie ){
        CJobPtr p_job = *it;
        // print id
        CSmallString id = p_job->GetItem("submit/job","INF_JOB_ID",true);
        if( id == NULL ){
            id = p_job->GetItem("batch/job","INF_JOB_ID",true);
        }
        string stmp(id);
        vector<string> items;
        split(items,stmp,is_any_of("."));
        CSmallString nid;
        if( items.size() >= 1 ){
            nid = items[0];
        }
        if( nid.GetLength() >12 ){
            nid = nid.GetSubStringFromTo(0,11);
        }
        sout << "  " << left << setw(12) << nid << " ";
        if( p_job->IsInfoFileLoaded() == false ){
            if( force ){
                bool lok = p_job->KillJob(force);
                result &= lok;
                if( lok ){
                    sout << "no info file loaded - forced termination - ok" << endl;
                } else {
                    sout << "no info file loaded - forced termination - failed" << endl;
                }
            } else{
                sout << "no info file loaded - skipped" << endl;
            }
        } else {
            bool lok = p_job->KillJob();
            if( lok ){
                sout << "killed" << endl;
            } else {
                sout << "failed" << endl;
            }
            result &= lok;
        }
        it++;
    }

    return(true);
}

//------------------------------------------------------------------------------

void CJobList::CleanJobs(void)
{
    list<CJobPtr>::iterator it = begin();
    list<CJobPtr>::iterator ie = end();

    while( it != ie ){
        CJobPtr p_job = *it;
        it++;

        CFileName job_dir = p_job->GetItem("basic/jobinput","INF_INPUT_DIR",true);
        if( (job_dir != NULL) && CFileSystem::SetCurrentDir(job_dir) ){

            CJobList jobs;

            // get all jobs in job input directory
            jobs.InitByInfoFiles(".",false);

            // keep finished or terminated jobs
            jobs.KeepForClean();

            // sort by prepare date
            jobs.SortByPrepareDateAndTime();

            CTerminalStr    console;
            console.Attach(stderr); // due to _abs-collection

            CVerboseStr     vout;
            vout.Attach(console);

            vout << endl;
            if( jobs.GetNumberOfJobs() == 0 ){
                vout << "No job runtime files to clean ..." << std::endl;
                break;
            }

            vout << "Jobs for cleaning ..." << std::endl;
            jobs.PrintInfosCompact(vout,false,false);

            // clean
            jobs.CleanRuntimeFiles();
        }

    }
}

//------------------------------------------------------------------------------

bool CJobList::LoadAllInfoFiles(void)
{
    list<CJobPtr>::iterator it = begin();
    list<CJobPtr>::iterator ie = end();

    bool result = true;

    while( it != ie ){
        result &= (*it)->LoadInfoFile();
        it++;
    }

    return(true);
}

//------------------------------------------------------------------------------

bool CJobList::SaveAllInfoFiles(void)
{
    list<CJobPtr>::iterator it = begin();
    list<CJobPtr>::iterator ie = end();

    bool result = true;

    while( it != ie ){
        result &= (*it)->SaveInfoFile();
        it++;
    }

    return(true);
}

//------------------------------------------------------------------------------

bool CJobList::IsGoActionPossible(std::ostream& sout,bool force,bool proxy,bool noterm)
{
    list<CJobPtr>::iterator it = begin();
    list<CJobPtr>::iterator ie = end();

    int nrun = 0;

    while( it != ie ){
        CJobPtr p_job = *it;

        (*it)->PrintJobInfo(sout);
        sout << std::endl;

        switch( p_job->GetJobStatus() ){
            case EJS_NONE:
            case EJS_PREPARED:
                sout << "<b><blue> WARNING: The job was not started therefore it does not have the working directory.</blue></b>" << endl;
                sout << "<b><blue>          The pgo command cannot be used.</blue></b>" << endl;
                sout << endl;
                it = erase(it);
                break;
            case EJS_MOVED:
                sout << "<b><blue> WARNING: The job was moved into the other destination.</blue></b>" << endl;
                sout << "<b><blue>          The pgo command cannot be used.</blue></b>" << endl;
                sout << endl;
                it = erase(it);
                break;               
            case EJS_SUBMITTED:
                sout << "<b><blue> WARNING: The job was not started therefore it does not have the working directory.</blue></b>" << endl;
                sout << "<b><blue>          Wait until the job is started.</blue></b>" << endl;
                sout << endl;
                it++;
                break;
            case EJS_BOOTING:
                sout << "<b><blue> WARNING: The job is booting.</blue></b>" << endl;
                sout << "<b><blue>          Wait until the job is fully started.</blue></b>" << endl;
                sout << endl;
                it++;
                break;
            case EJS_RUNNING:
                if( p_job->IsInteractiveJob() == false ){
                    nrun++;
                    it++;
                } else {
                    if( noterm == false ){
                        // interactive jobs
                        if( p_job->GetJobName() == "gui" ){
                            if( proxy == false ){
                                // is the display local?
                                string display = string(CShell::GetSystemVariable("DISPLAY"));
                                vector<string> items;
                                split(items,display,is_any_of(":"),boost::token_compress_on);
                                if( (items.size() != 2) ){
                                    sout << "<b><blue> WARNING: This is a GUI job, which requires an active DISPLAY but none or incorrect is provided '" << display << "'!</blue></b>" << endl;
                                    sout << "<b><blue>          You can also try to initialize the VNC proxy via the --proxy option.</blue></b>" << endl;
                                    sout << endl;
                                    it = erase(it);
                                    break;
                                } else if( items[0].empty() == false ){
                                    sout << "<b><blue> WARNING: This is a GUI job, which requires the local DISPLAY but '" << display << "' is set!</blue></b>" << endl;
                                    sout << "<b><blue>          You can also try to initialize the VNC proxy via the --proxy option.</blue></b>" << endl;
                                    sout << endl;
                                    it = erase(it);
                                    break;
                                }
                            } else {
                                sout << "<b><blue> INFO: The VNC proxy wil be started for this GUI job.</blue></b>" << endl;
                                sout << endl;
                                p_job->ActivateGUIProxy();
                            }
                        }

                        if( p_job->IsTerminalReady() == false ){
                            sout << "<b><blue> WARNING: The job was started but the job terminal is not ready yet.</blue></b>" << endl;
                            sout << "<b><blue>          Wait for the job terminal ...</blue></b>" << endl;
                            sout << endl;
                            it++;
                            break;
                        }
                    }

                    nrun++;
                    it++;
                }
                break;

            case EJS_INCONSISTENT:
                sout << "<b><blue> WARNING: The job is not in a well defined state.</blue></b>" << endl;
                sout << "<b><blue>          Usage of the --force option is recommended but the success is not guaranteed.</blue></b>" << endl;
                sout << endl;
                if( force ){
                    it++;
                    nrun++;
                } else {
                    it = erase(it);
                }
                break;

            case EJS_FINISHED:
                if( p_job->GetDataOut() == "keep" ){
                    sout << "<b><blue> WARNING: The job was finished but the job data were kept in the working directory.</blue></b>" << endl;
                    sout << "<b><blue>          The success of the pgo command is not guaranteed!</blue></b>" << endl;
                    sout << endl;
                    it++;
                    nrun++;
                } else {
                    sout << "<b><blue> WARNING: The job was finished and synchronized.</blue></b>" << endl;
                    sout << "<b><blue>          The pgo command cannot be used.</blue></b>" << endl;
                    sout << endl;
                    it = erase(it);
                }
                break;

            case EJS_KILLED:
            case EJS_ERROR:
                // do we have start/workdir
                if( ( p_job->GetItem("start/workdir","INF_MAIN_NODE",true) == NULL ) ||
                    ( p_job->GetItem("start/workdir","INF_WORK_DIR",true) == NULL ) ) {
                    sout << "<b><blue> WARNING: The job is not in a well defined state.</blue></b>" << endl;
                    sout << "<b><blue>          The pgo command cannot be used because the information about the job working directory is not known.</blue></b>" << endl;
                    sout << endl;
                    it = erase(it);
                } else {
                    sout << "<b><blue> WARNING: The job is not in a well defined state.</blue></b>" << endl;
                    sout << "<b><blue>          Usage of the --force option is recommended but the success is not guaranteed.</blue></b>" << endl;
                    sout << endl;
                    if( force ){
                        it++;
                        nrun++;
                    } else {
                        it = erase(it);
                    }
                }
                break;
        }
    }

    if( size() == 0 ){
        sout << "<b><red> ERROR: No job is suitable for the pgo command!</red></b>" << endl;
        sout << endl;
        return(false);
    }

    if( size() > 1 ){
        sout << "<b><red> ERROR: The pgo command cannot be applied to more than one job!</red></b>" << endl;
        sout << "<b><red>        Specify the required job as the argument of the pgo command.</red></b>" << endl;
        sout << endl;
        return(false);
    }

    return( nrun == 1 );
}

//------------------------------------------------------------------------------

bool CJobList::WaitForRunningJob(std::ostream& sout)
{
    if( size() != 1 ) return(false); // only one job is expected

    CJobPtr p_job = front();

    switch( p_job->GetJobStatus() ){
        case EJS_SUBMITTED:
        case EJS_BOOTING:
        case EJS_RUNNING:
            break;
        default:
            return(false);
    }

    int count = 0;
    for(;;){
        if( p_job->IsInteractiveJob() ){
            sout << "Awaiting for the job terminal ... ";
            if( p_job->IsTerminalReady() ) break;
        } else {
            sout << "Awaiting for the job execution ... ";
            if( p_job->GetJobInfoStatus() == EJS_RUNNING ) break;
        }
        switch(count % 8){
            case 0: sout << "|\r";
                break;
            case 1: sout << "/\r";
                break;
            case 2: sout << "-\r";
                break;
            case 3: sout << "\\\r";
                break;
            case 4: sout << "|\r";
                break;
            case 5: sout << "/\r";
                break;
            case 6: sout << "-\r";
                break;
            case 7: sout << "\\\r";
                break;
        }
        sout.flush();
        sleep(1);
        p_job->LoadInfoFile();
        count++;
    }
    sout << endl;
    p_job->UpdateJobStatus();
    sout << endl;

    return(true);
}

//------------------------------------------------------------------------------

bool CJobList::IsSyncActionPossible(std::ostream& sout)
{
    list<CJobPtr>::iterator it = begin();
    list<CJobPtr>::iterator ie = end();

    while( it != ie ){
        CJobPtr p_job = *it;

        (*it)->PrintJobInfo(sout);
        sout << std::endl;

        switch( p_job->GetJobStatus() ){
            case EJS_MOVED:
                sout << "<b><blue> WARNING: The job was moved into the other destination.</blue></b>" << endl;
                sout << "<b><blue>          The psync command cannot be used.</blue></b>" << endl;
                sout << endl;
                it = erase(it);
                break;
            case EJS_NONE:
            case EJS_PREPARED:
            case EJS_SUBMITTED:
                sout << "<b><blue> WARNING: The job was not started therefore it does not have the working directory.</blue></b>" << endl;
                sout << "<b><blue>          The psync command cannot be used.</blue></b>" << endl;
                sout << endl;
                it = erase(it);
                break;
            case EJS_BOOTING:
                sout << "<b><blue> WARNING: The job is booting.</blue></b>" << endl;
                sout << "<b><blue>          The psync command cannot be used.</blue></b>" << endl;
                sout << endl;
                it = erase(it);
                break;
            case EJS_RUNNING:
                if( p_job->GetWorkDirType() == "jobdir" ){
                    sout << "<b><blue> WARNING: The input directory and working directories are the same (workdir=jobdir).</blue></b>" << endl;
                    sout << endl;
                    it = erase(it);
                } else {
                    it++;
                }
                break;
            case EJS_FINISHED:
                if( p_job->GetDataOut() == "keep" ){
                    sout << "<b><blue> WARNING: The job was finished but the job data were left on the working directory (dataout=keep).</blue></b>" << endl;
                    sout << "<b><blue>          The success of the psync command is not guaranteed.</blue></b>" << endl;
                    sout << endl;
                    it++;
                } else {
                    sout << "<b><blue> WARNING: The job was finished and synchronized.</blue></b>" << endl;
                    sout << "<b><blue>          The psync command cannot be used.</blue></b>" << endl;
                    sout << endl;
                    it = erase(it);
                }
                break;
            case EJS_KILLED:
            case EJS_ERROR:
            case EJS_INCONSISTENT: {

                // do we have start/workdir
                if( ( p_job->GetItem("start/workdir","INF_MAIN_NODE",true) == NULL ) ||
                    ( p_job->GetItem("start/workdir","INF_WORK_DIR",true) == NULL ) ) {
                    sout << "<b><blue> WARNING: The job is not in a well defined state.</blue></b>" << endl;
                    sout << "<b><blue>          The psync command cannot be used because the information about the job working directory is not known.</blue></b>" << endl;
                    sout << endl;
                    it = erase(it);
                } else {
                    sout << "<b><blue> WARNING: The job is not in a well defined state.</blue></b>" << endl;
                    sout << "<b><blue>          The success of the psync command is not guaranteed.</blue></b>" << endl;
                    sout << endl;
                    it++;
                }
                }
                break;
        }
    }

    if( size() == 0 ){
        sout << "<b><red>ERROR: No job is suitable for the psync command!</red></b>" << endl;
        sout << endl;
        return(false);
    }

    if( size() > 1 ){
        sout << "<b><red>ERROR: The psync command cannot be applied to more than one job!</red></b>" << endl;
        sout << "<b><red>       Specify the required job as the argument of the psync command.</red></b>" << endl;
        sout << endl;
        return(false);
    }

    return(true);
}

//------------------------------------------------------------------------------

bool CJobList::PrepareGoWorkingDirEnv(std::ostream& sout,bool noterm)
{
    if( size() != 1 ) return(false);

    CJobPtr p_job = front();

    CSmallString job_key = CShell::GetSystemVariable("INF_JOB_KEY");
    if( p_job->IsInteractiveJob() && job_key == p_job->GetJobKey() ){
        sout << "<b><red> ERROR: pgo cannot be used once you are logged to the job terminal!</red></b>" << endl;
        sout << endl;
        return(false);
    }

    if( p_job->PrepareGoWorkingDirEnv(noterm) == false ){
        sout << "<b><red> ERROR: Unable to prepare environment for the pgo command as some items are missing in the info file!</red></b>" << endl;
        sout << endl;
        return(false);
    }
    return(true);
}

//------------------------------------------------------------------------------

bool CJobList::PrepareGoInputDirEnv(std::ostream& sout)
{
    if( size() != 1 ) return(false);

    CJobPtr p_job = front();

    if( p_job->PrepareGoInputDirEnv() == false ){
        sout << "<b><red>ERROR: Unable to prepare environment for the pgo command as some items are missing in the info file!</red></b>" << endl;
        sout << endl;
        return(false);
    }

    return(true);
}

//------------------------------------------------------------------------------

bool CJobList::PrepareSyncWorkingDirEnv(std::ostream& sout)
{
    if( size() != 1 ){
        sout << "<b><red>ERROR: None or more than one job is suitable for the psync command!</red></b>" << endl;
        sout << "<b><red>       In the second case, you must explicitly specified the info file of the job!</red></b>" << endl;
        sout << endl;
        return(false);
    }

    CJobPtr p_job = *begin();
    if( p_job->PrepareSyncWorkingDirEnv() == false ){
        sout << "<b><red>ERROR: Unable to prepare environment for the psync command as some items are missing in the info file!</red></b>" << endl;
        sout << endl;
        return(false);
    }
    return(true);
}

//------------------------------------------------------------------------------

bool CJobList::SortByPrepareDateAndTimeA(const CJobPtr& p_left,const CJobPtr& p_right)
{
    CSmallTimeAndDate lt;
    p_left->HasSection("basic",lt);
    CSmallTimeAndDate rt;
    p_right->HasSection("basic",rt);

    if( lt == rt ){
        int n1 = 0;
        stringstream s1(string(p_left->GetJobID()));
        s1 >> n1;
        int n2 = 0;
        stringstream s2(string(p_right->GetJobID()));
        s2 >> n2;
        return( n1 <= n2 );
    }

    return( lt <= rt );
}

//------------------------------------------------------------------------------

void CJobList::SortByPrepareDateAndTime(void)
{
    sort(SortByPrepareDateAndTimeA);
}

//------------------------------------------------------------------------------

bool CJobList::SortByBatchSubmitDateAndTimeA(const CJobPtr& p_left,const CJobPtr& p_right)
{
    CSmallTimeAndDate lt(p_left->GetItem("batch/job","INF_SUBMIT_TIME").ToLInt());
    CSmallTimeAndDate rt(p_right->GetItem("batch/job","INF_SUBMIT_TIME").ToLInt());

    // if from the same batch server then use job id
    if( p_left->ShortServerName == p_right->ShortServerName){
        int n1 = 0;
        stringstream s1(string(p_left->GetJobID()));
        s1 >> n1;
        int n2 = 0;
        stringstream s2(string(p_right->GetJobID()));
        s2 >> n2;
        return( n1 <= n2 );
    }

    return( lt <= rt );
}

//------------------------------------------------------------------------------

void CJobList::SortByBatchSubmitDateAndTime(void)
{
    sort(SortByBatchSubmitDateAndTimeA);
}

//------------------------------------------------------------------------------

bool CJobList::SortByFinishDateAndTimeA(const CJobPtr& p_left,const CJobPtr& p_right)
{
    CSmallTimeAndDate lt(p_left->GetItem("batch/job","INF_FINISH_TIME").ToLInt());
    CSmallTimeAndDate rt(p_right->GetItem("batch/job","INF_FINISH_TIME").ToLInt());
    return( lt <= rt );
}

//------------------------------------------------------------------------------

void CJobList::SortByFinishDateAndTime(void)
{
    sort(SortByFinishDateAndTimeA);
}

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

bool CJobList::TryToFindCollection(CSmallString& collname)
{
    CDirectoryEnum direnum;
    direnum.SetDirName(".");

    direnum.StartFindFile("*");

    CFileName file;
    CFileName fcollname;

    int count = 0;
    while( direnum.FindFile(file) == true ){
        if( file == "." ) continue;
        if( file == ".." ) continue;

        if( fnmatch("*.cofi",file,0) == 0 ) {
            fcollname = file;
            count++;
        }
    }

    direnum.EndFindFile();

    if( count == 1 ){
        collname = fcollname;
        return(true);
    }

    return(false);  // not found or more collection files
}

//------------------------------------------------------------------------------

bool CJobList::CreateCollection(const CSmallString& name,std::ostream& sout)
{
    CFileName cinput = name;
    CFileName cpath = cinput.GetFileDirectory();
    cpath.AbsolutizePath();
    CFileName cname = cinput.GetFileNameWithoutExt();

    CFileName full_name;
    full_name = cpath / cname + ".cofi";

    if( CFileSystem::IsFile(full_name) == true ){
        ES_TRACE_ERROR("collection already exist");
        sout << endl;
        sout << "<b><red> ERROR: The collection file '" << full_name << "' already exists!</red></b>" << endl;
        return(false);
    }

    // does path exist
    if( CFileSystem::IsDirectory(cpath) == false ){
        ES_TRACE_ERROR("specified path does not exist");
        sout << endl;
        sout << "<b><red> ERROR: The specified directory '" << cpath << "' does not exist!</red></b>" << endl;
        return(false);
    }

    CollectionHost = ABSConfig.GetHostName();
    CollectionPath = cpath;
    CollectionName = cname;
    CUUID coll_id;
    coll_id.CreateUUID();
    CollectionID = coll_id.GetStringForm();
    CollectionLastChange.GetActualTimeAndDate();
    CollectionSiteName = AMSGlobalConfig.GetActiveSiteName();
    CollectionSiteID = AMSGlobalConfig.GetActiveSiteID();

    return(true);
}

//------------------------------------------------------------------------------

bool CJobList::LoadCollection(const CSmallString& name,bool fallback_jobs)
{
    CXMLDocument xml_doc;
    CXMLParser   xml_parser;

    CFileName cname(name);

    if( cname.GetFileNameExt() != ".cofi" ) {
        cname += ".cofi";
    }

    // this is temporary
    CollectionName = cname.GetFileNameWithoutExt();

    if( CFileSystem::IsFile(cname) == false ){
        ES_TRACE_ERROR("specified collection file does not exist");
        return(false);
    }

    // get collection path
    CollectionPath = cname.GetFileDirectory();
    if( CollectionPath == NULL ){
        CollectionPath = CJob::GetJobInputPath();
    }

    xml_parser.SetOutputXMLNode(&xml_doc);
    if( xml_parser.Parse(cname) == false ){
        ES_TRACE_ERROR("unable to load collection file");
        return(false);
    }

    CXMLElement* p_hele = xml_doc.GetChildElementByPath("collection/header");
    if( p_hele != NULL ){
        if( LoadCollectionHeader(p_hele) == false ){
            ES_TRACE_ERROR("unable to load collection header");
            return(false);
        }
    } else {
        ES_TRACE_ERROR("header section is not present in the collection");
        return(false);
    }

    CXMLElement* p_jele = xml_doc.GetChildElementByPath("collection/jobs");
    if( p_jele != NULL ){
        if( LoadCollectionJobs(p_jele,fallback_jobs) == false ){
            ES_TRACE_ERROR("unable to load collection jobs");
            return(false);
        }
    } else {
        ES_TRACE_ERROR("jobs section is not present in the collection");
        return(false);
    }

    return(true);
}

//------------------------------------------------------------------------------

bool CJobList::LoadCollection(const CSmallString& path,const CSmallString& name,
                              const CSmallString& id,const CSmallString& sitename,bool fallback_jobs)
{
    CFileName full_name = CFileName(path) / name + ".cofi";
    if( LoadCollection(full_name,fallback_jobs) == false ){
        ES_TRACE_ERROR("unable to load collection");
        return(false);
    }

    // check collection ID
    if( CollectionID != id ){
        CSmallString error;
        error << "loaded collection has different ID (" << CollectionID << ") than requested one (" << id << ")";
        ES_TRACE_ERROR(error);
        return(false);
    }

    // allow to check jobs status under different site
//    if( CollectionSiteName != sitename ){
//        CSmallString error;
//        error << "loaded collection has different site name (" << CollectionSiteName << ") than requested one (" << sitename << ")";
//        ES_TRACE_ERROR(error);
//        return(false);
//    }

    return(true);
}

//------------------------------------------------------------------------------

const CSmallString& CJobList::GetCollectionName(void) const
{
    return(CollectionName);
}

//------------------------------------------------------------------------------

const CSmallString& CJobList::GetCollectionPath(void) const
{
    return(CollectionPath);
}

//------------------------------------------------------------------------------

const CSmallString& CJobList::GetCollectionID(void) const
{
    return(CollectionID);
}

//------------------------------------------------------------------------------

const CSmallString& CJobList::GetCollectionSiteName(void) const
{
    return(CollectionSiteName);
}

//------------------------------------------------------------------------------

bool CJobList::SaveCollection(void)
{
    CFileName full_name;
    full_name = CollectionPath / CollectionName + ".cofi";

    CXMLDocument xml_doc;

    // create collection
    xml_doc.CreateChildDeclaration();
    xml_doc.CreateChildComment(" Infinity collection ");
    CXMLElement* p_rele = xml_doc.CreateChildElement("collection");

    CXMLElement* p_hele = p_rele->CreateChildElement("header");
    SaveCollectionHeader(p_hele);

    CXMLElement* p_jele = p_rele->CreateChildElement("jobs");
    SaveCollectionJobs(p_jele);

    // save xml file
    CXMLPrinter xml_printer;
    xml_printer.SetPrintedXMLNode(&xml_doc);

    if( xml_printer.Print(full_name) == false ){
        CSmallString error;
        error << "unable to save collection file '" << full_name << "'";
        ES_TRACE_ERROR(error);
        return(false);
    }

    return(true);
}

//------------------------------------------------------------------------------

bool CJobList::LoadCollectionHeader(CXMLElement* p_ele)
{
    if( p_ele == NULL ){
        INVALID_ARGUMENT("p_ele == NULL");
    }

    bool result = true;

    result &= p_ele->GetAttribute("time",CollectionLastChange);
    result &= p_ele->GetAttribute("name",CollectionName);
    result &= p_ele->GetAttribute("host",CollectionHost);
    // path is set in LoadCollection
    //result &= p_ele->GetAttribute("path",CollectionPath);
    result &= p_ele->GetAttribute("id",CollectionID);
    result &= p_ele->GetAttribute("site",CollectionSiteName);
    result &= p_ele->GetAttribute("siteid",CollectionSiteID);

    return(result);
}

//------------------------------------------------------------------------------

bool CJobList::LoadCollectionJobs(CXMLElement* p_ele,bool fallback_jobs)
{
    if( p_ele == NULL ){
        INVALID_ARGUMENT("p_ele == NULL");
    }

    fs::path coll_path(CollectionPath);

    CXMLElement* p_jele = p_ele->GetFirstChildElement("job");
    while( p_jele != NULL ){

        CSmallString name;
        CSmallString machine;
        CSmallString spath;

        p_jele->GetAttribute("name",name);
        p_jele->GetAttribute("machine",machine);
        p_jele->GetAttribute("path",spath);

        fs::path job_path(spath);

        // ignore all invalid jobs
        if( (name != NULL) && (machine != NULL) && (spath != NULL) ){
            // resolve job path
            fs::path job_full_path = fs::absolute(job_path,coll_path);

            // this is fallback job
            CJobPtr p_job(new CJob);
            p_job->SetSimpleJobIdentification(name,machine,job_full_path.string());

            // try to locate last valid job info in the job directory
            if( ! fallback_jobs ){
                CJobList jobs;
                if( CFileSystem::IsDirectory(job_full_path.string()) ){
                    jobs.InitByInfoFiles(job_full_path.string().c_str());
                    jobs.SortByPrepareDateAndTime();
                }
                if( jobs.size() >= 1 ){
                    AddJob(jobs.back());
                } else {
                    AddJob(p_job);
                }
            } else {
                AddJob(p_job);
            }
        }

        p_jele = p_jele->GetNextSiblingElement("job");
    }

    return(true);
}

//------------------------------------------------------------------------------

void CJobList::SaveCollectionHeader(CXMLElement* p_ele)
{
    if( p_ele == NULL ){
        INVALID_ARGUMENT("p_ele == NULL");
    }

    // over write time
    CollectionLastChange.GetActualTimeAndDate();

    p_ele->SetAttribute("time",CollectionLastChange);
    p_ele->SetAttribute("name",CollectionName);
    p_ele->SetAttribute("host",CollectionHost);
    p_ele->SetAttribute("path",CollectionPath);
    p_ele->SetAttribute("id",CollectionID);
    p_ele->SetAttribute("site",CollectionSiteName);
    p_ele->SetAttribute("siteid",CollectionSiteID);
}

//------------------------------------------------------------------------------

void CJobList::SaveCollectionJobs(CXMLElement* p_ele)
{
    if( p_ele == NULL ){
        INVALID_ARGUMENT("p_ele == NULL");
    }

    fs::path parentPath(CollectionPath);

    list<CJobPtr>::iterator it = begin();
    list<CJobPtr>::iterator ie = end();

    while( it != ie ){
        CJobPtr p_job = *it;

        CSmallString name;
        CSmallString mach;
        CSmallString path;
        name = p_job->GetItem("basic/jobinput","INF_JOB_NAME",true);
        mach = p_job->GetItem("basic/jobinput","INF_INPUT_MACHINE",true);
        path = p_job->GetItem("basic/jobinput","INF_INPUT_DIR",true);

        // keep only jobs with full data
        if( (name != NULL) && (mach != NULL) && (path != NULL) ){
            CXMLElement* p_jele = p_ele->CreateChildElement("job");

            fs::path childPath(path);
            // cout << childPath.string() << " " << parentPath.string() << endl;

            fs::path relativePath = fs::relative(childPath, parentPath);
            // cout << "r:" << relativePath.string() << endl;

            p_jele->SetAttribute("name",name);
            p_jele->SetAttribute("machine",mach);
            p_jele->SetAttribute("path",relativePath.string());
        }
        it++;
    }
}


//------------------------------------------------------------------------------

bool CJobList::AddJobByPath(const CFileName& path)
{    
    // save .collinfo file
    CFileName coll_info = path / ".collinfo";
    ofstream ofs(coll_info);
    if( ofs ){
        ofs << CollectionID << endl;
        ofs << CollectionPath << endl;
        ofs << CollectionName << endl;
    }
    if( ! ofs ){
        CSmallString error;
        error << "unable to save collection info (" << coll_info << ")";
        ES_ERROR(error);
        return(false);
    }

    CJobList list;
    list.InitByInfoFiles(path,false);
    list.SortByPrepareDateAndTime();
    if( list.GetNumberOfJobs() == 0 ) return(false);
    AddJob(list.back());

    return(true);
}

//------------------------------------------------------------------------------

bool CJobList::AddJobDirContainer(const CFileName& path)
{
    if( CFileSystem::IsDirectory(path) == false ){
        CSmallString error;
        error << "the job directory (" << path << ") does not exist";
        ES_ERROR(error);
        return(false);
    }

    // save .collinfo file
    CFileName coll_info = path / ".collinfo";
    ofstream ofs(coll_info);
    if( ofs ){
        ofs << CollectionID << endl;
        ofs << CollectionPath << endl;
        ofs << CollectionName << endl;
    }
    if( ! ofs ){
        CSmallString error;
        error << "unable to save collection info (" << coll_info << ")";
        ES_ERROR(error);
        return(false);
    }

    CJobPtr p_job = CJobPtr(new CJob);
    p_job->SetSimpleJobIdentification("dirjob",ABSConfig.GetHostName(),path);
    push_back(p_job);

    return(true);
}

//------------------------------------------------------------------------------

void CJobList::AddJob(const CJobPtr& p_job)
{
    // check if job is already a part of collection
    list<CJobPtr>::iterator it = begin();
    list<CJobPtr>::iterator ie = end();

    while( it != ie ){
        CJobPtr p_cjob = *it;
        if( p_cjob->GetInputDir() == p_job->GetInputDir() ){
            // it is part of collection - replace the old object with new one
            (*it) = p_job;
            return;
        } else {
            it++;
        }
    }

    // add job to collection
    push_back(p_job);
}

//------------------------------------------------------------------------------

bool CJobList::RemoveCollectionJob(int cid)
{
    list<CJobPtr>::iterator it = begin();
    list<CJobPtr>::iterator ie = end();

    int icid = 1;
    while( it != ie ){
        if( icid == cid ){
            CJobPtr p_job = *it;
            // try to remove collinfo
            CFileName job_dir = p_job->GetItem("basic/jobinput","INF_INPUT_DIR",true);
            if( CFileSystem::IsDirectory(job_dir) ){
                CFileName coll_info = job_dir / ".collinfo";
                CFileSystem::RemoveFile(coll_info,true);
            }

            // it is part of collection - remove it
            it = erase(it);
            return(true);
        } else {
            it++;
        }
        icid++;
    }
    return(false);
}

//------------------------------------------------------------------------------

void CJobList::PrintCollectionInfo(std::ostream& sout,bool includepath,bool includecomment)
{
    sout << endl;
    sout << "# Collection name : " << CollectionName << endl;
    sout << "# Collection path : " << CollectionHost << ":" << CollectionPath << endl;
    sout << "# Collection site : " << CollectionSiteName;
    if( AMSGlobalConfig.GetActiveSiteName() != CollectionSiteName ){
        sout << " <red>(Active site: " << AMSGlobalConfig.GetActiveSiteName() << ")</red>";
    }
    sout << endl;
    sout << "# Collection ID   : " << CollectionID << endl;
    // sout << "# Last change     : " << CollectionLastChange.GetSDateAndTime() << endl;

    if( size() == 0 ) return;

    sout << endl;
    sout << "# CID  ST    Job Name          Queue      NCPUs NGPUs NNods Last change/Duration  Metrics  Comp" << endl;
    sout << "# ---- -- --------------- --------------- ----- ----- ----- -------------------- --------- ----" << endl;

    list<CJobPtr>::iterator it = begin();
    list<CJobPtr>::iterator ie = end();

    int total = 0;
    int total_jobs = size();
    int processing = 0;
    int waiting = 0;
    int finished = 0;
    int resubmit = 0;
    int inconsistent = 0;

    int i=1;
    while( it != ie ){
        sout << right << setw(6) << i << " ";
        CJobPtr p_job = *it;
        p_job->PrintJobInfoForCollection(sout,includepath,includecomment);

        int totrecycle = p_job->GetNumberOfRecycleJobs();
        int currecycle = p_job->GetCurrentRecycleJob();

        total += totrecycle;
        if( currecycle - 1 > 0 ){
            finished += currecycle - 1;
        }

        switch( p_job->GetJobStatus() ){
            case EJS_NONE:
            case EJS_PREPARED:
                resubmit++;
                break;
            case EJS_SUBMITTED:
                waiting++;
                break;
            case EJS_RUNNING:
            case EJS_BOOTING:
                processing++;
                break;
            case EJS_FINISHED:
                if( p_job->GetJobExitCode() != 0 ){
                    resubmit++;
                } else {
                    if( currecycle < totrecycle ){
                        resubmit++;
                    } else {
                        finished++;
                    }
                }
                break;
            case EJS_KILLED:
                finished++;
                break;
            case EJS_ERROR:
                resubmit++;
                break;
            case EJS_INCONSISTENT:
            case EJS_MOVED:         // this should not technically happen
                inconsistent++;
                break;
        }

        it++;
        i++;
    }

    sout << "# ---------------------------------------------------------------------------------------------" << endl;
//# Waiting: 502 ( 51%) | Processing:   0 (  0%) | Finished: 478 ( 48%) | Total: 980
//# Requires (re)submission:  49 (  5%)

    int wper = 0;
    if( total > 0 ) wper = 100*waiting / total_jobs;
    sout << "# Waiting:" << setw(4) << waiting << " (" << setw(3) << wper << "%)";

    int pper = 0;
    if( total > 0 ) pper = 100*processing / total_jobs;
    sout << " | Processing:" << setw(4) << processing << " (" << setw(3) << pper << "%)";

    int fper = 0;
    if( total > 0 ) fper = 100*finished / total;
    sout << " | Finished:" << setw(4) << finished << " (" << setw(3) << fper << "%)";

    sout << " | Total:" << right << setw(4) << total <<  endl;

    int rper = 0;
    if( total > 0 ) rper = 100*resubmit / total_jobs;
    sout << "# ";
    if( resubmit > 0 ) sout << "<b><red>";
    sout << "Requires (re)submission:" << setw(4) << resubmit << " (" << setw(3) << rper << "%)";
    if( resubmit > 0 ) sout << "</red></b>";

    int iper = 0;
    if( total > 0 ) iper = 100*inconsistent / total_jobs;
    sout << " | ";
    if( inconsistent > 0 ) sout << "<b><red>";
    sout << "Inconsistent:" << setw(4) << inconsistent << " (" << setw(3) << iper << "%)";
    if( inconsistent > 0 ) sout << "</red></b>";
    sout << endl;
}

//------------------------------------------------------------------------------

void CJobList::PrintCollectionStat(std::ostream& sout,bool compact,bool includepath,bool includecomment)
{
    CJobList jobs;

    // get all jobs in collection job directories
    list<CJobPtr>::iterator it = begin();
    list<CJobPtr>::iterator ie = end();

    while( it != ie ){
        jobs.InitByInfoFiles((*it)->GetInputDir(),true);
        it++;
    }

    // update theri statuses
    jobs.UpdateJobStatuses();

    sout << endl;
    sout << "# All past and present jobs in the collection ..." << endl;

    // print info about jobs
    if( compact ){
        jobs.PrintInfosCompact(sout,includepath,includecomment);
    } else {
        jobs.PrintInfos(sout);
    }

    // print statistics
    jobs.PrintStatistics(sout);
}

//------------------------------------------------------------------------------

void CJobList::KeepCollectionJobsForResubmit(void)
{
    list<CJobPtr>::iterator it = begin();
    list<CJobPtr>::iterator ie = end();

    while( it != ie ){
        CJobPtr p_job = *it;

        switch( p_job->GetJobStatus() ){
            case EJS_NONE:
            case EJS_PREPARED:
            case EJS_ERROR:
            case EJS_KILLED:
                // keep
                it++;
                break;
            case EJS_FINISHED:{
                    int totrecycle = p_job->GetNumberOfRecycleJobs();
                    int currecycle = p_job->GetCurrentRecycleJob();
                    if( currecycle != totrecycle ){
                        // keep  - all recycle stages are not process
                        it++;
                        break;
                    }
                    if( p_job->GetJobExitCode() != 0 ){
                        // keep  - the job did not finished well
                        it++;
                        break;
                    }
                    // delete
                    it = erase(it);
                }
                break;
            case EJS_SUBMITTED:
            case EJS_RUNNING:
            case EJS_INCONSISTENT:
            default:
                // delete
                it = erase(it);
                break;
        }
    }
}

//------------------------------------------------------------------------------

void CJobList::PrintCollectionResubmitJobs(std::ostream& sout)
{
    sout << endl;
    sout << "# List of jobs that will be resubmitted ..." << endl;

    sout << endl;
    sout << "# CID  ST    Job ID       Job Name     Comment" << endl;
    sout << "# ---- -- ------------ --------------- -----------------------------------------------------" << endl;

    list<CJobPtr>::iterator it = begin();
    list<CJobPtr>::iterator ie = end();

    int i=1;
    while( it != ie ){
        CJobPtr p_job = *it;
        sout << right << setw(6) << i << " ";

        switch( p_job->GetJobStatus() ){
            case EJS_NONE:
                sout << "UN";
                break;
            case EJS_PREPARED:
                sout << "<yellow>P</yellow> ";
                break;
            case EJS_SUBMITTED:
                sout << "<purple>Q</purple> ";
                break;
            case EJS_BOOTING:
                sout << "<green>B</green> ";
                break;
            case EJS_RUNNING:
                sout << "<green>R</green> ";
                break;
            case EJS_FINISHED:
                if( p_job->GetJobExitCode() == 0 ){
                    sout << "F ";
                } else {
                    sout << "<red>FE</red>";
                }
                break;
            case EJS_KILLED:
                sout << "<red>KI</red>";
                break;
            case EJS_ERROR:
                sout << "<red>ER</red>";
                break;
            case EJS_MOVED:
                sout << "<cyan>M</cyan>";
                break;
            case EJS_INCONSISTENT:
                sout << "<red>IN</red>";
                break;
        }

        if( p_job->GetItem("submit/job","INF_JOB_ID",true) != NULL ){
            CSmallString id = p_job->GetItem("submit/job","INF_JOB_ID");
            CSmallString srv = p_job->GetItem("specific/resources","INF_SERVER_SHORT");
            string stmp(id);
            vector<string> items;
            split(items,stmp,is_any_of("."));
            if( items.size() >= 1 ){
                id = items[0];
            }
            id << srv;
            sout << " " << right << setw(12) << id;
        } else {
            sout << "             ";
        }

        CSmallString name = p_job->GetJobName();
        if( name.GetLength() > 15 ){
            name = name.GetSubStringFromTo(0,14);
        }
        sout << " " << setw(15) << name;

        switch( p_job->GetJobStatus() ){
            case EJS_NONE:
            case EJS_PREPARED:
                break;
            case EJS_ERROR:
            case EJS_KILLED:
                sout << " " << p_job->GetJobBatchComment();
                break;
            case EJS_FINISHED:{
                    int totrecycle = p_job->GetNumberOfRecycleJobs();
                    int currecycle = p_job->GetCurrentRecycleJob();
                    if( currecycle != totrecycle ){
                        sout << " " << "recycle batch is not finished (" << currecycle << "/" << totrecycle << ")";
                    }
                    if( p_job->GetJobExitCode() != 0 ){
                        sout << " " << "job finished with error (" << p_job->GetJobExitCode() << ")";
                    }
                }
                break;
            case EJS_SUBMITTED:
            case EJS_BOOTING:
            case EJS_RUNNING:
            case EJS_INCONSISTENT:
            case EJS_MOVED:
                break;
        }
        sout << endl;

        it++;
        i++;
    }
}

//------------------------------------------------------------------------------

bool CJobList::CollectionResubmitJobs(std::ostream& sout,bool verbose)
{
    sout << endl;
    sout << "# CID    Job ID        Job Name     Comment" << endl;
    sout << "# ---- ------------ --------------- -----------------------------------------------------" << endl;

    list<CJobPtr>::iterator it = begin();
    list<CJobPtr>::iterator ie = end();

    bool result = true;

    int i=1;
    while( it != ie ){
        CJobPtr p_job = *it;

        ERetStatus rstat = p_job->ResubmitJob(verbose);
        result &= ! (rstat == ERS_FAILED);
        sout << right << setw(6) << i;

        if( rstat == ERS_OK ){
            if( p_job->GetItem("submit/job","INF_JOB_ID",true) != NULL ){
                CSmallString id = p_job->GetItem("submit/job","INF_JOB_ID");
                CSmallString srv = p_job->GetItem("specific/resources","INF_SERVER_SHORT");
                string stmp(id);
                vector<string> items;
                split(items,stmp,is_any_of("."));
                if( items.size() >= 1 ){
                    id = items[0];
                }
                id << srv;
                sout << " " << right << setw(12) << id;
            } else {
                sout << " -\?\?\?\?\?\?\?\?\?- ";
            }
        } else {
            if( rstat == ERS_TERMINATE ){
                sout << " -nosubjobs- ";
            }
            if( rstat == ERS_FAILED ){
                sout << " -failed-    ";
            }
        }

        CSmallString name = p_job->GetJobTitle(); // use here job title to show recycle stage
        if( name.GetLength() > 15 ){
            name = name.GetSubStringFromTo(0,14);
        }
        sout << " " << setw(15) << name;

        if( rstat == ERS_FAILED ){
            sout << " >>> Reason: " << p_job->GetLastError();
        }
        // no comment here
        sout << endl;

        it++;
        i++;
    }

    return(result);
}

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

void CJobList::PrintInfos(std::ostream& sout)
{
    list<CJobPtr>::iterator it = begin();
    list<CJobPtr>::iterator ie = end();

    sout << endl;
    while( it != ie ){    
        (*it)->PrintJobInfo(sout);
        it++;
        if( it != ie ) sout << std::endl;
    }
}

//------------------------------------------------------------------------------

void CJobList::PrintInfosCompact(std::ostream& sout,bool includepath,bool includecomment)
{
    sout << endl;
    sout << "# ST    Job ID        User        Job Title         Queue      NCPUs NGPUs NNods          Times           " << endl;
    sout << "# -- ------------ ------------ --------------- --------------- ----- ----- ----- -------------------------" << endl;

    list<CJobPtr>::iterator it = begin();
    list<CJobPtr>::iterator ie = end();

    while( it != ie ){
        (*it)->PrintJobInfoCompact(sout,includepath,includecomment);
        it++;
    }
}

//------------------------------------------------------------------------------

void CJobList::PrintTerminalJobStatus(std::ostream& sout)
{
    if( GetNumberOfJobs() != 1 ) return;

    CJobPtr p_job = front();
    sout << "Infinity CLI job ID: " << p_job->GetJobID();

    CSmallTimeAndDate ctd;
    ctd.GetActualTimeAndDate();
    CSmallTimeAndDate std;
    if( p_job->HasSection("start",std) && (p_job->HasSection("stop") == false) ){
        CSmallString swtd = CShell::GetSystemVariable("INF_MAX_WALLTIME");
        CSmallTime wtd(swtd.ToInt());
        CSmallTime rtime;
        rtime = std - ctd + wtd;
        if( rtime > 0 ) {
            if( rtime < CSmallTime(120) ){
                if( rtime.GetSecondsFromBeginning() % 2 == 0 ){
                    sout << " | <red>Walltime remaining: " << rtime.GetSTimeAndDay() << "</red>";
                }
            } else {
                sout << " | Walltime remaining: " << rtime.GetSTimeAndDay();
            }
        } else {
            sout << " | THE JOB WILL BE TERMINATED SHORTLY";
        }
    }
    sout << " | To detach the screen session, type Ctrl+a d";
    sout << endl;
}

//------------------------------------------------------------------------------

void CJobList::PrintStatistics(std::ostream& sout)
{
    int all = GetNumberOfJobs();
    if( all <= 1 ) return;

    int pre = 0;
    int sub = 0;
    int sub_stat = 0;
    int run = 0;
    int run_stat = 0;
    int fin = 0;
    int oth = 0;

    list<CJobPtr>::iterator it = begin();
    list<CJobPtr>::iterator ie = end();

    CSmallTime qtime,qtimel,qtimeh;
    CSmallTime rtime,rtimel,rtimeh;
    CSmallTime ctime;

    while( it != ie ){
        CJobPtr p_job = *it;

        if( p_job->GetJobStatus() == EJS_PREPARED ){
            pre++;
        }

        if( p_job->GetJobStatus() == EJS_SUBMITTED ){
            sub++;
        }

        if( p_job->GetJobStatus() == EJS_RUNNING ){
            run++;
            sub_stat++;
            CSmallTime qt = p_job->GetQueuedTime();
            qtime += qt;
            if( sub_stat == 1 ){
                qtimel = qt;
                qtimeh = qt;
            }
            if( qtimel > qt ) qtimel = qt;
            if( qtimeh < qt ) qtimeh = qt;
        }

        if( p_job->GetJobStatus() == EJS_FINISHED ){
            fin++;

            sub_stat++;
            CSmallTime qt = p_job->GetQueuedTime();
            qtime += qt;
            if( sub_stat == 1 ){
                qtimel = qt;
                qtimeh = qt;
            }
            if( qtimel > qt ) qtimel = qt;
            if( qtimeh < qt ) qtimeh = qt;

            run_stat++;
            qt = p_job->GetRunningTime();
            rtime += qt;

            ctime += qt*p_job->GetNumOfCPUs();

            if( run_stat == 1 ){
                rtimel = qt;
                rtimeh = qt;
            }
            if( rtimel > qt ) rtimel = qt;
            if( rtimeh < qt ) rtimeh = qt;
        }

        if( p_job->GetJobStatus() == EJS_ERROR ){
            oth++;
        }

        if( p_job->GetJobStatus() == EJS_INCONSISTENT ){
            oth++;
        }

        it++;
    }

    sout << endl;
    sout << "Final statistics" << endl;
    sout << ">>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>><<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<" << endl;
    sout << " Number of all jobs       = " << setw(7) << all << endl;
    sout << " Number of prepared jobs  = " << setw(7) << pre;
    sout << " " << fixed << setw(6) << setprecision(2) <<  (double)pre*100.0/(double)all << " %" << endl;
    sout << " Number of submitted jobs = " << setw(7) << sub;
    sout << " " << fixed << setw(6) << setprecision(2) <<  (double)sub*100.0/(double)all << " %" << endl;
    sout << " Number of running jobs   = " << setw(7) << run;
    sout << " " << fixed << setw(6) << setprecision(2) <<  (double)run*100.0/(double)all << " %" << endl;
    sout << " Number of finished jobs  = " << setw(7) << fin;
    sout << " " << fixed << setw(6) << setprecision(2) <<  (double)fin*100.0/(double)all << " %" << endl;
    sout << " Number of other jobs     = " << setw(7) << oth;
    sout << " " << fixed << setw(6) << setprecision(2) <<  (double)oth*100.0/(double)all << " %" << endl;
    sout << endl;

    sout << " state         min               max              total       number      averaged      " << endl;
    sout << "------- ----------------- ----------------- ----------------- ------- ----------------- " << endl;
    sout << "queued  " << right << setw(17) << qtimel.GetSTimeAndDay();
    sout << " "        << right << setw(17) << qtimeh.GetSTimeAndDay();
    sout << " "        << right << setw(17) << qtime.GetSTimeAndDay();
    sout << " "        << setw(7) << sub_stat;
    CSmallTime aqt;
    if( sub_stat > 0 ){
        aqt = qtime / sub_stat;
    }
    sout << " "        << right << setw(17) << aqt.GetSTimeAndDay() << endl;

    sout << "running " << right << setw(17) << rtimel.GetSTimeAndDay();
    sout << " "        << right << setw(17) << rtimeh.GetSTimeAndDay();
    sout << " "        << right << setw(17) << rtime.GetSTimeAndDay();
    sout << " "        << setw(7) << run_stat;
    CSmallTime art;
    if( run_stat > 0 ){
        art = rtime / run_stat;
    }
    sout << " "        << right << setw(17) << art.GetSTimeAndDay() << endl;
    sout << endl;
    sout << "Total CPU time = " << ctime.GetSTimeAndDay() << " (" << ctime.GetSTimeFull() << ")" << endl;
}

//------------------------------------------------------------------------------

int CJobList::GetNumberOfJobs(void)
{
    return(size());
}

//------------------------------------------------------------------------------

int CJobList::GetNumberOfJobs(EJobStatus status)
{
    list<CJobPtr>::iterator it = begin();
    list<CJobPtr>::iterator ie = end();

    int count = 0;
    while( it != ie ){
        if( (*it)->GetJobStatus() == status ) count++;
        it++;
    }
    return(count);
}

//------------------------------------------------------------------------------

int CJobList::GetNumberOfJobsFromBatchSys(EJobStatus status)
{
    list<CJobPtr>::iterator it = begin();
    list<CJobPtr>::iterator ie = end();

    int count = 0;
    while( it != ie ){
        if( (*it)->GetJobBatchStatus() == status ) count++;
        it++;
    }
    return(count);
}

//------------------------------------------------------------------------------

int CJobList::GetNumberOfOtherJobsFromBatchSys(void)
{
    list<CJobPtr>::iterator it = begin();
    list<CJobPtr>::iterator ie = end();

    int count = 0;
    while( it != ie ){
        if( ((*it)->GetJobBatchStatus() != EJS_SUBMITTED) &&
            ((*it)->GetJobBatchStatus() != EJS_RUNNING) &&
            ((*it)->GetJobBatchStatus() != EJS_FINISHED) ) count++;
        it++;
    }
    return(count);
}

//------------------------------------------------------------------------------

void CJobList::GetNumberOfResFromBatchSys(EJobStatus status,int& ncpus,int& ngpus)
{
    list<CJobPtr>::iterator it = begin();
    list<CJobPtr>::iterator ie = end();

    ncpus = 0;
    ngpus = 0;
    while( it != ie ){
        if( (*it)->GetJobBatchStatus() == status ){
            CSmallString tmp;
            tmp = (*it)->GetItem("specific/resources","INF_NCPUS");
            ncpus += tmp.ToInt();
            tmp = (*it)->GetItem("specific/resources","INF_NGPUS");
            ngpus += tmp.ToInt();
        }
        it++;
    }
}

//------------------------------------------------------------------------------

void CJobList::PrintBatchInfo(std::ostream& sout,bool includepath,bool includecomment,bool includeorigin)
{
    sout << endl;
    sout << "# ST    Job ID        User        Job Title         Queue      NCPUs NGPUs NNods          Times           " << endl;
    sout << "# -- ------------ ------------ --------------- --------------- ----- ----- ----- -------------------------" << endl;

    list<CJobPtr>::iterator it = begin();
    list<CJobPtr>::iterator ie = end();

    while( it != ie ){
        CJobPtr p_job = *it;
        p_job->PrintJobQStatInfo(sout,includepath,includecomment,includeorigin);
        it++;
    }
}

//------------------------------------------------------------------------------

void CJobList::PrintBatchInfoStat(std::ostream& sout)
{
    int ncpus,ngpus;
    GetNumberOfResFromBatchSys(EJS_SUBMITTED,ncpus,ngpus);
    sout << "# Queued:    " << setw(6) << GetNumberOfJobsFromBatchSys(EJS_SUBMITTED);
    sout << " Requested NCPUs: " << setw(6) << ncpus << " NGPUs:  " << setw(6) << ngpus << endl;
    GetNumberOfResFromBatchSys(EJS_RUNNING,ncpus,ngpus);
    sout << "# Running:   " << setw(6) << GetNumberOfJobsFromBatchSys(EJS_RUNNING);
    sout << " Allocated NCPUs: " << setw(6) << ncpus << " NGPUs:  " << setw(6) << ngpus << endl;
    sout << "# Total(QR): " << setw(6) << GetNumberOfJobsFromBatchSys(EJS_SUBMITTED) + GetNumberOfJobsFromBatchSys(EJS_RUNNING);
    sout << "        Finished: " << setw(6) << GetNumberOfJobsFromBatchSys(EJS_FINISHED);
    sout << " Others: " << setw(6) << GetNumberOfOtherJobsFromBatchSys() << endl;
}

//------------------------------------------------------------------------------

void CJobList::PrintLastJobStatus(std::ostream& sout)
{
    CJobPtr job = back();
    if( job != NULL ){
        job->PrintJobStatus(sout);
    }
}

//------------------------------------------------------------------------------

const CJobPtr CJobList::FindJob(CJob* p_job)
{
    list<CJobPtr>::iterator it = begin();
    list<CJobPtr>::iterator ie = end();

    while( it != ie ){
        CJobPtr p_jobptr = *it;
        if( p_jobptr.get() == p_job ) return(p_jobptr);
        it++;
    }

    return(CJobPtr());
}

//------------------------------------------------------------------------------

const CJobPtr CJobList::CreateNewJob(void)
{
    CJobPtr job_ptr = CJobPtr(new CJob);
    push_back(job_ptr);
    return(job_ptr);
}

//------------------------------------------------------------------------------

void CJobList::ArchiveRuntimeFiles(const CSmallString& format)
{
    list<CJobPtr>::iterator it = begin();
    list<CJobPtr>::iterator ie = end();

    while( it != ie ){
        CJobPtr p_jobptr = *it;
        p_jobptr->ArchiveRuntimeFiles(format);
        it++;
    }
}

//------------------------------------------------------------------------------

void CJobList::CleanRuntimeFiles(void)
{
    list<CJobPtr>::iterator it = begin();
    list<CJobPtr>::iterator ie = end();

    while( it != ie ){
        CJobPtr p_jobptr = *it;
        p_jobptr->CleanRuntimeFiles();
        it++;
    }
}

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

