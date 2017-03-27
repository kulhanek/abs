#ifndef JobH
#define JobH
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
#include <XMLDocument.hpp>
#include <iostream>
#include <QueueList.hpp>
#include <AliasList.hpp>
#include <SmallTimeAndDate.hpp>
#include <boost/shared_ptr.hpp>
#include <map>
#include <JobType.hpp>
#include <FileName.hpp>

// -----------------------------------------------------------------------------

class CXMLElement;

// -----------------------------------------------------------------------------

enum EJobStatus {
    EJS_NONE,
    EJS_PREPARED,
    EJS_SUBMITTED,
    EJS_RUNNING,
    EJS_FINISHED,
    EJS_KILLED,
    EJS_ERROR,
    EJS_INCONSISTENT
};

// -----------------------------------------------------------------------------

class CJob;
typedef boost::shared_ptr<CJob>  CJobPtr;

// -----------------------------------------------------------------------------

/// Infinity job

class ABS_PACKAGE CJob : private CXMLDocument {
public:
// constructor -----------------------------------------------------------------
        CJob(void);

// i/o methods -----------------------------------------------------------------
    /// load info file
    bool LoadInfoFile(const CSmallString& filename);

    /// load info file - data from batch system are used
    bool LoadInfoFile(void);

    /// save info file
    bool SaveInfoFile(void);

    /// save info file including permissions
    bool SaveInfoFileWithPerms(void);

    /// save info file
    bool SaveInfoFile(const CFileName& name);

    /// save job key
    bool SaveJobKey(void);

    /// init job with torque information
    bool Init(struct batch_status* p_job);

// submit methods --------------------------------------------------------------
    /// 0.0 create info file header
    void CreateHeader(void);

    /// 1.0 create basic section
    void CreateBasicSection(void);

    /// 1.1) set external options
    void SetExternalOptions(void);

    /// 1.2) set arguments
    bool SetArguments(const CSmallString& dst,const CSmallString& js,
                      const CSmallString& res,const CSmallString& cm);

    bool SetOutputNumber(std::ostream& sout,int i);

    /// 1.3) check for runtime files
    bool CheckRuntimeFiles(std::ostream& sout,bool ignore);

    /// 1.4) job input
    ERetStatus JobInput(std::ostream& sout);

    /// 2.0) decode resources
    /// decode resources for Torque
    bool DecodeTorqueResources(std::ostream& sout);

    /// decode resources for pstart
    bool DecodeStartResources(std::ostream& sout);

    /// last job check before it is submitted to Torque server
    bool LastJobCheck(std::ostream& sout);

    /// 3.0 should submit the job
    bool ShouldSubmitJob(std::ostream& sout,bool assume_yes);

    /// submit job
    bool SubmitJob(const CJobPtr& self,std::ostream& sout,bool siblings);

    /// resubmit job
    bool ResubmitJob(void);

    /// write submit section
    void WriteSubmitSection(char* p_jobid);

    /// write error section
    void WriteErrorSection(char* p_error);

// execute job -----------------------------------------------------------------
    /// get job info file name from job shell environment
    bool GetJobInfoFileName(CSmallString& info_file_name);

    /// write info about job beginning
    bool WriteStart(void);

    /// write info about job terminal start
    bool WriteTerminalReady(void);

    /// write info about job termination
    bool WriteStop(void);

// job management methods ------------------------------------------------------
    /// kill job
    bool KillJob(bool force=false);

    /// terminate job
    bool TerminateJob(void);

    /// update job status from batch system
    bool UpdateJobStatus(void);

// information methods ---------------------------------------------------------
    /// print information about job
    void PrintJobInfo(std::ostream& sout);

    /// print information about job
    void PrintJobInfoCompact(std::ostream& sout,bool includepath,bool includecomment);

    /// print information about job in the format for collection
    void PrintJobInfoForCollection(std::ostream& sout,bool includepath,bool includecomment);

    /// print information about job
    void PrintJobQStatInfo(std::ostream& sout,bool includepath,bool includecomment);

    /// print job status using abbreviations: P, Q, R, F, IN
    void PrintJobStatus(std::ostream& sout);

    /// print basic info about job
    void PrintBasic(std::ostream& sout);

    /// print resources
    void PrintResources(std::ostream& sout);

    /// print executive part
    bool PrintExec(std::ostream& sout);

    /// get job main script name
    const CFileName GetMainScriptName(void);

    /// get list of external variables
    const CSmallString GetExternalVariables(void);

    /// get job title
    const CSmallString GetJobTitle(void);

    /// get job name
    const CSmallString GetJobName(void);

    /// get job name suffix
    const CSmallString GetJobNameSuffix(void);

    /// get job path
    const CSmallString GetJobPath(void);

    /// get job machine
    const CSmallString GetJobMachine(void);

    /// get job id
    const CSmallString GetJobID(void);

    /// get job key
    const CSmallString GetJobKey(void);

    /// get job sync mode
    const CSmallString GetSyncMode(void);

    /// get job main script stadard output file name
    const CFileName GetInfoutName(void);

    /// get job queue
    const CSmallString GetQueue(void);

    /// get num of CPUs
    int GetNumOfCPUs(void);

    /// get num of GPUs
    int GetNumOfGPUs(void);

    /// get num of nodes
    int GetNumOfNodes(void);

    /// get job status from info file
    EJobStatus GetJobInfoStatus(void);

    /// get job status fIsInteractiveJobrom batch system
    EJobStatus GetJobBatchStatus(void);

    /// get job status from batch system
    const CSmallString GetJobBatchStatusString(void);

    /// determine final job status
    EJobStatus GetJobStatus(void);

    /// get job batch comment
    const CSmallString GetJobBatchComment(void);

