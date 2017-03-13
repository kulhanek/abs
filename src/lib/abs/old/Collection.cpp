// =============================================================================
//  ABS - Advanced Batch System - Uniform Job Management for Scientific Calculations
// -----------------------------------------------------------------------------
//    Copyright (C) 2009 Petr Kulhanek, kulhanek@chemi.muni.cz
//
//     This library is free software; you can redistribute it and/or
//     modify it under the terms of the GNU Lesser General Public
//     License as published by the Free Software Foundation; either
//     version 2.1 of the License, or (at your option) any later version.
//
//     This library is distributed in the hope that it will be useful,
//     but WITHOUT ANY WARRANTY; without even the implied warranty of
//     MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
//     Lesser General Public License for more details.
//
//     You should have received a copy of the GNU Lesser General Public
//     License along with this library; if not, write to the Free Software
//     Foundation, Inc., 51 Franklin Street, Fifth Floor,
//     Boston, MA  02110-1301  USA
// =============================================================================

#include "Collection.hpp"
#include <ErrorSystem.hpp>
#include <XMLParser.hpp>
#include <XMLPrinter.hpp>
#include <XMLElement.hpp>
#include <XMLIterator.hpp>
#include <string.h>
#include <Shell.hpp>
#include <ShellProcessor.hpp>

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

__hipolyspec CCollection::CCollection(void)
{
 Modified = false;
}

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

bool __hipolyspec CCollection::LoadCollection(const CSmallString& name)
{
 CXMLParser xml_parser;
 xml_parser.SetOutputXMLNode(&Collection);

 if( xml_parser.Parse(name) == false ){
    ErrorSystem.AddError("unable to parse collection file","CCollection::LoadCollection");
    return(false);
    }
 return(true);
}

//------------------------------------------------------------------------------

bool __hipolyspec CCollection::SaveCollection(const CSmallString& name)
{
 CXMLPrinter xml_printer;
 xml_printer.SetPrintedXMLNode(&Collection);
 if( xml_printer.Print(name) == false ){
    ErrorSystem.AddError("unable to save collection file","CCollection::SaveCollection");
    return(false);
    }
 Modified = false;
 return(true);
}

//------------------------------------------------------------------------------

bool __hipolyspec CCollection::IsFileModified(void)
{
 return(Modified);
}

//------------------------------------------------------------------------------

bool __hipolyspec CCollection::GetJobCollectionName(CSmallString& info_file_name)
{
 CFileName path,name;

 // complete collection file name ----------------------
 path = CShell::GetSystemVariable("INF_COLLECTION_PATH");
 if( path == NULL ){
    ErrorSystem.AddError("INF_COLLECTION_PATH is empty string","CCollection::GetJobCollectionName");
    return(false);
    }

 name = CShell::GetSystemVariable("INF_COLLECTION_NAME");
 if( name == NULL ){
    ErrorSystem.AddError("INF_COLLECTION_NAME is empty string","CCollection::GetJobCollectionName");
    return(false);
    }
 info_file_name = path / name + ".cofi";
 return(true);
}

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

bool __hipolyspec CCollection::WriteHeader(void)
{
 if( Collection.CreateChildDeclaration() == NULL ){
    ErrorSystem.AddError("unable to create XML declaration","CCollection::WriteHeader");
    return(false);
    }

 CXMLElement* p_header = Collection.CreateChildElementByPath("collection_info/infinity");
 if( p_header == NULL ){
    ErrorSystem.AddError("unable to create collection file header (collection_info/infinity)",
                         "CCollection::WriteHeader");
    return(false);
    }

 CSmallString version,site,site_name,driver;

 version = "INFINITY_COLLECTION_INFO_v_1_0";
 site = CShell::GetSystemVariable("AMS_SITE");
 site_name = CShell::GetSystemVariable("INF_SITE_NAME");
 driver = CShell::GetSystemVariable("INF_DRIVER_NAME");

 bool result = true;
 result &= p_header->SetAttribute("version",version);
 result &= p_header->SetAttribute("site",site);
 result &= p_header->SetAttribute("site_name",site_name);
 result &= p_header->SetAttribute("driver",driver);

 if( result == false ){
    ErrorSystem.AddError("unable to set header attributes","CCollection::WriteHeader");
    return(false);
    }

 Modified = true;

 return(true);
}

//------------------------------------------------------------------------------

bool __hipolyspec CCollection::WriteBasic(void)
{
 if( CreateSection("basic") == false ) return(false);

 if( WriteInfo("basic","INF_COLLECTION_NAME") == false ) return(false);
 if( WriteInfo("basic","INF_COLLECTION_PATH") == false ) return(false);
 if( WriteInfo("basic","INF_COLLECTION_MACHINE") == false ) return(false);

 return(true);
}

