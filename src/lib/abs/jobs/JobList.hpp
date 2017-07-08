#ifndef JobListH
#define JobListH
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

#include <ABSMainHeader.hpp>
#include <Job.hpp>
#include <FileName.hpp>
#include <iostream>

// -----------------------------------------------------------------------------

/// list of jobs

class ABS_PACKAGE CJobList : public std::list<CJobPtr> {
public:
// constructor -----------------------------------------------------------------
    CJobList(void);

// populate list ---------------------------------------------------------------
    /// scan for all info files in the current directory
    void InitByInfoFiles(const CFileName& dir,bool recursive=false);

    /// load info file of running job
    void InitFromRunningJob(void);

    /// add job using its info file
    bool AddJob(const CFileName& info_name);

// collection ------------------------------------------------------------------
    /// try to locate collection file
    static bool TryToFindCollection(CSmallString& collname);

    /// create collection
    bool CreateCollection(const CSmallString& name,std::ostream& sout);

    /// load collection
    bool LoadCollection(const CSmallString& name);

    /// load collection
    bool LoadCollection(const CSmallString& path,const CSmallString& name,
                        const CSmallString& id,const CSmallString& sitename);

    /// get collection name
    const CSmallString& GetCollectionName(void) const;

    /// get collection path
    const CSmallString& GetCollectionPath(void) const;

    /// get collection id
    const CSmallString& GetCollectionID(void) const;

    /// get collection site name
    const CSmallString& GetCollectionSiteName(void) const;

    /// save collection
    bool SaveCollection(void);

    /// add job into collection
    void AddJob(const CJobPtr& p_job);

    /// print info about collection jobs
    void PrintCollectionInfo(std::ostream& sout,bool includepath,bool includecomment);

    /// generate collection statistics
    void PrintCollectionStat(std::ostream& sout,bool compact,bool includepath,bool includecomment);

    /// print jobs that need to be resubmitted
    void PrintCollectionResubmitJobs(std::ostream& sout);

    /// return number of jobs to be resubmitted
    int NumOfJobsToBeResubmitted(void);

    /// resubmit jobs
    bool CollectionResubmitJobs(std::ostream& sout);

    /// print batch jobs
    void PrintBatchInfo(std::ostream& sout,bool includepath,bool includecomment);

    /// print batch jobs
    void PrintBatchInfoStat(std::ostream& sout);

    /// print last job status
    void PrintLastJobStatus(std::ostream& sout);

// executive methods -----------------------------------------------------------
    /// contact batch system manager and update job statuses
    void UpdateJobStatuses(void);

    /// keep only live jobs (running or submitted)
    void KeepOnlyLiveJobs(bool submitted=true);

    /// keep only jobs of given user
    void KeepUserJobs(const CSmallString& name);

    /// keep only completed jobs
    void KeepOnlyCompletedJobs(void);

    /// keep jobs by mask
    void KeepJobsByMask(struct SExpression* p_mask);

    /// kill all jobs
    bool KillAllJobs(void);

    /// kill job softly
    bool KillJobSoftly(std::ostream& sout);

    /// kill all jobs with info
    bool KillAllJobsWithInfo(std::ostream& sout,bool force);

    /// load all info files
    bool LoadAllInfoFiles(void);

    /// save info all files
    bool SaveAllInfoFiles(void);

    /// is pgo action allowed or possible
    bool IsGoActionPossible(std::ostream& sout);

    /// wait until one job is running
    bool WaitForRunningJob(std::ostream& sout);

    /// is psync action allowed or possible
    bool IsSyncActionPossible(std::ostream& sout);

    /// prepare environment for pgo
    bool PrepareGoWorkingDirEnv(std::ostream& sout,bool noterm);

    /// prepare environment for pgo
    bool PrepareGoInputDirEnv(void);

    /// prepare environment for psync
    bool PrepareSyncWorkingDirEnv(void);

    /// sort by prepare date
    void SortByPrepareDateAndTime(void);

    /// sort by submit date from batch manager
    void SortByBatchSubmitDateAndTime(void);

    /// sort by prepare date
    void SortByFinishDateAndTime(void);

    /// create new job
    const CJobPtr CreateNewJob(void);

    /// find job
    const CJobPtr FindJob(CJob* p_job);

// information methods ---------------------------------------------------------
    /// print info about all jobs
    void PrintInfos(std::ostream& sout);

    /// print info about all jobs
    void PrintInfosCompact(std::ostream& sout,bool includepath,bool includecomment);

    /// print statistics about all jobs
    void PrintStatistics(std::ostream& sout);

    /// return number of jobs in the list
    int GetNumberOfJobs(void);

    /// return number of jobs in the list
    int GetNumberOfJobs(EJobStatus status);

    /// return number of jobs in the list
    int GetNumberOfJobsFromBatchSys(EJobStatus status);

    /// not submitted, running, completed
    int GetNumberOfOtherJobsFromBatchSys(void);

    /// not submitted, running, completed
    void GetNumberOfResFromBatchSys(EJobStatus status,int& ncpus,int& ngpus);

    /// print short info for terminal caption
    void PrintTerminalJobStatus(std::ostream& sout);

// section of private data -----------------------------------------------------
private:
    CSmallString        CollectionSiteName;
    CSmallString        CollectionSiteID;
    CSmallString        CollectionHost;
    CFileName           CollectionPath;
    CFileName           CollectionName;
    CSmallString        CollectionID;
    CSmallTimeAndDate   CollectionLastChange;

    static bool SortByPrepareDateAndTimeA(const CJobPtr& p_left,const CJobPtr& p_right);

    static bool SortByFinishDateAndTimeA(const CJobPtr& p_left,const CJobPtr& p_right);

    static bool SortByBatchSubmitDateAndTimeA(const CJobPtr& p_left,const CJobPtr& p_right);

    /// load collection header
    bool LoadCollectionHeader(CXMLElement* p_ele);

    /// load collection jobs
    bool LoadCollectionJobs(CXMLElement* p_ele);

    /// save collection header
    void SaveCollectionHeader(CXMLElement* p_ele);

    /// save collection jobs
    void SaveCollectionJobs(CXMLElement* p_ele);

    // helper methods
    bool IsJobSelected(CJobPtr p_job,struct SExpression* p_expr);
    bool IsJobSelected(CJobPtr p_job,struct SSelection* p_sel);
};

// -----------------------------------------------------------------------------

extern CJobList JobList;

// -----------------------------------------------------------------------------

#endif
