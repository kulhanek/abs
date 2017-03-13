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

#include "STMServerJobType.hpp"
#include "BasicJobTypesModule.hpp"
#include <CategoryUUID.hpp>
#include <FileName.hpp>
#include <Job.hpp>
#include <FileSystem.hpp>
#include <PluginDatabase.hpp>
#include <ErrorSystem.hpp>
#include <GlobalConfig.hpp>
#include <boost/format.hpp>
#include <FileName.hpp>
#include <iomanip>
#include <Cache.hpp>
#include <Utils.hpp>
#include "STMServerJobTypeOptions.hpp"
#include <boost/algorithm/string/split.hpp>
#include <boost/algorithm/string/classification.hpp>

//------------------------------------------------------------------------------

using namespace std;
using namespace boost;
using namespace boost::algorithm;

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

CComObject* STMServerJobTypeCB(void* p_data);

CExtUUID        STMServerJobTypeID(
                    "{STM_SERVER_JOB_TYPE:00381034-1773-4b3f-aa63-a6dcd2567cd4}",
                    "STM Server Job Type",
                    "support for fully asynchronous STM server/clients execution");

CPluginObject   STMServerJobTypeObject(&BasicJobTypesPlugin,
                    STMServerJobTypeID,JOB_TYPE_CAT,
                    STMServerJobTypeCB);

// -----------------------------------------------------------------------------

CComObject* STMServerJobTypeCB(void* p_data)
{
    CComObject* p_object = new CSTMServerJobType();
    return(p_object);
}

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

CSTMServerJobType::CSTMServerJobType(void)
    : CJobType(&STMServerJobTypeObject)
{
}

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