//------------------------------------------------------------------------------

bool __hipolyspec CCollection::WriteAddJob(void)
{
 CXMLElement* p_ele = GetElementByPath("basic/jobs",true);
 if( p_ele == NULL ){
    CSmallString error;
    error << "unable to create basic/jobs element by path";
    ErrorSystem.AddError(error,"CCollection::WriteAddJob");
    return(false);
    }

 p_ele = p_ele->CreateChildElement("job");
 if( p_ele == NULL ){
    ErrorSystem.AddError("unable to create job element","CCollection::WriteAddJob");
    return(false);
    }

 bool result = true;

 CSmallString value;

 value = CShell::GetSystemVariable("INF_JOB_NAME");
 result &= p_ele->SetAttribute("name",value);

 value = CShell::GetSystemVariable("INF_JOB_PATH");
 result &= p_ele->SetAttribute("path",value);

 return(true);
}

//------------------------------------------------------------------------------

bool __hipolyspec CCollection::WriteClose(void)
{
 if( CreateSection("close") == false ) return(false);

 return(true);
}

//------------------------------------------------------------------------------

bool __hipolyspec CCollection::WriteSubmit(void)
{
 if( CreateSection("submit") == false ) return(false);

 return(true);
}

//------------------------------------------------------------------------------

bool __hipolyspec CCollection::WriteKill(void)
{
 if( CreateSection("kill") == false ) return(false);

 return(true);
}

//------------------------------------------------------------------------------

bool __hipolyspec CCollection::WriteResurect(void)
{
 if( CreateSection("resurect") == false ) return(false);

 return(true);
}

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

bool __hipolyspec CCollection::CreateSection(const CSmallString& section)
{
 if( section == NULL ){
    ErrorSystem.AddError("section name is empty string","CCollection::CreateSection");
    return(false);
    }

 CXMLElement* p_ele = GetElementByPath(section,true);
 if( p_ele == NULL ){
    CSmallString error;
    error << "unable to create section element by path '" << section << "'";
    ErrorSystem.AddError(error,"CCollection::CreateSection");
    return(false);
    }

 // remove previous contents
 p_ele->RemoveAllAttributes();
 p_ele->RemoveAllChildNodes();

 // set timing
 CSmallTimeAndDate timedate;
 timedate.GetActualTimeAndDate();

 p_ele->SetAttribute("timedate",timedate);
 p_ele->SetAttribute("stimedate",timedate.GetSDateAndTime());

 Modified = true;

 return(true);
}

// --------------------------------------------------------------------------

bool __hipolyspec CCollection::WriteInfo(const CSmallString& category,
                                    const CSmallString& name)
{
 if( name == NULL ){
    ErrorSystem.AddError("name is empty string","CCollection::WriteInfo");
    return(false);
    }

 CXMLElement* p_ele = GetElementByPath(category,true);
 if( p_ele == NULL ){
    CSmallString error;
    error << "unable to create element by path '" << category << "'";
    ErrorSystem.AddError(error,"CCollection::WriteInfo");
    return(false);
    }

 p_ele = p_ele->CreateChildElement("item");
 if( p_ele == NULL ){
    ErrorSystem.AddError("unable to create item element","CCollection::WriteInfo");
    return(false);
    }

 CSmallString value;
 value = CShell::GetSystemVariable(name);

 bool result = true;

 result &= p_ele->SetAttribute("name",name);
 result &= p_ele->SetAttribute("value",value);

 if( result == false ){
    ErrorSystem.AddError("unable to set item","CCollection::WriteInfo");
    return(false);
    }

 Modified = true;

 return(true);
}

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

CXMLElement* __hipolyspec CCollection::GetElementByPath(
                                        const CSmallString& p_path,bool create)
{
 return(Collection.GetChildElementByPath("collection_info/"+p_path,create));
}

//------------------------------------------------------------------------------

CXMLElement* __hipolyspec CCollection::FindItem(CXMLElement* p_root,
                                        const char* p_name)
{
 CXMLIterator    I(p_root);
 CXMLElement*    p_sele;

 while( (p_sele = I.GetNextChildElement("item")) != NULL ){
    CSmallString name;
    bool result = true;
    result &= p_sele->GetAttribute("name",name);
    if( result == false ) return(NULL);
    if( strcmp(name,p_name) == 0 ) return(p_sele);
    }

 return(NULL);
}

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

