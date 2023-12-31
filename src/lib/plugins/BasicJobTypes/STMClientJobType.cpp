// =============================================================================
//  ABS - Advanced Batch System
// -----------------------------------------------------------------------------
//    Copyright (C) 2013 Petr Kulhanek (kulhanek@chemi.muni.cz)
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

#include "STMClientJobType.hpp"
#include "BasicJobTypesModule.hpp"
#include <CategoryUUID.hpp>
#include <FileName.hpp>
#include <Job.hpp>
#include <FileSystem.hpp>
#include <PluginDatabase.hpp>
#include <ErrorSystem.hpp>
#include <boost/format.hpp>
#include <FileName.hpp>
#include <iomanip>
#include <Utils.hpp>
#include <DirectoryEnum.hpp>
#include "STMClientJobTypeOptions.hpp"
#include <boost/algorithm/string/split.hpp>
#include <boost/algorithm/string/classification.hpp>

//------------------------------------------------------------------------------

using namespace std;
using namespace boost;
using namespace boost::algorithm;

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

CComObject* STMClientJobTypeCB(void* p_data);

CExtUUID        STMClientJobTypeID(
                    "{STM_CLIENT_JOB_TYPE:beaa2a32-bd5f-44c3-8249-524a02ca0cc5}",
                    "STM Client Job Type",
                    "support for fully asynchronous STM server/clients execution");

CPluginObject   STMClientJobTypeObject(&BasicJobTypesPlugin,
                    STMClientJobTypeID,JOB_TYPE_CAT,
                    STMClientJobTypeCB);

// -----------------------------------------------------------------------------

CComObject* STMClientJobTypeCB(void* p_data)
{
    CComObject* p_object = new CSTMClientJobType();
    return(p_object);
}

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

CSTMClientJobType::CSTMClientJobType(void)
    : CJobType(&STMClientJobTypeObject)
{
}

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

ERetStatus CSTMClientJobType::DetectJobType(CJob& job,bool& detected,std::ostream& sout)
{
    detected = false;

    CFileName job_file = job.GetItem("basic/arguments","INF_ARG_JOB");

    CSmallString job_type;
    if( PluginDatabase.FindObjectConfigValue(STMClientJobTypeID,"_type",job_type) == false ){
        ES_ERROR("_type key is not provided");
        return(ERS_FAILED);
    }

    // job file should be real file
    if( CFileSystem::IsFile(job_file) == false ){
        ES_ERROR("the job file is not a file");
        return(ERS_OK);
    }

    // cache file
    if( CacheJobFile(job_file) == false ){
        ES_ERROR("unable to cache job input file");
        return(ERS_OK);
    }

    sout << endl;
    sout << "# ------------------------------------------------" << endl;
    sout << "# stm-client job summary" << endl;
    sout << "# ------------------------------------------------" << endl;
    sout << "# Master job script name      : " << job_file << endl;

    string storage;
    storage = GetKeyValue("storage");
    if( storage.empty() ){
        storage = "storage";
    }
    sout << "# Storage directory (storage) : " << storage << " (it is excluded)" << endl;
    if( CFileSystem::IsDirectory(storage) == false ){
        if( CFileSystem::IsFile(storage) == true ){
            sout << endl;
            sout << "<b><red> ERROR: Unable to create storage directory '" << storage << "' (it is already file)!</red></b>" << endl;
            return(ERS_FAILED);
        }
        if( CFileSystem::CreateDir(storage) == false ){
            sout << endl;
            sout << "<b><red> ERROR: Unable to create storage directory '" << storage << "'!</red></b>" << endl;
            return(ERS_FAILED);
        }
    }
    int nkeys = GetNumberOfKeys("backup");

    CFileName cwd;
    CFileSystem::GetCurrentDir(cwd);

    for(int i=0; i < nkeys; i++){
        string iargs = GetKeyArguments("backup",i);
        // parse arguments
        CSTMBackupOptions backupopts;
        if( backupopts.ParseCmdLine(iargs) != SO_CONTINUE ){
            sout << endl;
            sout << "<b><red> ERROR: Unable to parse arguments for " << iargs << "!</red></b>" << endl;
            return(ERS_FAILED);
        }
        for(int arg=0; arg < backupopts.GetNumberOfProgArgs(); arg++){
            // get argument
            CSmallString source = backupopts.GetProgArg(arg);
            sout << "# Backuping data (backup)     : " << source;

            int count = 0;
            CDirectoryEnum denum(cwd);
            denum.StartFindFile(source);
            CFileName file;
            while( denum.FindFile(file) ){
                if( CFileSystem::IsFile(cwd/file) == false ) continue;
                if( CFileSystem::CopyFile(cwd/file,cwd/CFileName(storage)/file,true) == true ){
                    CFileSystem::RemoveFile(cwd/file,true);
                    count++;
                }
            }
            denum.EndFindFile();
            sout << " (" << count << " files backuped)" << endl;
        }
    }

    nkeys = GetNumberOfKeys("remove");

    for(int i=0; i < nkeys; i++){
        string iargs = GetKeyArguments("remove",i);
        // parse arguments
        CSTMRemoveOptions removeopts;
        if( removeopts.ParseCmdLine(iargs) != SO_CONTINUE ){
            sout << endl;
            sout << "<b><red> ERROR: Unable to parse arguments for " << iargs << "!</red></b>" << endl;
            return(ERS_FAILED);
        }
        for(int arg=0; arg < removeopts.GetNumberOfProgArgs(); arg++){
            // get argument
            CSmallString source = removeopts.GetProgArg(arg);
            sout << "# Removing data (remove)      : " << source;

            int count = 0;
            CDirectoryEnum denum(cwd);
            denum.StartFindFile(source);
            CFileName file;
            while( denum.FindFile(file) ){
                if( CFileSystem::IsFile(cwd/file) == false ) continue;
                CFileSystem::RemoveFile(cwd/file,true);
                count++;
            }
            denum.EndFindFile();
            sout << " (" << count << " files removed)" << endl;
        }
    }

    sout << "# ------------------------------------------------" << endl;

    // complete setup
    detected = true;

    job.SetItem("basic/jobinput","INF_JOB_NAME",job_file);
    job.SetItem("basic/jobinput","INF_JOB_TYPE",job_type);
    job.SetItem("basic/jobinput","INF_EXCLUDED_FILES",storage);

    return(ERS_OK);
}

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

