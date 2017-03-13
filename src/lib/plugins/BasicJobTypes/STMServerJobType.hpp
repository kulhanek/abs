#ifndef STMServerJobTypeH
#define STMServerJobTypeH
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

#include "BasicJobTypesMainHeader.hpp"
#include <JobType.hpp>
#include <fstream>
#include <vector>

// -----------------------------------------------------------------------------

class CIClient {
public:
    std::string dir;
    std::string name;
    int         bead_id;
};

// -----------------------------------------------------------------------------

class CSTMServerJobType : public CJobType {
public:
// constructor -----------------------------------------------------------------
        CSTMServerJobType(void);

// executive methods -----------------------------------------------------------
    /// determine if the job is job of given type
    virtual ERetStatus DetectJobType(CJob& job,bool& detected,std::ostream& sout);

// section of private data -----------------------------------------------------
private:
    std::vector<std::string>    JobFileCached;
    std::vector<CIClient>       IClients;

    /// cache the job file
    bool CacheJobFile(const CSmallString& file);

    /// get number of keys
    int GetNumberOfKeys(const std::string& key);

    /// get arguments of specified key
    const std::string GetKeyArguments(const std::string& key,int id);

    /// get key value
    const std::string GetKeyValue(const std::string& key);

    /// write file with client description
    bool WriteFileWithClients(const std::string& name,CJob& job,std::ostream& sout);

    /// read bead id file
    bool ReadBeadID(CIClient& isubcmd,const std::string& beadidfile,std::ostream& sout);
};

// -----------------------------------------------------------------------------

#endif