bool __hipolyspec CCollection::SetItem(const CSmallString& path,
                                  const CSmallString& name,
                                  const CSmallString& value,bool verbose)
{
 // find element ----------------------------
 CXMLElement* p_rele = GetElementByPath(path,true);
 if( p_rele == NULL ){
    ErrorSystem.AddError("unable to open/create element","CCollection::SetItem");
    return(false);
    }

 CXMLElement* p_iele = FindItem(p_rele,name);
 if( p_iele == NULL ){
    p_iele = p_rele->CreateChildElement("item");
    }

 if( p_iele == NULL ){
    ErrorSystem.AddError("unable to find/create item","CCollection::SetItem");
    return(false);
    }

 bool result = true;
 result &= p_iele->SetAttribute("name",name);
 result &= p_iele->SetAttribute("value",value);

 if( result == true ){
    Modified = true;
    if( verbose == true ){
        printf(" item '%s' created or set\n",(const char*)name);
        printf("    value: '%s'\n",(const char*)value);
        }
    }
  else{
    ErrorSystem.AddError("item was not set","CCollection::SetItem");
    }

 return(result);
}

//------------------------------------------------------------------------------

bool __hipolyspec CCollection::IsSection(const CSmallString& path)
{
 CXMLElement* p_rele = GetElementByPath(path,false);
 return( p_rele != NULL );
}

//------------------------------------------------------------------------------

bool __hipolyspec CCollection::PrintItem(const CSmallString& path,
                                  const CSmallString& name)
{
 CXMLElement* p_rele = GetElementByPath(path,false);
 if( p_rele == NULL ){
    ErrorSystem.AddError("unable to open element","CCollection::PrintItem");
    return(false);
    }

 CXMLElement* p_iele = FindItem(p_rele,name);
 if( p_iele == NULL ){
    ErrorSystem.AddError("unable to find item","CCollection::PrintItem");
    return(false);
    }

 CSmallString value;
 p_iele->GetAttribute("value",value);
 printf("%s",(const char*)value);

 return(true);
}

//------------------------------------------------------------------------------

const CSmallString __hipolyspec CCollection::GetItem(const CSmallString& path,
                                  const CSmallString& name)
{
 CXMLElement* p_rele = GetElementByPath(path,false);
 if( p_rele == NULL ){
    ErrorSystem.AddError("unable to open element","CCollection::GetItem");
    return("");
    }

 CXMLElement* p_iele = FindItem(p_rele,name);
 if( p_iele == NULL ){
    ErrorSystem.AddError("unable to find item","CCollection::GetItem");
    return("");
    }

 CSmallString value;
 p_iele->GetAttribute("value",value);

 return(value);
}

//------------------------------------------------------------------------------

bool __hipolyspec CCollection::PrintTime(const CSmallString& path)
{
 CXMLElement* p_rele = GetElementByPath(path,false);
 if( p_rele == NULL ){
    ErrorSystem.AddError("unable to open element","CCollection::PrintTime");
    return(false);
    }

 CSmallString stime;
 p_rele->GetAttribute("stimedate",stime);
 printf("%s\n",(const char*)stime);

 return(true);
}

//------------------------------------------------------------------------------

bool __hipolyspec CCollection::PrintWaitTime(const CSmallString& path)
{
 CXMLElement* p_rele = GetElementByPath(path,false);
 if( p_rele == NULL ){
    ErrorSystem.AddError("unable to open element","CCollection::PrintWaitTime");
    return(false);
    }

 CSmallString           stime;
 CSmallTimeAndDate      current_time_and_date;
 CSmallTimeAndDate      section_time;
 CSmallTime             wait;

 if( p_rele->GetAttribute("timedate",section_time) == false ) return(true);

 current_time_and_date.GetActualTimeAndDate();
 wait = current_time_and_date - section_time;
 printf("%s\n",(const char*)wait.GetSTimeAndDay());

 return(true);
}

//------------------------------------------------------------------------------

bool __hipolyspec CCollection::PrintTimeDiff(const CSmallString& path1,
                                      const CSmallString& path2)
{
 CXMLElement* p_rele1 = GetElementByPath(path1,false);
 if( p_rele1 == NULL ){
    ErrorSystem.AddError("unable to open element 1","CCollection::PrintTimeDiff");
    return(false);
    }

 CXMLElement* p_rele2 = GetElementByPath(path2,false);
 if( p_rele2 == NULL ){
    ErrorSystem.AddError("unable to open element 2","CCollection::PrintTimeDiff");
    return(false);
    }

 CSmallTimeAndDate    section_time1;
 CSmallTimeAndDate    section_time2;

 if( p_rele1->GetAttribute("timedate",section_time1) == false ) return(false);
 if( p_rele2->GetAttribute("timedate",section_time2) == false ) return(false);

 CSmallTime        wait;
 wait = section_time1 - section_time2;
 printf("%s\n",(const char*)wait.GetSTimeAndDay());

 return(true);
}