    /// return name of batch owner
    const CSmallString GetBatchUserOwner(void);

    /// is section defined?
    bool HasSection(const CSmallString& section);

    /// is section defined?
    bool HasSection(const CSmallString& section,CSmallTimeAndDate& tad);

    /// get job configuration item
    const CSmallString GetItem(const CSmallString& path,
                               const CSmallString& name,bool noerror=false);

    /// get job configuration item
    bool GetItem(const CSmallString& path,
                 const CSmallString& name,
                 CSmallString& value,bool noerror=false);

    /// set job configuration item
    void SetItem(const CSmallString& path,
                 const CSmallString& name,
                 const CSmallString& value);

    /// get job configuration item
    bool GetTime(const CSmallString& path,
                 const CSmallString& name,
                 CSmallTime& value,bool noerror=false);

    /// set job configuration item
    void SetTime(const CSmallString& path,
                 const CSmallString& name,
                 const CSmallTime& value);

    /// restore job env
    void RestoreEnv(void);

    /// prepare environment for pgo
    void PrepareGoWorkingDirEnv(bool noterm);

    /// prepare environment for pgo
    void PrepareGoInputDirEnv(void);

    /// prepare environment for psync
    void PrepareSyncWorkingDirEnv(void);

    /// prepare environment for soft kill
    void PrepareSoftKillEnv(void);

    /// restore job env IsInteractiveJobfor given element
    void RestoreEnv(CXMLElement* p_ele);

    /// get job last error
    const CSmallString GetLastError(void);

    /// get info file version
    CSmallString GetInfoFileVersion(void);

    /// get site name
    const CSmallString GetSiteName(void);

    /// get site id
    const CSmallString GetSiteID(void);

    /// get server nameIsInteractiveJob
    const CSmallString GetServerName(void);

    /// get queued time
    const CSmallTime GetQueuedTime(void);

    /// get running time
    const CSmallTime GetRunningTime(void);

    /// get time of last change
    const CSmallTimeAndDate GetTimeOfLastChange(void);

    /// get number of CPUs
    int GetNCPU(void);

    /// get umask
    const CSmallString GetUMask(void);

    /// get umask number
    mode_t GetUMaskNumber(void);

    /// get umask bits
    const CSmallString GetUMaskPermissions(void);

    /// get user group
    const CSmallString GetUserGroup(void);

    /// get user group with realm
    const CSmallString GetUserGroupWithRealm(void);

    /// is job dir local
    bool IsJobDirLocal(bool no_deep=false);

    /// get surrogate machine
    const CSmallString GetSurrogateMachine(void);

    /// set simple job identification
    void SetSimpleJobIdentification(const CSmallString& name, const CSmallString& machine,
                                    const CSmallString& path);

    /// get number of jobs in recycle mode or 1 if general job
    int GetNumberOfRecycleJobs(void);

    /// get current recycle job or 1 if general job
    int GetCurrentOfRecycleJob(void);

    /// is info file loaded
    bool IsInfoFileLoaded(void);

    /// register ijob
    void RegisterIJob(const CSmallString& dir, const CSmallString& name, int ncpus);

    /// get number of ijobs
    int GetNumberOfIJobs(void);

    /// get ijob property - id counted from 1
    bool GetIJobProperty(int id,const CSmallString& name, CSmallString& value);

    /// set ijob property - id counted from 1
    void SetIJobProperty(int id,const CSmallString& name,const CSmallString& value);

    /// detect job type
    ERetStatus DetectJobType(std::ostream& sout,CSmallString job_type=CSmallString());

    /// is job interactive
    bool IsInteractiveJob(void);

    /// is job interactive terminal ready?
    bool IsTerminalReady(void);

    /// get job input directory - either PWD or CurrentDir
    static CFileName GetJobInputPath(void);

// section of private data -----------------------------------------------------
protected:
    bool        InfoFileLoaded;

    /// list nodes
    bool ListNodes(std::ostream& sout);

    /// list gpu nodes
    bool ListGPUNodes(std::ostream& sout);

    /// create section
    bool CreateSection(const CSmallString& section);

    /// destroy section
    void DestroySection(const CSmallString& section);

    /// get element by path
    CXMLElement* GetElementByPath(const CSmallString& p_path,bool create);

    /// find job configuration item
    CXMLElement* FindItem(CXMLElement* p_root,const char* p_name);


    /// set item from shell environment
    bool WriteInfo(const CSmallString& category,const CSmallString& name,
                   bool error_on_empty_value);

    /// check job name
    bool CheckJobName(std::ostream& sout);

    /// check job path
    const CSmallString JobPathCheck(const CSmallString& inpath,std::ostream& sout);

    /// detect job project
    void DetectJobProject(void);

    /// detect job project
    CSmallString GetJobProject(const CFileName& dir);

    /// detect job collection
    void DetectJobCollection(void);

    /// get full job script name
    const CFileName GetFullJobName(void);

    /// get item form job file
    bool GetJobFileConfigItem(const CSmallString& key,CSmallString& value);

    /// print information about job
    void PrintJobInfoNew(std::ostream& sout);

    /// print information about job
    void PrintJobInfoOld(std::ostream& sout);

    /// print information about job
    void PrintJobInfoCompactNew(std::ostream& sout,bool includepath,bool includecomment);

    /// print information about job
    void PrintJobInfoCompactOld(std::ostream& sout,bool includepath);

    bool                                DoNotSave;

    // job status --------------------------------------------------------------
public:
    CSmallString                        BatchServerName;
    EJobStatus                          BatchJobStatus;
    CSmallString                        BatchJobComment;
    std::map<std::string,std::string>   BatchVariables;
};

// -----------------------------------------------------------------------------

#endif