bool CSTMClientJobType::CacheJobFile(const CSmallString& file)
{
    ifstream ifs;
    ifs.open(file);
    if( ! ifs ) {
        ES_ERROR("unable to open the job input file");
        return(false);
    }

    string line;
    while( getline(ifs,line) ){
        if( line.empty() == true ) continue;
        JobFileCached.push_back(line);
    }

    return(true);
}

//------------------------------------------------------------------------------

int CSTMClientJobType::GetNumberOfKeys(const std::string& key)
{
    std::vector<std::string>::iterator  it = JobFileCached.begin();
    std::vector<std::string>::iterator  ie = JobFileCached.end();

    int count = 0;

    while( it != ie ){
        string line = *it;
        // split line into words
        vector<string> words;
        split(words,line,is_any_of("\t "),boost::token_compress_on);
        if( words.size() >= 3 ){
            if( (words[0] == "#") && (words[1] == "INFINITY") && (words[2] == key) ){
                count++;
            }
        }

        it++;
    }

    return(count);
}

//------------------------------------------------------------------------------

const std::string CSTMClientJobType::GetKeyArguments(const std::string& key,int id)
{
    std::vector<std::string>::iterator  it = JobFileCached.begin();
    std::vector<std::string>::iterator  ie = JobFileCached.end();

    int count = 0;

    stringstream args;

    args << key;

    while( it != ie ){
        string line = *it;
        // split line into words
        vector<string> words;
        split(words,line,is_any_of("\t "),boost::token_compress_on);
        if( words.size() >= 3 ){
            if( (words[0] == "#") && (words[1] == "INFINITY") && (words[2] == key) ){
                if( count == id ){
                    for(size_t i=3; i < words.size(); i++ ){
                        args << " " << words[i];
                    }
                    return(args.str());
                }
                count++;
            }
        }

        it++;
    }

    return(args.str());
}

//------------------------------------------------------------------------------

const std::string CSTMClientJobType::GetKeyValue(const std::string& key)
{
    std::vector<std::string>::iterator  it = JobFileCached.begin();
    std::vector<std::string>::iterator  ie = JobFileCached.end();

    while( it != ie ){
        string line = *it;

        // split line into words
        vector<string> words;
        split(words,line,is_any_of("\t "),boost::token_compress_on);
        if( words.size() == 4 ){
            if( (words[0] == "#") && (words[1] == "INFINITY") && (words[2] == key) ){
                return(words[3]);
            }
        }

        it++;
    }

    return(string());
}

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