ERetStatus CSTMServerJobType::DetectJobType(CJob& job,bool& detected,std::ostream& sout)
{
    detected = false;

    CFileName job_file = job.GetItem("basic/arguments","INF_ARG_JOB");

    CSmallString job_type;
    if( PluginDatabase.FindObjectConfigValue(STMServerJobTypeID,"_type",job_type) == false ){
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
    sout << "# stm-server job summary" << endl;
    sout << "# ------------------------------------------------" << endl;
    sout << "# Master job script name : " << job_file << endl;
    sout << "#" << endl;

    string clients_name =  GetKeyValue("output");
    if( clients_name.empty() ){
        sout << endl;
        sout << "<b><red> ERROR: The name of output file (output) with description of STM client jobs is not provided!</red></b>" << endl;
        return(ERS_FAILED);
    }

    sout << "# STM client jobs description file (output) : " << clients_name << endl;

    string beadidfile;
    beadidfile = GetKeyValue("beadidfile");
    if( beadidfile.empty() ){
        beadidfile = "beadid";
    }

    sout << "# Bead ID definition file (beadidfile)      : " << beadidfile << endl;


    int nkeys = GetNumberOfKeys("client");
    sout << "# Number of STM client jobs specifications  : " << nkeys << endl;

    sout << "#" << endl;
    sout << "# ID BeadID Directory            JobName        " << endl;
    sout << "# -- ------ -------------------- ---------------" << endl;

    // enumerate via all clients commands
    int id = 1;
    for(int i=0; i < nkeys; i++){
        string iargs = GetKeyArguments("client",i);
        // parse arguments
        CIClientOptions clientsopts;
        if( clientsopts.ParseCmdLine(iargs) != SO_CONTINUE ){
            sout << endl;
            sout << "<b><red> ERROR: Unable to parse arguments for " << iargs << "!</red></b>" << endl;
            return(ERS_FAILED);
        }
        // expand
        if( clientsopts.IsOptFromSet() ){
            string pdir = string(clientsopts.GetArgDirectory());
            for(int j=clientsopts.GetOptFrom(); j<=clientsopts.GetOptTo(); j++ ){
                CIClient isubcmd;
                stringstream ssufix;
                try{
                    ssufix << format(pdir) % j;
                } catch(...){
                    sout << endl;
                    sout << "<b><red> ERROR: Unable to expand directory name " << pdir << "!</red></b>" << endl;
                    return(ERS_FAILED);
                }

                isubcmd.dir = ssufix.str();
                isubcmd.name = clientsopts.GetArgJobName();

                if( ReadBeadID(isubcmd,beadidfile,sout) == false ) return(ERS_FAILED);

                IClients.push_back(isubcmd);
                sout << right << "# " << setw(2) << id << " ";
                sout << setw(6) << isubcmd.bead_id << " ";
                sout << left << setw(20) << isubcmd.dir << " ";
                sout << left << setw(15) << isubcmd.name << " ";
                sout << endl;
                id++;
            }
        } else {
            CIClient isubcmd;
            isubcmd.dir = clientsopts.GetArgDirectory();
            isubcmd.name = clientsopts.GetArgJobName();

            if( ReadBeadID(isubcmd,beadidfile,sout) == false ) return(ERS_FAILED);

            IClients.push_back(isubcmd);
            sout << right << "# " << setw(2) << id << " ";
            sout << setw(6) << isubcmd.bead_id << " ";
            sout << left << setw(20) << isubcmd.dir << " ";
            sout << left << setw(15) << isubcmd.name << " ";
            sout << endl;
        }
    }

    sout << "# ---------------------------------------------" << endl;

    if( IClients.size() <= 0 ){
        sout << endl;
        sout << "<b><red> ERROR: No clients defined for STM server!</red></b>" << endl;
        return(ERS_FAILED);
    }

    CFileName excfiles;
    excfiles = CFileName(IClients[0].dir).GetFileDirectory();

    for(size_t i=0; i < IClients.size(); i++){
        if( CFileName(IClients[i].dir).GetFileDirectory() != excfiles ){
            sout << endl;
            sout << "<b><red> ERROR: The root directory for all clients must be the same!</red></b>" << endl;
            sout << "<b><red>        The client " << i+1 << " has " << CFileName(IClients[i].dir).GetFileDirectory();
            sout << " but it should be " << excfiles << "</red></b>" << endl;
            return(ERS_FAILED);
        }
        if( CFileSystem::IsDirectory(CFileName(IClients[i].dir)) == false ){
            sout << endl;
            sout << "<b><red> ERROR: The client directory " << IClients[i].dir << " not found for client " << i+1 << "!</red></b>" << endl;
            return(ERS_FAILED);
        }
        if( CFileSystem::IsFile(CFileName(IClients[i].dir) / CFileName(IClients[i].name)) == false ){
            sout << endl;
            sout << "<b><red> ERROR: The client script " << IClients[i].name << " not found in directory of client " << i+1 << "!</red></b>" << endl;
            return(ERS_FAILED);
        }
    }
    sout << "# Excluded directory   : " << excfiles << endl;

    // write clients file
    if( WriteFileWithClients(clients_name,job,sout) == false ) return(ERS_FAILED);

    // complete setup
    detected = true;

    job.SetItem("basic/jobinput","INF_JOB_NAME",job_file);
    job.SetItem("basic/jobinput","INF_JOB_TYPE",job_type);
    job.SetItem("basic/jobinput","INF_EXCLUDED_FILES",excfiles);

    return(ERS_OK);
}

//------------------------------------------------------------------------------

bool CSTMServerJobType::ReadBeadID(CIClient& isubcmd,const std::string& beadidfile,std::ostream& sout)
{
    // read beadid
    ifstream bfs;
    CFileName bfile = CFileName(isubcmd.dir) / CFileName(beadidfile);

    bfs.open(bfile);
    if( ! bfs ){
        sout << endl;
        sout << "<b><red> ERROR: Unable to open bead ID file '" << bfile << "'!</red></b>" << endl;
        return(false);
    }
    isubcmd.bead_id = -1;
    bfs >> isubcmd.bead_id;
    bfs.close();

    if( isubcmd.bead_id <= 0 ){
        sout << endl;
        sout << "<b><red> ERROR: Bead ID is not in legal range!</red></b>" << endl;
        return(false);
    }

    return(true);
}

//------------------------------------------------------------------------------

bool CSTMServerJobType::WriteFileWithClients(const std::string& name,CJob& job,std::ostream& sout)
{
    ofstream ofs;

    ofs.open(name.c_str());
    if( ! ofs ){
        sout << endl;
        sout << "<b><red> ERROR: Unable to open file '" << name << "' with description of STM client jobs for writing!</red></b>" << endl;
        return(false);
    }

    ofs << "# ----------------------------------------------------" << endl;
    ofs << "# description of STM client jobs generated by INFINITY" << endl;
    ofs << "# Associated STM server job" << endl;
    ofs << "# " << GlobalConfig.GetHostName() << ":" << job.GetJobPath() << "/" << endl;
    ofs << "# ----------------------------------------------------" << endl;
    ofs << endl;
    ofs << "[default]" << endl;
    ofs << "hostname  " << GlobalConfig.GetHostName() << endl;

    // do we have same job names?
    string first_name;
    bool same_name = true;
    for(size_t i=0; i < IClients.size(); i++){
        if( i == 0 ) first_name = IClients[i].name;
        if( first_name != IClients[i].name ){
            same_name = false;
            break;
        }
    }

    if( same_name ){
        ofs << "jobname   " << first_name << endl;
    }

    // individual files
    for(size_t i=0; i < IClients.size(); i++){
        ofs << endl;
        ofs << "[job]" << endl;
        CFileName jobdir = job.GetJobPath();
        jobdir = jobdir / IClients[i].dir;
        ofs << "path    " << jobdir << endl;
        if( ! same_name ){
            ofs << "jobname " << IClients[i].name << endl;
        }
        ofs << "bead_id " << IClients[i].bead_id << endl;
    }

    return(true);
}

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

bool CSTMServerJobType::CacheJobFile(const CSmallString& file)
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

int CSTMServerJobType::GetNumberOfKeys(const std::string& key)
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

const std::string CSTMServerJobType::GetKeyArguments(const std::string& key,int id)
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

const std::string CSTMServerJobType::GetKeyValue(const std::string& key)
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

