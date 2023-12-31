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

#include <Job.hpp>
#include <XMLIterator.hpp>
#include <ErrorSystem.hpp>
#include <iomanip>
#include <Shell.hpp>
#include <DirectoryEnum.hpp>
#include <FileSystem.hpp>
#include <XMLPrinter.hpp>
#include <NodeList.hpp>
#include <string.h>
#include <ABSConfig.hpp>
#include <XMLParser.hpp>
#include <AliasList.hpp>
#include <Utils.hpp>
#include <ShellProcessor.hpp>
#include <boost/algorithm/string/split.hpp>
#include <boost/algorithm/string/classification.hpp>
#include <boost/algorithm/string/join.hpp>
#include <JobList.hpp>
#include <PluginDatabase.hpp>
#include <PluginObject.hpp>
#include <CategoryUUID.hpp>
#include <JobType.hpp>
#include <iostream>
#include <vector>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <BatchServers.hpp>
#include <ResourceList.hpp>
#include <sys/types.h>
#include <grp.h>
#include <sstream>
#include <Host.hpp>
#include <UserUtils.hpp>
#include <boost/format.hpp>

//------------------------------------------------------------------------------

using namespace std;
using namespace boost;
using namespace boost::algorithm;

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

void CJob::PrintJobInfoCompactV3(std::ostream& sout,bool includepath,bool includecomment)
{

//    sout << "# ST    Job ID        User        Job Title         Queue      NCPUs NGPUs NNods Last change          Exit" << endl;
//    sout << "# -- ------------ ------------ --------------- --------------- ----- ----- ----- -------------------- ----" << endl;

    sout << "  ";
    switch( GetJobStatus() ){
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
            if( GetJobExitCode() == 0 ){
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
    if( GetItem("submit/job","INF_JOB_ID",true) != NULL ){
    CSmallString id = GetItem("submit/job","INF_JOB_ID");
    CSmallString srv = GetItem("specific/resources","INF_SERVER_SHORT");
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

    CSmallString user = GetItem("specific/resources","INF_JOB_OWNER",true);
    if( user.GetLength() > 12 ){
        user = user.GetSubStringFromTo(0,11);
    }
    sout << " " << left << setw(12) << user;

    CSmallString title = GetItem("basic/jobinput","INF_JOB_TITLE");
    if( title.GetLength() > 15 ){
        title = title.GetSubStringFromTo(0,14);
    }
    sout << " " << setw(15) << title;
    CSmallString queue = GetItem("specific/resources","INF_QUEUE");
    if( queue.GetLength() > 15 ){
        queue = queue.GetSubStringFromTo(0,14);
    }
    sout << " " << setw(15) << left << queue;
    sout << right;
    sout << " " << setw(5) << GetItem("specific/resources","INF_NCPUS");
    sout << " " << setw(5) << GetItem("specific/resources","INF_NGPUS");
    sout << " " << setw(5) << GetItem("specific/resources","INF_NNODES");

    CSmallTimeAndDate   last_change;
    CSmallTimeAndDate   curr_time;
    CSmallTime          from_last_change;

    curr_time.GetActualTimeAndDate();
    last_change = GetTimeOfLastChange();

    sout << " " ;
    switch( GetJobStatus() ){
        case EJS_NONE:
        case EJS_INCONSISTENT:
        case EJS_ERROR:
        case EJS_MOVED:
            break;
        case EJS_PREPARED:
        case EJS_SUBMITTED:
        case EJS_BOOTING:
        case EJS_RUNNING:
            from_last_change = curr_time - last_change;
            sout << right << setw(25) << from_last_change.GetSTimeAndDay();
            break;
        case EJS_FINISHED:
        case EJS_KILLED:
            sout << right << setw(25) << last_change.GetSDateAndTime();
            break;
    }

    sout << endl;

    if( includepath ){
        if( IsJobDirLocal() ){
            sout << "                  <blue>> " << GetItem("basic/jobinput","INF_INPUT_DIR") << "</blue>" << endl;
        } else {
            sout << "                  <blue>> " << GetItem("basic/jobinput","INF_INPUT_MACHINE") << ":" << GetItem("basic/jobinput","INF_INPUT_DIR") << "</blue>" << endl;
        }
    }

    if( includecomment ){
        switch( GetJobStatus() ){
            case EJS_NONE:
            case EJS_PREPARED:
            case EJS_FINISHED:
            case EJS_KILLED:
                // nothing to be here
                break;

            case EJS_ERROR:
            case EJS_INCONSISTENT:
                sout << "                  <red>" << GetJobBatchComment() << "</red>" << endl;
                break;
            case EJS_SUBMITTED:
            case EJS_BOOTING:
            case EJS_MOVED:
                sout << "                  <purple>" << GetJobBatchComment() << "</purple>" << endl;
                break;
            case EJS_RUNNING:
                sout << "                  <green>" << GetItem("start/workdir","INF_MAIN_NODE");
                if( GetItem("specific/resources","INF_NNODES").ToInt() > 1 ){
                    sout << ",+";
                }
                sout << "</green>" << endl;
                break;
        }
    }
}

//------------------------------------------------------------------------------

void CJob::PrintJobInfoCompactV2(std::ostream& sout,bool includepath,bool includecomment)
{
//    sout << "# ST       Job ID            Job Title         Queue      NCPUs NGPUs NNods Last change         " << endl;
//    sout << "# -- -------------------- --------------- --------------- ----- ----- ----- --------------------" << endl;

    sout << "  ";
    switch( GetJobStatus() ){
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
        case EJS_RUNNING:
            sout << "<green>R</green> ";
            break;
        case EJS_FINISHED:
            sout << "F ";
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
    if( GetItem("submit/job","INF_JOB_ID",true) != NULL ){
    CSmallString id = GetItem("submit/job","INF_JOB_ID");
    string stmp(id);
    vector<string> items;
    split(items,stmp,is_any_of("."));
    if( items.size() >= 1 ){
        id = items[0];
    }
    if( id.GetLength() > 20 ){
        id = id.GetSubStringFromTo(0,19);
    }
    sout << " " << left << setw(20) << id;
    } else {
    sout << "                     ";
    }
    CSmallString title = GetItem("basic/jobinput","INF_JOB_TITLE");
    sout << " " << setw(15) << title;
    CSmallString queue = GetItem("specific/resources","INF_QUEUE");
    if( queue.GetLength() > 15 ){
        queue = queue.GetSubStringFromTo(0,14);
    }
    sout << " " << setw(15) << left << queue;
    sout << right;
    sout << " " << setw(5) << GetItem("specific/resources","INF_NCPU");
    sout << " " << setw(5) << GetItem("specific/resources","INF_NGPU");
    sout << " " << setw(5) << GetItem("specific/resources","INF_NNODE");

    CSmallTimeAndDate   last_change;
    CSmallTimeAndDate   curr_time;
    CSmallTime          from_last_change;

    curr_time.GetActualTimeAndDate();
    last_change = GetTimeOfLastChange();

    sout << " " ;
    switch( GetJobStatus() ){
        case EJS_NONE:
        case EJS_INCONSISTENT:
        case EJS_ERROR:
        case EJS_MOVED:
            break;
        case EJS_PREPARED:
        case EJS_SUBMITTED:
        case EJS_BOOTING:
        case EJS_RUNNING:
            from_last_change = curr_time - last_change;
            sout << right << setw(20) << from_last_change.GetSTimeAndDay();
            break;
        case EJS_FINISHED:
        case EJS_KILLED:
            sout << right << setw(20) << last_change.GetSDateAndTime();
            break;
    }

    sout << endl;

    if( includepath ){
        if( IsJobDirLocal() ){
            sout << "     <blue>> " << GetItem("basic/jobinput","INF_JOB_PATH") << "</blue>" << endl;
        } else {
            sout << "     <blue>> " << GetItem("basic/jobinput","INF_JOB_MACHINE") << ":" << GetItem("basic/jobinput","INF_JOB_PATH") << "</blue>" << endl;
        }
    }

    if( includecomment ){
        switch( GetJobStatus() ){
            case EJS_NONE:
            case EJS_PREPARED:
            case EJS_FINISHED:
            case EJS_KILLED:
                // nothing to be here
                break;

            case EJS_ERROR:
            case EJS_INCONSISTENT:
                sout << "     <red>" << GetJobBatchComment() << "</red>" << endl;
                break;
            case EJS_SUBMITTED:
            case EJS_BOOTING:
            case EJS_MOVED:
                sout << "     <purple>" << GetJobBatchComment() << "</purple>" << endl;
                break;
            case EJS_RUNNING:
                sout << "     <green>" << GetItem("start/workdir","INF_MAIN_NODE");
                if( GetItem("specific/resources","INF_NNODE").ToInt() > 1 ){
                    sout << ",+";
                }
                sout << "</green>" << endl;
                break;
        }
    }
}

//------------------------------------------------------------------------------

void CJob::PrintJobInfoCompactV1(std::ostream& sout,bool includepath)
{
//    sout << "# ST       Job ID            Job Title         Queue      NCPUs NGPUs NNods Last change         " << endl;
//    sout << "# -- -------------------- --------------- --------------- ----- ----- ----- --------------------" << endl;

    if( includepath ){
        if( IsJobDirLocal() ){
            sout << "<blue>>" << GetItem("basic/jobinput","INF_JOB_PATH") << "</blue>" << endl;
        } else {
            sout << "<blue>>" << GetItem("basic/jobinput","INF_JOB_MACHINE") << ":" << GetItem("basic/jobinput","INF_JOB_PATH") << "</blue>" << endl;
        }
    }

    sout << "  ";
    switch( GetJobStatus() ){
        case EJS_NONE:
            sout << "UN";
            break;
        case EJS_PREPARED:
            sout << "P ";
            break;
        case EJS_SUBMITTED:
            sout << "Q ";
            break;
        case EJS_BOOTING:
            sout << "B ";
            break;
        case EJS_RUNNING:
            sout << "R ";
            break;
        case EJS_FINISHED:
            sout << "F ";
            break;
        case EJS_KILLED:
            sout << "KI";
            break;
        case EJS_ERROR:
            sout << "ER";
            break;
        case EJS_MOVED:
            sout << "M";
            break;
        case EJS_INCONSISTENT:
            sout << "IN";
            break;
    }
    if( GetItem("submit/job","INF_JOB_ID",true) != NULL ){
    CSmallString id = GetItem("submit/job","INF_JOB_ID");
    if( id.GetLength() > 20 ){
        id = id.GetSubStringFromTo(0,19);
    }
    sout << " " << left << setw(20) << id;
    } else {
    sout << "                    ";
    }
    sout << " " << setw(15) << GetItem("basic/jobinput","INF_JOB_TITLE");
    sout << right;
    CSmallString queue = GetItem("specific/resources","INF_QUEUE");
    if( queue.GetLength() > 15 ){
        queue = queue.GetSubStringFromTo(0,14);
    }
    sout << " " << setw(15) << queue;
    sout << " " << setw(5) << GetItem("specific/resources","INF_NCPU");
    int ngpus = 0;
    sout << " " << setw(5) << ngpus;
    sout << " " << setw(5) << GetItem("specific/resources","INF_NUM_OF_NODES");

    CSmallTimeAndDate   last_change;
    CSmallTimeAndDate   curr_time;
    CSmallTime          from_last_change;

    curr_time.GetActualTimeAndDate();
    last_change = GetTimeOfLastChange();

    sout << " " << right << setw(20);
    switch( GetJobStatus() ){
        case EJS_NONE:
        case EJS_INCONSISTENT:
        case EJS_ERROR:
        case EJS_MOVED:
            break;
        case EJS_PREPARED:
        case EJS_SUBMITTED:
        case EJS_BOOTING:
        case EJS_RUNNING:
            from_last_change = curr_time - last_change;
            sout << from_last_change.GetSTimeAndDay();
            break;
        case EJS_FINISHED:
        case EJS_KILLED:
            sout << last_change.GetSDateAndTime();
            break;
    }

    sout << endl;
}

//------------------------------------------------------------------------------

void CJob::PrintJobInfoV3(std::ostream& sout)
{
    PrintBasicV3(sout);
    PrintResourcesV3(sout);

    if( HasSection("start") == true ){
        PrintExecV3(sout);
    }

    CSmallTimeAndDate ctad;
    ctad.GetActualTimeAndDate();

    CSmallTimeAndDate stad;
    if( HasSection("submit",stad) == true ){
        sout << "Job was submitted on " << stad.GetSDateAndTime() << endl;
    } else {
        HasSection("basic",stad);
        sout << "Job was prepared for submission on " << stad.GetSDateAndTime() << endl;
        return;
    }

    bool inconsistent = false;

    CSmallTimeAndDate btad;
    if( HasSection("start",btad) == true ){
        CSmallTime qtime;
        qtime = btad - stad;
        sout << "  and was queued for " << qtime.GetSTimeAndDay() << endl;
        sout << "Job was started on " << btad.GetSDateAndTime() << endl;
    } else {
        if( GetJobBatchStatus() == EJS_SUBMITTED ){
            CSmallTime qtime;
            qtime = ctad - stad;
            sout << "<purple>";
            sout << "  and is queued for " << qtime.GetSTimeAndDay() << endl;
            sout << "  >>> Comment: " << GetJobBatchComment() << endl;
            sout << "</purple>";
            return;
        }
        if( GetJobStatus() == EJS_INCONSISTENT ){
            CSmallTime qtime;
            qtime = ctad - stad;
            sout << "  and is queued for " << qtime.GetSTimeAndDay() << endl;
            inconsistent = true;
        }
    }

    CSmallTimeAndDate etad;
    if( HasSection("stop",etad) == true ){
        CSmallTime qtime;
        qtime = etad - btad;
        sout << "  and was running for " << qtime.GetSTimeAndDay() << endl;
        sout << "Job was finished on " << etad.GetSDateAndTime() << endl;
        return;
    } else {
        if( GetJobStatus() == EJS_RUNNING  ){
            CSmallTime qtime;
            qtime = ctad - btad;
            sout << "<green>";
            sout << "  and is running for " << qtime.GetSTimeAndDay() << endl;
            sout << "</green>";
            return;
        }
        if( GetJobBatchStatus() == EJS_RUNNING  ){
            sout << "<purple>Job was started but details are not known yet" << endl;
            sout << "    >>> Comment: " << GetJobBatchComment() << endl;
            sout << "</purple>";
            return;
        }
        if( (GetJobStatus() == EJS_INCONSISTENT) && (inconsistent == false) ){
            CSmallTime qtime;
            qtime = ctad - btad;
            sout << "  and is running for " << qtime.GetSTimeAndDay() << endl;
        }
    }

    CSmallTimeAndDate ktad;
    if( HasSection("kill",ktad) == true ){
        CSmallTime qtime;
        if( HasSection("start") == true ) {
            qtime = ktad - btad;
            sout << "  and was running for " << qtime.GetSTimeAndDay() << endl;
        } else {
            if( HasSection("submit") == true ) {
                qtime = ktad - stad;
                sout << "  and was queued for " << qtime.GetSTimeAndDay() << endl;
            }
        }
        sout << "Job was killed on " << ktad.GetSDateAndTime() << endl;
        return;
    }

    if( GetJobStatus() == EJS_INCONSISTENT ){
        sout << "<red>>>> Job is in inconsistent state!"<< endl;
        sout <<         "    Comment: " << GetJobBatchComment() << "</red>" << endl;
    }

    if( GetJobStatus() == EJS_ERROR){
        sout << "<red>>>> Job is in error state!"<< endl;
        sout <<         "    Comment: " << GetJobBatchComment() << "</red>" << endl;
    }
}

//------------------------------------------------------------------------------

void CJob::PrintBasicV3(std::ostream& sout)
{
    CSmallString tmp,col;

    sout << "Job name         : " << GetItem("basic/jobinput","INF_JOB_NAME") << endl;
    if( GetItem("submit/job","INF_JOB_ID",true) != NULL ){
    sout << "Job ID           : " << GetItem("submit/job","INF_JOB_ID") << endl;
    }
    sout << "Job title        : " << GetItem("basic/jobinput","INF_JOB_TITLE") << " (Job type: ";
    sout << GetItem("basic/jobinput","INF_JOB_TYPE");
    if( HasSection("basic/recycle") == true ){
        sout << " ";
        sout << format("%03d") % GetItem("basic/recycle","INF_RECYCLE_CURRENT",false).ToInt();
        sout << "/";
        sout << format("%03d") % GetItem("basic/recycle","INF_RECYCLE_STOP",false).ToInt();
    }
    sout << ")" << endl;

    sout << "Job input dir    : " << GetItem("basic/jobinput","INF_INPUT_MACHINE");
    sout << ":" << GetItem("basic/jobinput","INF_INPUT_DIR") << endl;

    sout << "Job key          : " << GetItem("basic/jobinput","INF_JOB_KEY") << endl;

    tmp = GetItem("basic/arguments","INF_OUTPUT_SUFFIX",true);
    if( tmp != NULL ) {
    sout << "Parametric job   : " << tmp << endl;
    }

    col = GetItem("basic/collection","INF_COLLECTION_NAME",true);
    if( col == NULL ) col = "-none-";
    tmp = GetItem("basic/jobinput","INF_JOB_PROJECT");
    if( tmp == NULL ) tmp = "-none-";
    sout << "Job project      : " << tmp << " (Collection: " << col << ")" << endl;

    sout << "========================================================" << endl;
}

//------------------------------------------------------------------------------

void CJob::PrintResourcesV3(std::ostream& sout)
{
    CSmallString tmp,tmp1,tmp2;

    tmp = GetItem("basic/arguments","INF_ARG_DESTINATION");
    sout << "Req destination  : " << tmp << endl;

    tmp = GetItem("basic/arguments","INF_ARG_RESOURCES");
    if( tmp != NULL ){
    PrintResourceTokens(sout,"Req resources    : ",tmp," ");
    } else {
    sout << "Req resources    : -none-" << endl;
    }

    sout << "-----------------------------------------------" << endl;

    tmp = GetItem("specific/resources","INF_SERVER_SHORT");
    sout << "Site spec        : " << GetSiteName() << "/" << GetABSModule() << "/" << GetServerName() << "|" << tmp << endl;
    CSmallString server = GetServerName();

    tmp = GetItem("specific/resources","INF_DEFAULT_RESOURCES");
    if( tmp != NULL ){
    PrintResourceTokens(sout,"Default resources: ",tmp,", ");
    } else {
    sout << "Default resources: -none-" << endl;
    }

    tmp = GetItem("basic/jobinput","INF_JOB_PROJECT_RESOURCES",true);
    if( tmp != NULL ){
    PrintResourceTokens(sout,"Project resources: ",tmp,", ");
    } else {
    sout << "Project resources: -none-" << endl;
    }

    tmp = GetItem("specific/resources","INF_ALIAS");
    if( tmp == NULL ){
    sout << "Alias            : -none-" << endl;
    }
    else{
    sout << "Alias            : " << tmp << endl;
    tmp = GetItem("specific/resources","INF_ALIAS_RESOURCES");
    PrintResourceTokens(sout,"Alias resources  : ",tmp,", ");
    }

    tmp = GetItem("specific/resources","INF_RESOURCES");
    PrintResourceTokens(sout,"All resources    : ",tmp,", ");

    tmp = GetItem("specific/resources","INF_QUEUE");
    sout << "Requested queue  : " << tmp << endl;
    CSmallString queue = tmp;

    sout << "-----------------------------------------------" << endl;
    sout << "NCPUs NGPUs NNodes Memory WorkSize     WallTime" << endl;
    tmp = GetItem("specific/resources","INF_NCPUS");
    sout << setw(5) << tmp;
    sout << " ";
    tmp = GetItem("specific/resources","INF_NGPUS");
    sout << setw(5) << tmp;
    sout << " ";
    tmp = GetItem("specific/resources","INF_NNODES");
    sout << setw(6) << tmp;
    sout << " ";
    tmp = GetItem("specific/resources","INF_MEMORY");
    sout << setw(6) << tmp;
    sout << " ";
    tmp = GetItem("specific/resources","INF_WORK_SIZE");
    sout << setw(8) << tmp;
    sout << " ";
    tmp = GetItem("specific/resources","INF_WALLTIME");
    CSmallTime wtime;
    wtime.SetFromString(tmp);
    sout << setw(12) << wtime.GetSTimeAndDay();
    sout << endl;

    sout << "-----------------------------------------------" << endl;
    tmp = GetItem("basic/jobinput","INF_INPUT_DIR");
    sout << "Input directory  : " << tmp << endl;

    tmp = GetItem("specific/resources","INF_INPUT_PATH_FSTYPE");
    sout << "File system type : " << tmp << endl;

    sout << "Input storage    : " << GetItem("specific/resources","INF_STORAGE_MACHINE") << ":" << GetItem("specific/resources","INF_STORAGE_DIR") << endl;

    sout << "-----------------------------------------------" << endl;

    tmp = GetItem("specific/resources","INF_WORK_DIR_TYPE");
    sout << "Work directory   : " << tmp << endl;

    tmp = GetItem("specific/resources","INF_WORK_SIZE");
    sout << "Work dir size    : " << tmp << endl;

    tmp1 = GetItem("specific/resources","INF_DATAIN");
    tmp2 = GetItem("specific/resources","INF_DATAOUT");
    sout << "Data IN/OUT      : " << tmp1 << "/" << tmp2 << endl;

    tmp = GetItem("basic/jobinput","INF_EXCLUDED_FILES");
    if( tmp == NULL ){
    sout << "Excluded files   : -none-" << endl;
    }
    else{
    sout << "Excluded files   : " << tmp << endl;
    }

    sout << "-----------------------------------------------" << endl;
    sout << "Group namespaces : ";
    sout << GetItem("specific/resources","INF_INPUT_MACHINE_GROUPNS") << " (input machine) | ";

    // refactorization: INF_STORAGE_GROUPNS -> INF_STORAGE_MACHINE_GROUPNS
    tmp = GetItem("specific/resources","INF_STORAGE_MACHINE_GROUPNS",true);
    if( tmp == NULL ){
        tmp = GetItem("specific/resources","INF_STORAGE_GROUPNS");
    }
    sout << tmp << " (storage machine) | ";
    sout << GetItem("specific/resources","INF_BATCH_SERVER_GROUPNS") << " (batch server)" <<  endl;

    // refactorization: INF_USTORAGEGROUP introduced
    tmp = GetItem("specific/resources","INF_USTORAGEGROUP",true);
    if( tmp != NULL ){
        tmp1 = GetItem("specific/resources","INF_STORAGE_MACHINE_REALM");
    sout << "Storage user grp : " <<  tmp << "[@" << tmp1 << "]" << endl;
    }

    // refactorization: INF_UGROUP -> INF_BATCH_UGROUP
    tmp = GetItem("specific/resources","INF_UBATCHGROUP",true);
    if( tmp == NULL ) tmp = "-default server group-";
    sout << "Batch user group : " <<  tmp << endl;

    tmp = GetItem("specific/resources","INF_UMASK");
    sout << "User file mask   : " << tmp << " [" << CUserUtils::GetUMaskPermissions(CUserUtils::GetUMaskMode(tmp)) << "]" <<  endl;

    tmp = GetItem("specific/resources","INF_FIXPERMS",true);
    if( tmp != NULL ){
    sout << "Fix permissions  : " << tmp << endl;
    } else {
    sout << "Fix permissions  : none" << endl;
    }

    sout << "-----------------------------------------------" << endl;

    tmp = GetItem("basic/external","INF_EXTERNAL_START_AFTER");
    if( tmp == NULL ){
    sout << "Start after      : -not defined-" << endl;
    }
    else{
    sout << "Start after      : " << tmp << endl;
    }

    tmp = GetItem("basic/modules","INF_EXPORTED_MODULES");
    if( tmp == NULL ){
    sout << "Exported modules : -none-" << endl;
    }
    else{
    sout << "Exported modules : " << tmp << endl;
    }

    // moved jobs - optional section
    bool result = true;
    CSmallString    rqueue;
    CSmallString    rsrv;
    result &= GetItem("specific/resources","INF_REAL_QUEUE",rqueue,true);
    result &= GetItem("specific/resources","INF_REAL_SERVER",rsrv,true);

    if( result ){
        if( ((rqueue != NULL) && (rqueue != queue)) ||
            ((rsrv != NULL) && (rsrv != server))  ) {
        sout << "-----------------------------------------------" << endl;
        }

        if( (rqueue != NULL) && (rqueue != queue) ){
        sout << "> Real queue     : " << rqueue << endl;
        }
        if( (rsrv != NULL) && (rsrv != server) ){
        sout << "> Real server    : " << rsrv << endl;
        }
    }

    sout << "========================================================" << endl;
}

//------------------------------------------------------------------------------

bool CJob::PrintExecV3(std::ostream& sout)
{
    CSmallString tmp;

    tmp = GetItem("start/workdir","INF_MAIN_NODE");
    sout << "Main node        : " << tmp << endl;

    tmp = GetItem("start/workdir","INF_WORK_DIR");
    sout << "Working directory: " << tmp << endl;

    tmp = GetItem("terminal","INF_AGENT_MODULE",true);
    if( tmp != NULL ){
    sout << "Terminal agent   : " << tmp << endl;
    }

    tmp = GetItem("terminal","INF_VNC_ID",true);
    if( tmp != NULL ){
    sout << "VNC Server ID    : " << tmp << endl;
    }

    tmp = GetItem("stop/result","INF_JOB_EXIT_CODE",true);
    if( tmp != NULL ){
    sout << "Job exit code    : " << tmp << endl;
    }
    sout << "----------------------------------------" << endl;

    ListNodes(sout);

    if( (GetItem("specific/resources","INF_NGPUS").ToInt() > 0) &&
        (GetElementByPath("start/gpus",false) != NULL) ){
        sout << "----------------------------------------" << endl;

        ListGPUNodes(sout);
    }

    sout << "========================================================" << endl;

    return(true);
}

//------------------------------------------------------------------------------

void CJob::PrintJobInfoV2(std::ostream& sout)
{
    PrintBasicV2(sout);
    PrintResourcesV2(sout);

    if( HasSection("start") == true ){
        PrintExecV2(sout);
    }

    CSmallTimeAndDate ctad;
    ctad.GetActualTimeAndDate();

    CSmallTimeAndDate stad;
    if( HasSection("submit",stad) == true ){
        sout << "Job was submitted on " << stad.GetSDateAndTime() << endl;
    } else {
        HasSection("basic",stad);
        sout << "Job was prepared for submission on " << stad.GetSDateAndTime() << endl;
        return;
    }

    bool inconsistent = false;

    CSmallTimeAndDate btad;
    if( HasSection("start",btad) == true ){
        CSmallTime qtime;
        qtime = btad - stad;
        sout << "  and was queued for " << qtime.GetSTimeAndDay() << endl;
        sout << "Job was started on " << btad.GetSDateAndTime() << endl;
    } else {
        if( GetJobBatchStatus() == EJS_SUBMITTED ){
            CSmallTime qtime;
            qtime = ctad - stad;
            sout << "<purple>";
            sout << "  and is queued for " << qtime.GetSTimeAndDay() << endl;
            sout << "  >>> Comment: " << GetJobBatchComment() << endl;
            sout << "</purple>";
            return;
        }
        if( GetJobStatus() == EJS_INCONSISTENT ){
            CSmallTime qtime;
            qtime = ctad - stad;
            sout << "  and is queued for " << qtime.GetSTimeAndDay() << endl;
            inconsistent = true;
        }
    }

    CSmallTimeAndDate etad;
    if( HasSection("stop",etad) == true ){
        CSmallTime qtime;
        qtime = etad - btad;
        sout << "  and was running for " << qtime.GetSTimeAndDay() << endl;
        sout << "Job was finished on " << etad.GetSDateAndTime() << endl;
        return;
    } else {
        if( GetJobStatus() == EJS_RUNNING  ){
            CSmallTime qtime;
            qtime = ctad - btad;
            sout << "<green>";
            sout << "  and is running for " << qtime.GetSTimeAndDay() << endl;
            sout << "</green>";
            return;
        }
        if( GetJobBatchStatus() == EJS_RUNNING  ){
            sout << "<purple>Job was started but details are not known yet" << endl;
            sout << "    >>> Comment: " << GetJobBatchComment() << endl;
            sout << "</purple>";
            return;
        }
        if( (GetJobStatus() == EJS_INCONSISTENT) && (inconsistent == false) ){
            CSmallTime qtime;
            qtime = ctad - btad;
            sout << "  and is running for " << qtime.GetSTimeAndDay() << endl;
        }
    }

    CSmallTimeAndDate ktad;
    if( HasSection("kill",ktad) == true ){
        CSmallTime qtime;
        if( HasSection("start") == true ) {
            qtime = ktad - btad;
            sout << "  and was running for " << qtime.GetSTimeAndDay() << endl;
        } else {
            if( HasSection("submit") == true ) {
                qtime = ktad - stad;
                sout << "  and was queued for " << qtime.GetSTimeAndDay() << endl;
            }
        }
        sout << "Job was killed on " << ktad.GetSDateAndTime() << endl;
        return;
    }

    if( GetJobStatus() == EJS_INCONSISTENT ){
        sout << "<red>>>> Job is in inconsistent state!"<< endl;
        sout <<         "    Comment: " << GetJobBatchComment() << "</red>" << endl;
    }

    if( GetJobStatus() == EJS_ERROR){
        sout << "<red>>>> Job is in error state!"<< endl;
        sout <<         "    Comment: " << GetJobBatchComment() << "</red>" << endl;
    }
}

//------------------------------------------------------------------------------

void CJob::PrintJobInfoV1(std::ostream& sout)
{
    CSmallString tmp,col;

    sout << "Job name         : " << GetItem("basic/jobinput","INF_JOB_NAME") << endl;
    if( GetItem("submit/job","INF_JOB_ID",true) != NULL ){
    sout << "Job ID           : " << GetItem("submit/job","INF_JOB_ID") << endl;
    }
    sout << "Job title        : " << GetItem("basic/jobinput","INF_JOB_TITLE") << " (Job type: ";
    sout << GetItem("basic/jobinput","INF_JOB_TYPE") << ")" << endl;

    sout << "Job directory    : " << GetItem("basic/jobinput","INF_JOB_MACHINE");
    sout << ":" << GetItem("basic/jobinput","INF_JOB_PATH") << endl;

    col = GetItem("basic/collection","INF_COLLECTION_NAME",true);
    if( col == NULL ) col = "-none-";
    tmp = GetItem("basic/jobinput","INF_JOB_PROJECT");
    if( tmp == NULL ) tmp = "-none-";
    sout << "Job project      : " << tmp << " (Collection: " << col << ")" << endl;

    //sout << "Site ID          : " << GetSiteID() << endl;

    sout << "========================================================" << endl;

    tmp = GetItem("specific/resources","INF_ALIAS");
    if( tmp == NULL ){
    sout << "Alias            : -none-" << endl;
    } else {
    sout << "Alias            : " << tmp << endl;
    }

    tmp = GetItem("specific/resources","INF_QUEUE");
    sout << "Queue            : " << tmp << endl;

    sout << "----------------------------------------" << endl;

    tmp = GetItem("specific/resources","INF_NCPU");
    sout << "Number of CPUs   : " << tmp << endl;

    tmp = GetItem("specific/resources","INF_MAX_CPUS_PER_NODE");
    sout << "Max CPUs / node  : " << tmp << endl;

    tmp = GetItem("specific/resources","INF_NUM_OF_NODES");
    if( tmp != NULL ){
    sout << "Number of nodes  : " << tmp << endl;
    }
    tmp = GetItem("specific/resources","INF_PROPERTIES");
    sout << "Properties       : " << tmp << endl;

    tmp = GetItem("specific/resources","INF_RESOURCES");
    sout << "Resources        : " << tmp << endl;

    tmp = GetItem("specific/resources","INF_SYNCMODE");
    sout << "Sync mode        : " << tmp << endl;
    sout << "----------------------------------------" << endl;

    tmp = GetItem("basic/external","INF_EXTERNAL_START_AFTER");
    if( tmp == NULL ){
    sout << "Start after      : -not defined-" << endl;
    }
    else{
    sout << "Start after      : " << tmp << endl;
    }

    tmp = GetItem("basic/modules","INF_EXPORTED_MODULES");
    if( tmp == NULL ){
    sout << "Exported modules : -none-" << endl;
    }
    else{
    sout << "Exported modules : " << tmp << endl;
    }

    tmp = GetItem("basic/jobinput","INF_EXCLUDED_FILES");
    if( tmp == NULL ){
    sout << "Excluded files   : -none-" << endl;
    }
    else{
    sout << "Excluded files   : " << tmp << endl;
    }

    sout << "========================================================" << endl;

    if( HasSection("start") == true ){
        CSmallString tmp;

        tmp = GetItem("start/workdir","INF_MAIN_NODE");
        sout << "Main node        : " << tmp << endl;
        tmp = GetItem("start/workdir","INF_WORK_DIR");
        sout << "Working directory: " << tmp << endl;
        sout << "----------------------------------------" << endl;

        ListNodes(sout);

        sout << "========================================================" << endl;
    }

    CSmallTimeAndDate ctad;
    ctad.GetActualTimeAndDate();

    CSmallTimeAndDate stad;
    if( HasSection("submit",stad) == true ){
        sout << "Job was submitted on " << stad.GetSDateAndTime() << endl;
    } else {
        HasSection("basic",stad);
        sout << "Job was prepared for submission on " << stad.GetSDateAndTime() << endl;
        return;
    }

    CSmallTimeAndDate btad;
    if( HasSection("start",btad) == true ){
        CSmallTime qtime;
        qtime = btad - stad;
        sout << "  and was queued for " << qtime.GetSTimeAndDay() << endl;
        sout << "Job was started on " << btad.GetSDateAndTime() << endl;
    }

    CSmallTimeAndDate etad;
    if( HasSection("stop",etad) == true ){
        CSmallTime qtime;
        qtime = etad - btad;
        sout << "  and was running for " << qtime.GetSTimeAndDay() << endl;
        sout << "Job was finished on " << etad.GetSDateAndTime() << endl;
        return;
    }

    sout << "<b><blue>  >>> This is an old style info file." << endl;
    sout <<          "      The current job status from the batch system was not determined.</blue></b>"<< endl;
}

//------------------------------------------------------------------------------

void CJob::PrintBasicV2(std::ostream& sout)
{
    CSmallString tmp,col;

    sout << "Job name         : " << GetItem("basic/jobinput","INF_JOB_NAME") << endl;
    if( GetItem("submit/job","INF_JOB_ID",true) != NULL ){
    sout << "Job ID           : " << GetItem("submit/job","INF_JOB_ID") << endl;
    }
    sout << "Job title        : " << GetItem("basic/jobinput","INF_JOB_TITLE") << " (Job type: ";
    sout << GetItem("basic/jobinput","INF_JOB_TYPE") << ")" << endl;

    sout << "Job directory    : " << GetItem("basic/jobinput","INF_JOB_MACHINE");
    sout << ":" << GetItem("basic/jobinput","INF_JOB_PATH") << endl;

    col = GetItem("basic/collection","INF_COLLECTION_NAME",true);
    if( col == NULL ) col = "-none-";
    tmp = GetItem("basic/jobinput","INF_JOB_PROJECT");
    if( tmp == NULL ) tmp = "-none-";
    sout << "Job project      : " << tmp << " (Collection: " << col << ")" << endl;

    CSmallString srvname = GetServerNameV2();
    sout << "Site name        : " << GetSiteName() << " (Torque server: " << srvname << ")" << endl;
    sout << "Job key          : " << GetItem("basic/jobinput","INF_JOB_KEY") << endl;

    tmp = GetItem("basic/arguments","INF_OUTPUT_SUFFIX",true);
    if( tmp != NULL ) {
    sout << "Parametric job   : " << tmp << endl;
    }

    sout << "========================================================" << endl;
}

// -----------------------------------------------------------------------------

void CJob::PrintResourcesV2(std::ostream& sout)
{
    CSmallString tmp,tmp1,tmp2;

    tmp = GetItem("basic/arguments","INF_ARG_DESTINATION");
    sout << "Req destination  : " << tmp << endl;

    tmp = GetItem("basic/arguments","INF_ARG_RESOURCES");
    PrintResourceTokens(sout,"Req resources    : ",tmp," ");

    tmp = GetItem("basic/arguments","INF_ARG_SYNC_MODE");
    if( tmp == NULL ){
    sout << "Req sync mode    : -none-" << endl;
    }
    else{
    sout << "Req sync mode    : " << tmp << endl;
    }

    sout << "----------------------------------------" << endl;
    tmp = GetItem("specific/resources","INF_DEFAULT_RESOURCES");
    if( tmp != NULL ) {
        PrintResourceTokens(sout,"Default resources: ",tmp,", ");
    }

    tmp = GetItem("specific/resources","INF_ALIAS");
    if( tmp == NULL ){
    sout << "Alias            : -none-" << endl;
    }
    else{
    sout << "Alias            : " << tmp << endl;
    tmp = GetItem("specific/resources","INF_ALIAS_RESOURCES");
    PrintResourceTokens(sout,"Alias resources  : ",tmp,", ");

    tmp = GetItem("specific/resources","INF_ALIAS_SYNC_MODE");
    sout << "Alias sync mode  : " << tmp << endl;
    }

    tmp = GetItem("specific/resources","INF_QUEUE");
    sout << "Queue            : " << tmp << endl;

    tmp = GetItem("specific/resources","INF_RESOURCES");
    PrintResourceTokens(sout,"All resources    : ",tmp,", ");

    sout << "----------------------------------------" << endl;

    tmp = GetItem("specific/resources","INF_NCPU");
    sout << "Number of CPUs   : " << tmp << endl;

    tmp = GetItem("specific/resources","INF_NGPU");
    sout << "Number of GPUs   : " << tmp << endl;

    tmp = GetItem("specific/resources","INF_MAX_CPUS_PER_NODE");
    sout << "Max CPUs / node  : " << tmp << endl;

    tmp = GetItem("specific/resources","INF_NNODE");
    if( tmp != NULL ){
    sout << "Number of nodes  : " << tmp << endl;
    }

    sout << "----------------------------------------" << endl;

    tmp = GetItem("specific/resources","INF_SURROGATE_MACHINE",true);
    if( tmp == NULL ) tmp = GetItem("basic/jobinput","INF_JOB_MACHINE");
    if( tmp == GetItem("basic/jobinput","INF_JOB_MACHINE") ){
    sout << "Surrogate machine: " << " -- none -- " << endl;
    } else {
    sout << "Surrogate machine: " << tmp << endl;
    }

    tmp = GetItem("specific/resources","INF_SYNC_MODE");
    sout << "Sync mode        : " << tmp << endl;

    tmp = GetItem("specific/resources","INF_SCRATCH_TYPE",true);
    if( tmp == NULL ) tmp = "scratch";
    sout << "Scratch type     : " << tmp << endl;

    CSmallString fs_type = GetItem("specific/resources","INF_FS_TYPE",true);
    if( fs_type == NULL ) fs_type = "consistent";
    sout << "Input FS type    : " << fs_type << endl;

    if( fs_type == "inconsistent" ) sout << "<blue>";

    tmp = GetItem("specific/resources","INF_UGROUP",true);
    tmp1 = GetItem("specific/resources","INF_UGROUP_REALM",true);
    tmp2 = GetItem("specific/resources","INF_UGROUP_ORIG",true);
    if( tmp == NULL ) tmp = "-not specified-";
    sout << "User group       : ";
    if( tmp2 != NULL ){
        sout << tmp2 << " -> ";
    }
    sout << tmp;
    if( tmp1 != NULL ){
        sout << "@" << tmp1 << endl;
    } else {
        sout << endl;
    }

    tmp = GetItem("specific/resources","INF_UMASK",true);
    if( tmp == NULL ){
        tmp = "-not specified-";
    sout << "User file mask   : " << tmp << endl;
    } else {
    sout << "User file mask   : " << tmp << " [" << CUserUtils::GetUMaskPermissions(CUserUtils::GetUMaskMode(tmp)) << "]" << endl;
    }

    if( fs_type == "inconsistent" ) sout << "</blue>";

    tmp = GetItem("basic/jobinput","INF_EXCLUDED_FILES");
    if( tmp == NULL ){
    sout << "Excluded files   : -none-" << endl;
    }
    else{
    sout << "Excluded files   : " << tmp << endl;
    }

    sout << "----------------------------------------" << endl;

    tmp = GetItem("basic/external","INF_EXTERNAL_START_AFTER");
    if( tmp == NULL ){
    sout << "Start after      : -not defined-" << endl;
    }
    else{
    sout << "Start after      : " << tmp << endl;
    }

    tmp = GetItem("basic/modules","INF_EXPORTED_MODULES");
    if( tmp == NULL ){
    sout << "Exported modules : -none-" << endl;
    }
    else{
    sout << "Exported modules : " << tmp << endl;
    }

    sout << "========================================================" << endl;
}


//------------------------------------------------------------------------------

bool CJob::PrintExecV2(std::ostream& sout)
{
    CSmallString tmp;

    tmp = GetItem("start/workdir","INF_MAIN_NODE");
    sout << "Main node        : " << tmp << endl;
    tmp = GetItem("start/workdir","INF_WORK_DIR");
    sout << "Working directory: " << tmp << endl;
    tmp = GetItem("stop/result","INF_JOB_EXIT_CODE",true);
    if( tmp != NULL ){
    sout << "Job exit code    : " << tmp << endl;
    }
    sout << "----------------------------------------" << endl;

    ListNodes(sout);

    if( (GetItem("specific/resources","INF_NGPU").ToInt() > 0) &&
        (GetElementByPath("start/gpus",false) != NULL) ){
        sout << "----------------------------------------" << endl;

        ListGPUNodes(sout);
    }

    sout << "========================================================" << endl;

    return(true);
}

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================