//------------------------------------------------------------------------------

bool __hipolyspec CCollection::PrintTimeDiffInSeconds(const CSmallString& path1,
                                                 const CSmallString& path2)
{
 CXMLElement* p_rele1 = GetElementByPath(path1,false);
 if( p_rele1 == NULL ){
    ErrorSystem.AddError("unable to open element 1","CCollection::PrintTimeDiffInSeconds");
    return(false);
    }

 CXMLElement* p_rele2 = GetElementByPath(path2,false);
 if( p_rele2 == NULL ){
    ErrorSystem.AddError("unable to open element 2","CCollection::PrintTimeDiffInSeconds");
    return(false);
    }

 int    section_time1;
 int    section_time2;

 if( p_rele1->GetAttribute("timedate",section_time1) == false ) return(false);
 if( p_rele2->GetAttribute("timedate",section_time2) == false ) return(false);

 int        diff;
 diff = section_time1 - section_time2;
 printf("%d\n",diff);

 return(true);
}

//------------------------------------------------------------------------------

bool __hipolyspec CCollection::PrintVer(void)
{
 CXMLElement* p_rele = GetElementByPath("infinity",false);
 if( p_rele == NULL ){
    p_rele = GetElementByPath("cel",false);
    }

 CSmallString ver;
 ver = "CEL_INFO_v_1_0";

 if( p_rele != NULL ){
    p_rele->GetAttribute("version",ver);
    }

 printf("%s\n",(const char*)ver);
 return(true);
}

//------------------------------------------------------------------------------

bool __hipolyspec CCollection::PrintStamp(void)
{
 CXMLElement* p_rele = GetElementByPath("infinity",false);
 if( p_rele == NULL ){
    p_rele = GetElementByPath("cel",false);
    }
 if( p_rele == NULL ){
    ErrorSystem.AddError("infinity(cel) element was not found","CCollection::GetStamp");
    return(false);
    }

 CSmallString mode;
 bool result;
 result = p_rele->GetAttribute("site",mode);
 if( result == false ){
    // charon compatibility
    result = p_rele->GetAttribute("stamp",mode);
    }
 printf("%s\n",(const char*)mode);
 return(result);
}

//------------------------------------------------------------------------------

bool __hipolyspec CCollection::PrintStampName(void)
{
 CXMLElement* p_rele = GetElementByPath("infinity",false);
 if( p_rele == NULL ){
    p_rele = GetElementByPath("cel",false);
    }
 if( p_rele == NULL ){
    ErrorSystem.AddError("infinity(cel) element was not found","CCollection::PrintStampName");
    return(false);
    }

 CSmallString mode;
 bool result;
 result = p_rele->GetAttribute("site_name",mode);

 printf("%s\n",(const char*)mode);
 return(result);
}

//------------------------------------------------------------------------------

const CSmallString __hipolyspec CCollection::GetStampName(void)
{
 CXMLElement* p_rele = GetElementByPath("infinity",false);
 if( p_rele == NULL ){
    p_rele = GetElementByPath("cel",false);
    }
 if( p_rele == NULL ){
    ErrorSystem.AddError("infinity(cel) element was not found","CCollection::GetStampName");
    return("");
    }

 CSmallString mode;
 bool result;
 result = p_rele->GetAttribute("site_name",mode);
 return(mode);
}

//------------------------------------------------------------------------------

bool __hipolyspec CCollection::PrintDriver(void)
{
 CXMLElement* p_rele = GetElementByPath("infinity",false);
 if( p_rele == NULL ){
    p_rele = GetElementByPath("cel",false);
    }
 if( p_rele == NULL ){
    ErrorSystem.AddError("infinity(cel) element was not found","CCollection::GetStamp");
    return(false);
    }

 CSmallString mode;
 bool result = p_rele->GetAttribute("driver",mode);
 printf("%s\n",(const char*)mode);
 return(result);
}

//------------------------------------------------------------------------------

const CSmallString __hipolyspec CCollection::GetDriver(void)
{
 CXMLElement* p_rele = GetElementByPath("infinity",false);
 if( p_rele == NULL ){
    p_rele = GetElementByPath("cel",false);
    }
 if( p_rele == NULL ){
    ErrorSystem.AddError("infinity(cel) element was not found","CCollection::GetStamp");
    return("");
    }

 CSmallString mode;
 p_rele->GetAttribute("driver",mode);
 return(mode);
}

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

