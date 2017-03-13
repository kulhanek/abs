// =============================================================================
//  ABS - Advanced Batch System
// -----------------------------------------------------------------------------
//    Copyright (C) 2012 Petr Kulhanek (kulhanek@chemi.muni.cz)
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

#include <User.hpp>
#include <pwd.h>
#include <unistd.h>
#include <grp.h>
#include <fnmatch.h>
#include <errno.h>
#include <ErrorSystem.hpp>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <FileName.hpp>
#include <XMLParser.hpp>
#include <FileSystem.hpp>
#include <XMLElement.hpp>
#include <GlobalConfig.hpp>
#include <boost/algorithm/string/split.hpp>
#include <boost/algorithm/string/trim.hpp>
#include <boost/algorithm/string/classification.hpp>
#include <boost/algorithm/string/join.hpp>

//------------------------------------------------------------------------------

using namespace std;
using namespace boost;
using namespace boost::algorithm;

//------------------------------------------------------------------------------

CUser    User;

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

CUser::CUser(void)
{
    UID = -1;
    RGID = -1;
    EGID = -1;
}

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

bool CUser::InitUser(void)
{
    if( ProvidedName == NULL ){
        uid_t uid =  getuid();
        return(InitUser(uid));
    } else {
        return(InitUser(ProvidedName));
    }
}

//------------------------------------------------------------------------------

void CUser::ClearAll(void)
{
    UID = -1;
    RGID = -1;
    EGID = -1;
    Name = NULL;
    RGroup = NULL;
    EGroup = NULL;
    PosixGroups.clear();
}

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

bool CUser::InitUser(const CSmallString& name)
{
    struct passwd* pwd;
    errno = 0;
    pwd = getpwnam(name);
    if( pwd == NULL ){
        CSmallString error;
        error << "unable to get user ID of '" << name << "' (" << strerror(errno) << ")";
        ES_ERROR(error);
        return(false);
    }
    return(InitUser(pwd->pw_uid));
}

//------------------------------------------------------------------------------

bool CUser::InitUser(uid_t userid)
{
    PosixGroups.clear();

    struct passwd* pwd;
    errno = 0;
    pwd = getpwuid(userid);
    if( pwd == NULL ){
        CSmallString error;
        error << "unable to get passwd info of '" << userid << "' (" << strerror(errno) << ")";
        ES_ERROR(error);
        return(false);
    }

    UID = pwd->pw_uid;
    RGID = pwd->pw_gid;
    EGID = getegid();
    Name = pwd->pw_name;

    // get group name
    struct group* grp;
    grp = getgrgid(RGID);
    if( grp == NULL ){
        CSmallString error;
        error << "unable to get group info of '" << RGID << "' (" << strerror(errno) << ")";
        ES_ERROR(error);
        return(false);
    }

    RGroup = grp->gr_name;

    grp = getgrgid(EGID);
    if( grp == NULL ){
        CSmallString error;
        error << "unable to get effective group info of '" << EGID << "' (" << strerror(errno) << ")";
        ES_ERROR(error);
        return(false);
    }

    // determine primary group
    EGroup = grp->gr_name;

    // get groups
    int ngroups = 0;
    getgrouplist(Name,RGID,NULL,&ngroups);
    if( ngroups < 0 ){
        ES_ERROR("no groups for user");
        return(false);
    }

    gid_t* groups = (gid_t*)malloc(ngroups * sizeof (gid_t));
    getgrouplist(Name,RGID,groups,&ngroups);

    for(int i=0; i < ngroups; i++){
        grp = getgrgid(groups[i]);
        if( grp != NULL ) PosixGroups.push_back(grp->gr_name);
    }

    free(groups);

    return(true);
}

//------------------------------------------------------------------------------

void CUser::SetUserName(const CSmallString& name)
{
    ProvidedName = name;
}

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

const CSmallString& CUser::GetName(void) const
{
    return(Name);
}

//------------------------------------------------------------------------------

const CSmallString CUser::GetGroups(void)
{
    CSmallString groups;

    std::vector<CSmallString>::iterator   it = PosixGroups.begin();
    std::vector<CSmallString>::iterator   ie = PosixGroups.end();
    while(it != ie){
        if( it != PosixGroups.begin() )  groups << ",";
        groups << (*it);
        it++;
    }
    return(groups);
}

//------------------------------------------------------------------------------

bool CUser::IsInGroup(const CSmallString& grpname)
{
    std::vector<CSmallString>::iterator   it = PosixGroups.begin();
    std::vector<CSmallString>::iterator   ie = PosixGroups.end();
    while(it != ie){
        if( grpname == (*it) )  return(true);
        it++;
    }

    return(false);
}

//------------------------------------------------------------------------------

void CUser::PrintUserInfo(CVerboseStr& vout)
{
    vout << "# User name            : " << Name << endl;
    vout << "# User UID             : " << UID << endl;
    vout << "# Real group name      : " << RGroup << endl;
    vout << "# Real group ID        : " << RGID << endl;
    vout << "# Effective group name : " << EGroup << endl;
    vout << "# Effective group ID   : " << EGID << endl;
    vout << "# User groups          : " << GetGroups() << endl;
}

//------------------------------------------------------------------------------

const CSmallString& CUser::GetEGroup(void) const
{
    return(EGroup);
}

//------------------------------------------------------------------------------

const CSmallString CUser::GetUMask(void) const
{
   mode_t mumask = 0;
   mumask = umask(mumask); // get umask - it destroys current setup
   umask(mumask); // restore umask
   stringstream str;
   char c1 = (mumask & 0007) + '0';
   char c2 = ((mumask & 0070) >> 3) + '0';
   char c3 = ((mumask & 0700) >> 6) + '0';
   str << c3 << c2 << c1;
   return(str.str());
}

//------------------------------------------------------------------------------

uid_t CUser::GetUserID(void) const
{
    return(UID);
}

//------------------------------------------------------------------------------

gid_t CUser::GetGroupID(const CSmallString& name)
{
    struct group *p_grp = getgrnam(name);
    if( p_grp == NULL ){
        CSmallString error;
        error << "no gid for '" << name << "' - trying to use nogroup as bypass";
        ES_ERROR(error);
        p_grp = getgrnam("nogroup");
        if( p_grp == NULL ) return(-1);
    }
    return(p_grp->gr_gid);
}

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