bool __hipolyspec CCollection::PrintBasic(void)
{
 CSmallString tmp,tmp1,col;

 tmp = GetItem("basic","INF_COLLECTION_NAME");
 printf("Collection name      : %s\n",(const char*)tmp);
 tmp  = GetItem("basic","INF_COLLECTION_MACHINE");
 tmp1 = GetItem("basic","INF_COLLECTION_PATH");
 printf("Collection directory : %s:%s\n",(const char*)tmp,(const char*)tmp1);

 tmp  = GetStampName();
 tmp1 = GetDriver();

 printf("Site name            : %s (Driver: %s)\n",(const char*)tmp,(const char*)tmp1);

 CXMLElement* p_ele;

 tmp = "-unknown-";
 CSmallTimeAndDate section_time;

 p_ele = GetElementByPath("kill",false);
 if( p_ele != NULL ){
    tmp = "killed";
    p_ele->GetAttribute("timedate",section_time);
    }
 else{
    p_ele = GetElementByPath("submit",false);
    if( p_ele != NULL ){
        tmp = "submitted";
        p_ele->GetAttribute("timedate",section_time);
        }
      else{
        p_ele = GetElementByPath("close",false);
        if( p_ele != NULL ){
            tmp = "closed";
            p_ele->GetAttribute("timedate",section_time);
            }
          else{
            p_ele = GetElementByPath("basic",false);
            if( p_ele != NULL ){
                tmp = "opened";
                p_ele->GetAttribute("timedate",section_time);
                }
            }
        }
    }

 if( tmp != "-unknown-"){
    printf("Collection status    : %s (%s)\n",(const char*)tmp,
                                              (const char*)section_time.GetSDateAndTime());
    }
  else{
    printf("Collection status    : %s\n",(const char*)tmp);
    }

 return(true);
}

//------------------------------------------------------------------------------

bool __hipolyspec CCollection::IsJob(const CSmallString& nid)
{
 if( nid == NULL ) return(false);
 int id = nid.ToInt();

 CXMLElement* p_ele = GetElementByPath("basic/jobs",false);
 if( p_ele == NULL ) return(false);

 CXMLIterator I(p_ele);
 bool result = I.SetToChildElement("job",id) != NULL;

 return(result);
}

//------------------------------------------------------------------------------

bool __hipolyspec CCollection::PrintJobName(const CSmallString& nid)
{
 if( nid == NULL ) return(false);
 int id = nid.ToInt();

 CXMLElement* p_ele = GetElementByPath("basic/jobs",false);
 if( p_ele == NULL ) return(false);

 CXMLIterator I(p_ele);
 CXMLElement* p_job = I.SetToChildElement("job",id);
 if( p_job == NULL ) return(false);

 CSmallString name;
 p_job->GetAttribute("name",name);

 printf("%s",(const char*)name);

 return(true);
}

//------------------------------------------------------------------------------

bool __hipolyspec CCollection::PrintJobPath(const CSmallString& nid)
{
 if( nid == NULL ) return(false);
 int id = nid.ToInt();

 CXMLElement* p_ele = GetElementByPath("basic/jobs",false);
 if( p_ele == NULL ) return(false);

 CXMLIterator I(p_ele);
 CXMLElement* p_job = I.SetToChildElement("job",id);
 if( p_job == NULL ) return(false);

 CSmallString path;
 p_job->GetAttribute("path",path);

 printf("%s",(const char*)path);

 return(true);
}

//------------------------------------------------------------------------------

bool __hipolyspec CCollection::IsJobPath(const CSmallString& path)
{
 CXMLElement* p_ele = GetElementByPath("basic/jobs",false);

 CXMLIterator    I(p_ele);
 CXMLElement*    p_sele;

 while( (p_sele = I.GetNextChildElement("job")) != NULL ){
    CSmallString lpath;
    p_sele->GetAttribute("path",lpath);
    if( lpath == path ) return(true);
    }

 return(false);
}

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

bool __hipolyspec CCollection::RestoreEnvironmentForElement(CXMLElement* p_ele)
{
 bool result = true;

 CXMLIterator   I(p_ele);
 CXMLElement*   p_sele;

 while( (p_sele = I.GetNextChildElement()) != NULL ){
    if( p_sele->GetName() == "item" ){
        CSmallString name;
        CSmallString value;
        p_sele->GetAttribute("name",name);
        p_sele->GetAttribute("value",value);
        if( name != NULL ){
            result &= ShellProcessor.SetVariable(name,value);
            }
        }
      else{
        result &= RestoreEnvironmentForElement(p_sele);
        }
    }

 return(result);
}

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

