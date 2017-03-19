// =============================================================================
//  ABS - Advanced Batch System - Uniform Job Management for Scientific Calculations
// -----------------------------------------------------------------------------
//    Copyright (C) 2001-2008 Petr Kulhanek, kulhanek@chemi.muni.cz
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

#include "InfoFile.hpp"
#include <ErrorSystem.hpp>
#include <XMLParser.hpp>
#include <XMLPrinter.hpp>
#include <XMLElement.hpp>
#include <XMLIterator.hpp>
#include <string.h>
#include <Shell.hpp>
#include <ShellProcessor.hpp>

//------------------------------------------------------------------------------

// <?xml version='1.0' encoding='utf-8' ?>
// <!-- XML File -->
// <job_info>
//     <infinity version="INFINITY_INFO_v_1_0" driver="" site=""/>
//     <basic>
//         <external>
//             <item name="INF_EXTERNAL_NAME_AFIX" value=""/>
//             <item name="INF_EXTERNAL_VARIABLES" value=""/>
//             <item name="INF_EXTERNAL_START_AFTER" value=""/>
//         </external>
//         <arguments>
//             <item name="INF_ARG_QUEUE" value=""/>
//             <item name="INF_ARG_JOB" value=""/>
//             <item name="INF_ARG_NCPU" value=""/>
//             <item name="INF_ARG_SYNC_MODE" value=""/>
//         </arguments>
//         <jobinput>
//             <item name="INF_JOB_NAME" value=""/>
//             <item name="INF_JOB_TITLE" value=""/>
//             <item name="INF_JOB_TYPE" value=""/>
//             <item name="INF_JOB_PATH" value=""/>
//             <item name="INF_JOB_MACHINE" value=""/>
//             <item name="INF_JOB_PROJECT" value=""/>
//         </jobinput>
//         <modules>
//             <item name="INF_EXPORTED_MODULES" value=""/>
//         </modules>
//     </basic>
// </job_info>

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

__hipolyspec CInfoFile::CInfoFile(void)
{
 Modified = false;
}

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

bool __hipolyspec CInfoFile::LoadInfoFile(const CSmallString& name)
{
 CXMLParser xml_parser;
 xml_parser.SetOutputXMLNode(&InfoFile);

 if( xml_parser.Parse(name) == false ){
    ES_ERROR("unable to parse info file","CInfoFile::LoadInfoFile");
    return(false);
    }
 return(true);
}

//------------------------------------------------------------------------------

bool __hipolyspec CInfoFile::SaveInfoFile(const CSmallString& name)
{
 CXMLPrinter xml_printer;
 xml_printer.SetPrintedXMLNode(&InfoFile);
 if( xml_printer.Print(name) == false ){
    ES_ERROR("unable to save info file","CInfoFile::SaveInfoFile");
    return(false);
    }
 Modified = false;
 return(true);
}

//------------------------------------------------------------------------------

bool __hipolyspec CInfoFile::IsFileModified(void)
{
 return(Modified);
}

//------------------------------------------------------------------------------

bool __hipolyspec CInfoFile::GetJobInfoFileName(CSmallString& info_file_name)
{
 CSmallString ext;

 // complete info file name ----------------------
 info_file_name = CShell::GetSystemVariable("INF_JOB_NAME");
 if( info_file_name == NULL ){
    ES_ERROR("INF_JOB_NAME is empty string","CInfoFile::GetJobInfoFileName");
    return(false);
    }
 ext = CShell::GetSystemVariable("INF_EXTERNAL_NAME_AFIX");
 if( ext != NULL ){
    info_file_name += ext;
    }
 info_file_name += ".info";
 return(true);
}

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

bool __hipolyspec CInfoFile::WriteHeader(void)
{
 if( InfoFile.CreateChildDeclaration() == NULL ){
    ES_ERROR("unable to create XML declaration","CInfoFile::WriteHeader");
    return(false);
    }

 CXMLElement* p_header = InfoFile.CreateChildElementByPath("job_info/infinity");
 if( p_header == NULL ){
    ES_ERROR("unable to create info file header (job_info/infinity)",
                         "CInfoFile::WriteHeader");
    return(false);
    }

 CSmallString version,site,site_name,driver;

 version = "INFINITY_INFO_v_1_0";
 site = CShell::GetSystemVariable("AMS_SITE");
 site_name = CShell::GetSystemVariable("INF_SITE_NAME");
 driver = CShell::GetSystemVariable("INF_DRIVER_NAME");

 bool result = true;
 result &= p_header->SetAttribute("version",version);
 result &= p_header->SetAttribute("site",site);
 result &= p_header->SetAttribute("site_name",site_name);
 result &= p_header->SetAttribute("driver",driver);

 if( result == false ){
    ES_ERROR("unable to set header attributes","CInfoFile::WriteHeader");
    return(false);
    }

 Modified = true;

 return(true);
}

//------------------------------------------------------------------------------

bool __hipolyspec CInfoFile::WriteBasic(void)
{
 if( CreateSection("basic") == false ) return(false);

 // external ------------------------------------------------------------------ 

 if( WriteInfo("basic/external","INF_EXTERNAL_NAME_AFIX") == false ) return(false);
 if( WriteInfo("basic/external","INF_EXTERNAL_VARIABLES") == false ) return(false);
 if( WriteInfo("basic/external","INF_EXTERNAL_START_AFTER") == false ) return(false);

 // arguments -----------------------------------------------------------------

 if( WriteInfo("basic/arguments","INF_ARG_QUEUE") == false ) return(false);
 if( WriteInfo("basic/arguments","INF_ARG_JOB") == false ) return(false);
 if( WriteInfo("basic/arguments","INF_ARG_NCPU") == false ) return(false);
 if( WriteInfo("basic/arguments","INF_ARG_SYNC_MODE") == false ) return(false);

 // variables from infinity_determine_job_type ----------------------------------

 if( WriteInfo("basic/jobinput","INF_JOB_NAME") == false ) return(false); 
 if( WriteInfo("basic/jobinput","INF_JOB_TITLE") == false ) return(false);
 if( WriteInfo("basic/jobinput","INF_JOB_TYPE") == false ) return(false);
 if( WriteInfo("basic/jobinput","INF_JOB_PATH") == false ) return(false);
 if( WriteInfo("basic/jobinput","INF_JOB_MACHINE") == false ) return(false);
 if( WriteInfo("basic/jobinput","INF_JOB_PROJECT") == false ) return(false);
 if( WriteInfo("basic/jobinput","INF_EXCLUDED_FILES") == false ) return(false);

 // variables - exported modules ----------------------------------------------

 if( WriteInfo("basic/modules","INF_EXPORTED_MODULES") == false ) return(false);

 // recycle ------------------------------------------------------------------ 

 if( WriteInfo("basic/recycle","INF_RECYCLE_START") == false ) return(false);
 if( WriteInfo("basic/recycle","INF_RECYCLE_STOP") == false ) return(false);
 if( WriteInfo("basic/recycle","INF_RECYCLE_CURRENT") == false ) return(false);

 // collection ------------------------------------------------------------------ 

 if( WriteInfo("basic/collection","INF_COLLECTION_NAME") == false ) return(false);
 if( WriteInfo("basic/collection","INF_COLLECTION_PATH") == false ) return(false);
 if( WriteInfo("basic/collection","INF_COLLECTION_MACHINE") == false ) return(false);

 return(true);
}

//------------------------------------------------------------------------------

bool __hipolyspec CInfoFile::WriteSpecificPBS(void)
{
 if( CreateSection("specific") == false ) return(false);

 // common specific items
 if( WriteInfo("specific/resources","INF_QUEUE") == false ) return(false);
 if( WriteInfo("specific/resources","INF_ALIAS") == false ) return(false);
 if( WriteInfo("specific/resources","INF_NCPU") == false ) return(false);
 if( WriteInfo("specific/resources","INF_MAX_CPUS_PER_NODE") == false ) return(false);
 if( WriteInfo("specific/resources","INF_NUM_OF_NODES") == false ) return(false);
 if( WriteInfo("specific/resources","INF_RESOURCES") == false ) return(false);
 if( WriteInfo("specific/resources","INF_PROPERTIES") == false ) return(false);
 if( WriteInfo("specific/resources","INF_W_ARGS") == false ) return(false); 
 if( WriteInfo("specific/resources","INF_SYNCMODE") == false ) return(false); 

 return(true);
}

//------------------------------------------------------------------------------

bool __hipolyspec CInfoFile::WriteSpecificGLiteWMS(void)
{
 if( CreateSection("specific") == false ) return(false);

 // common specific items
 if( WriteInfo("specific/resources","INF_QUEUE") == false ) return(false);
 if( WriteInfo("specific/resources","INF_ALIAS") == false ) return(false);
 if( WriteInfo("specific/resources","INF_NCPU") == false ) return(false);
 if( WriteInfo("specific/resources","INF_MAX_CPUS_PER_NODE") == false ) return(false);
 if( WriteInfo("specific/resources","INF_NUM_OF_NODES") == false ) return(false);
 if( WriteInfo("specific/resources","INF_RESOURCES") == false ) return(false);
 if( WriteInfo("specific/resources","INF_PROPERTIES") == false ) return(false);
 if( WriteInfo("specific/resources","INF_SYNCMODE") == false ) return(false);

 return(true);
}

//------------------------------------------------------------------------------

bool __hipolyspec CInfoFile::WriteBackgroundSubmit(void)
{
 // this operation can be repeated several times
 if( IsSection("background_submit") == false ){
    if( CreateSection("background_submit") == false ) return(false);
    }

 if( WriteInfo("background_submit/job","INF_BACKGROUND_JOB_ID") == true ) return(true);

 return(true);
}

//------------------------------------------------------------------------------

bool __hipolyspec CInfoFile::WriteSubmit(void)
{
 if( CreateSection("submit") == false ) return(false);
 
 if( WriteInfo("submit/job","INF_JOB_ID") == false ) return(false);

 return(true);
}

//------------------------------------------------------------------------------

bool __hipolyspec CInfoFile::WriteStart(void)
{
 if( CreateSection("start") == false ) return(false);

 // workdir --------------------------------------
 if( WriteInfo("start/workdir","INF_MAIN_NODE") == false ) return(1);
 if( WriteInfo("start/workdir","INF_WORK_DIR") == false ) return(1);

 CXMLElement* p_ele;

 p_ele = GetElementByPath("start/nodes",true);
 if( p_ele == NULL ) return(false);

 // nodes ----------------------------------------
 CSmallString machinefile;

 machinefile = CShell::GetSystemVariable("INF_NODEFILE");

 FILE* p_file = fopen(machinefile,"r");
 if( p_file == NULL ){
    CSmallString error;
    error << "unable to open machinefile '" << machinefile << "'";
    ES_ERROR(error,"CInfoFile::WriteStart");
    return(false);
    }

 char buffer[1024];

 buffer[1023]='\0';

 while(fgets(buffer,1023,p_file) != NULL){
        CXMLElement* p_sele = p_ele->CreateChildElement("node");
        if( p_sele == NULL ){
                fprintf(stderr,"\n");
                fprintf(stderr," ERROR: Unable create 'node' element !\n");
                fprintf(stderr,"\n");
                return(false);
                }
        CSmallString nodename(buffer);
        if( strlen(buffer) > 1 ){
                buffer[strlen(buffer)-1]='\0';
                nodename = buffer;
                }
        p_sele->SetAttribute("name",nodename);
        }

 fclose(p_file);

 return(true);
}

//------------------------------------------------------------------------------

bool __hipolyspec CInfoFile::WriteStop(void)
{
 if( CreateSection("stop") == false ) return(false);

 return(true);
}

//------------------------------------------------------------------------------

bool __hipolyspec CInfoFile::WriteKill(void)
{
 if( CreateSection("kill") == false ) return(false);

 return(true);
}

//------------------------------------------------------------------------------

bool __hipolyspec CInfoFile::WriteAbort(void)
{
 if( CreateSection("abort") == false ) return(false);

 return(true);
}

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

bool __hipolyspec CInfoFile::CreateSection(const CSmallString& section)
{
 if( section == NULL ){
    ES_ERROR("section name is empty string","CInfoFile::CreateSection");
    return(false);
    }

 CXMLElement* p_ele = GetElementByPath(section,true);
 if( p_ele == NULL ){
    CSmallString error;
    error << "unable to create section element by path '" << section << "'";
    ES_ERROR(error,"CInfoFile::CreateSection");
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

bool __hipolyspec CInfoFile::WriteInfo(const CSmallString& category,
                                    const CSmallString& name)
{
 if( name == NULL ){
    ES_ERROR("name is empty string","CInfoFile::WriteInfo");
    return(false);
    }

 CXMLElement* p_ele = GetElementByPath(category,true);
 if( p_ele == NULL ){
    CSmallString error;
    error << "unable to create element by path '" << category << "'";
    ES_ERROR(error,"CInfoFile::WriteInfo");
    return(false);
    }

 p_ele = p_ele->CreateChildElement("item");
 if( p_ele == NULL ){
    ES_ERROR("unable to create item element","CInfoFile::WriteInfo");
    return(false);
    }

 CSmallString value;
 value = CShell::GetSystemVariable(name);

 bool result = true;

 result &= p_ele->SetAttribute("name",name);
 result &= p_ele->SetAttribute("value",value);

 if( result == false ){
    ES_ERROR("unable to set item","CInfoFile::WriteInfo");
    return(false);
    }

 Modified = true;

 return(true);
}

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

CXMLElement* __hipolyspec CInfoFile::GetElementByPath(
                                        const CSmallString& p_path,bool create)
{
 return(InfoFile.GetChildElementByPath("job_info/"+p_path,create));
}

//------------------------------------------------------------------------------

CXMLElement* __hipolyspec CInfoFile::FindItem(CXMLElement* p_root,
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

bool __hipolyspec CInfoFile::SetItem(const CSmallString& path,
                                  const CSmallString& name,
                                  const CSmallString& value,bool verbose)
{
 // find element ----------------------------
 CXMLElement* p_rele = GetElementByPath(path,true);
 if( p_rele == NULL ){
    ES_ERROR("unable to open/create element","CInfoFile::SetItem");
    return(false);
    }

 CXMLElement* p_iele = FindItem(p_rele,name);
 if( p_iele == NULL ){
    p_iele = p_rele->CreateChildElement("item");
    }

 if( p_iele == NULL ){
    ES_ERROR("unable to find/create item","CInfoFile::SetItem");
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
    ES_ERROR("item was not set","CInfoFile::SetItem");
    }

 return(result);
}

//------------------------------------------------------------------------------

bool __hipolyspec CInfoFile::IsSection(const CSmallString& path)
{
 CXMLElement* p_rele = GetElementByPath(path,false);
 return( p_rele != NULL );
}

//------------------------------------------------------------------------------

bool __hipolyspec CInfoFile::PrintItem(const CSmallString& path,
                                  const CSmallString& name)
{
 CXMLElement* p_rele = GetElementByPath(path,false);
 if( p_rele == NULL ){
    ES_ERROR("unable to open element","CInfoFile::PrintItem");
    return(false);
    }

 CXMLElement* p_iele = FindItem(p_rele,name);
 if( p_iele == NULL ){
    ES_ERROR("unable to find item","CInfoFile::PrintItem");
    return(false);
    }

 CSmallString value;
 p_iele->GetAttribute("value",value);
 printf("%s",(const char*)value);

 return(true);
}

//------------------------------------------------------------------------------

const CSmallString __hipolyspec CInfoFile::GetItem(const CSmallString& path,
                                  const CSmallString& name)
{
 CXMLElement* p_rele = GetElementByPath(path,false);
 if( p_rele == NULL ){
    ES_ERROR("unable to open element","CInfoFile::GetItem");
    return("");
    }

 CXMLElement* p_iele = FindItem(p_rele,name);
 if( p_iele == NULL ){
    ES_ERROR("unable to find item","CInfoFile::GetItem");
    return("");
    }

 CSmallString value;
 p_iele->GetAttribute("value",value);

 return(value);
}

//------------------------------------------------------------------------------

bool __hipolyspec CInfoFile::PrintTime(const CSmallString& path)
{
 CXMLElement* p_rele = GetElementByPath(path,false);
 if( p_rele == NULL ){
    ES_ERROR("unable to open element","CInfoFile::PrintTime");
    return(false);
    }

 CSmallString stime;
 p_rele->GetAttribute("stimedate",stime);
 printf("%s\n",(const char*)stime);

 return(true);
}

//------------------------------------------------------------------------------

bool __hipolyspec CInfoFile::PrintWaitTime(const CSmallString& path)
{
 CXMLElement* p_rele = GetElementByPath(path,false);
 if( p_rele == NULL ){
    ES_ERROR("unable to open element","CInfoFile::PrintWaitTime");
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

bool __hipolyspec CInfoFile::PrintTimeDiff(const CSmallString& path1,
                                      const CSmallString& path2)
{
 CXMLElement* p_rele1 = GetElementByPath(path1,false);
 if( p_rele1 == NULL ){
    ES_ERROR("unable to open element 1","CInfoFile::PrintTimeDiff");
    return(false);
    }

 CXMLElement* p_rele2 = GetElementByPath(path2,false);
 if( p_rele2 == NULL ){
    ES_ERROR("unable to open element 2","CInfoFile::PrintTimeDiff");
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

bool __hipolyspec CInfoFile::PrintTimeDiffInSeconds(const CSmallString& path1,
                                                 const CSmallString& path2)
{
 CXMLElement* p_rele1 = GetElementByPath(path1,false);
 if( p_rele1 == NULL ){
    ES_ERROR("unable to open element 1","CInfoFile::PrintTimeDiffInSeconds");
    return(false);
    }

 CXMLElement* p_rele2 = GetElementByPath(path2,false);
 if( p_rele2 == NULL ){
    ES_ERROR("unable to open element 2","CInfoFile::PrintTimeDiffInSeconds");
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

bool __hipolyspec CInfoFile::PrintVer(void)
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

bool __hipolyspec CInfoFile::PrintMode(void)
{
 CXMLElement* p_rele = GetElementByPath("infinity",false);
 if( p_rele == NULL ){
    p_rele = GetElementByPath("cel",false);
    }

 CSmallString mode;
 mode = "PBS";

 if( p_rele != NULL ){
    p_rele->GetAttribute("mode",mode);
    }

 printf("%s\n",(const char*)mode);
 return(true);
}

//------------------------------------------------------------------------------

bool __hipolyspec CInfoFile::PrintSubMode(void)
{
 CXMLElement* p_rele = GetElementByPath("infinity",false);
 if( p_rele == NULL ){
    p_rele = GetElementByPath("cel",false);
    }

 CSmallString mode;
 mode = "PBS";

 if( p_rele != NULL ){
    p_rele->GetAttribute("submode",mode);
    }

 printf("%s\n",(const char*)mode);
 return(true);
}

//------------------------------------------------------------------------------

bool __hipolyspec CInfoFile::PrintStamp(void)
{
 CXMLElement* p_rele = GetElementByPath("infinity",false);
 if( p_rele == NULL ){
    p_rele = GetElementByPath("cel",false);
    }
 if( p_rele == NULL ){
    ES_ERROR("infinity(cel) element was not found","CInfoFile::GetStamp");
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

bool __hipolyspec CInfoFile::PrintStampName(void)
{
 CXMLElement* p_rele = GetElementByPath("infinity",false);
 if( p_rele == NULL ){
    p_rele = GetElementByPath("cel",false);
    }
 if( p_rele == NULL ){
    ES_ERROR("infinity(cel) element was not found","CInfoFile::PrintStampName");
    return(false);
    }

 CSmallString mode;
 bool result;
 result = p_rele->GetAttribute("site_name",mode);

 printf("%s\n",(const char*)mode);
 return(result);
}

//------------------------------------------------------------------------------

const CSmallString __hipolyspec CInfoFile::GetStampName(void)
{
 CXMLElement* p_rele = GetElementByPath("infinity",false);
 if( p_rele == NULL ){
    p_rele = GetElementByPath("cel",false);
    }
 if( p_rele == NULL ){
    ES_ERROR("infinity(cel) element was not found","CInfoFile::GetStampName");
    return("");
    }

 CSmallString mode;
 bool result;
 result = p_rele->GetAttribute("site_name",mode);
 return(mode);
}

//------------------------------------------------------------------------------

bool __hipolyspec CInfoFile::PrintDriver(void)
{
 CXMLElement* p_rele = GetElementByPath("infinity",false);
 if( p_rele == NULL ){
    p_rele = GetElementByPath("cel",false);
    }
 if( p_rele == NULL ){
    ES_ERROR("infinity(cel) element was not found","CInfoFile::GetStamp");
    return(false);
    }

 CSmallString mode;
 bool result = p_rele->GetAttribute("driver",mode);
 printf("%s\n",(const char*)mode);
 return(result);
}

//------------------------------------------------------------------------------

const CSmallString __hipolyspec CInfoFile::GetDriver(void)
{
 CXMLElement* p_rele = GetElementByPath("infinity",false);
 if( p_rele == NULL ){
    p_rele = GetElementByPath("cel",false);
    }
 if( p_rele == NULL ){
    ES_ERROR("infinity(cel) element was not found","CInfoFile::GetStamp");
    return("");
    }

 CSmallString mode;
 p_rele->GetAttribute("driver",mode);
 return(mode);
}

//------------------------------------------------------------------------------

bool __hipolyspec CInfoFile::MergeSections(const CSmallString& outsec,
                                           const CSmallString& source,
                                           const CSmallString& insec)
{
 CXMLElement* p_oele = GetElementByPath(outsec,true);
 if( p_oele == NULL ){
    ES_ERROR("output section was not found","CInfoFile::MergeSections");
    return(false);
    }

 CXMLDocument   source_file;
 CXMLParser     xml_parser;

 xml_parser.SetOutputXMLNode(&source_file);

 if( xml_parser.Parse(source) == false ){
    ES_ERROR("unable to parse source file","CInfoFile::MergeSections");
    return(false);
    }

 CXMLElement* p_iele = source_file.GetChildElementByPath("job_info/"+insec,false);
 if( p_iele == NULL ){
    ES_ERROR("input section does not exist","CInfoFile::MergeSections");
    return(false);
    }

 bool result = p_oele->CopyContentsFrom(p_iele);

 Modified = result;

 return(result);
}

//------------------------------------------------------------------------------

bool __hipolyspec CInfoFile::ListNodes(void)
{
 CXMLElement* p_rele = GetElementByPath("start/nodes",false);
 if( p_rele == NULL ){
    ES_ERROR("start/nodes element was not found","CInfoFile::ListNodes");
    return(false);
    }

 int i=0;
 CXMLIterator    I(p_rele);
 CXMLElement*    p_sele;

 while( (p_sele = I.GetNextChildElement("node")) != NULL ){
    CSmallString nodename;
    p_sele->GetAttribute("name",nodename);
    fprintf(stdout,"CPU %03d          : %s\n",i+1,(const char*)nodename);
    i++;
    }

 return(true);
}

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

bool __hipolyspec CInfoFile::RestoreEnvironment(void)
{
 bool result = true;
 result &= RestoreEnvironmentForElement(InfoFile.GetChildElementByPath("job_info/basic"));
 result &= RestoreEnvironmentForElement(InfoFile.GetChildElementByPath("job_info/specific"));
 return(result);
}

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

bool __hipolyspec CInfoFile::RestoreEnvironmentForElement(CXMLElement* p_ele)
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

bool __hipolyspec CInfoFile::PrintBasic(void)
{
 CSmallString tmp,tmp1,col;

 tmp = GetItem("basic/jobinput","INF_JOB_NAME");
 printf("Job name         : %s\n",(const char*)tmp);
 tmp = GetItem("submit/job","INF_JOB_ID");
 printf("Job ID           : %s\n",(const char*)tmp);
 tmp  = GetItem("basic/jobinput","INF_JOB_TITLE");
 tmp1 = GetItem("basic/jobinput","INF_JOB_TYPE");
 printf("Job title        : %s (Job type: %s)\n",(const char*)tmp,(const char*)tmp1);

 tmp  = GetItem("basic/jobinput","INF_JOB_MACHINE");
 tmp1 = GetItem("basic/jobinput","INF_JOB_PATH");
 printf("Job directory    : %s:%s\n",(const char*)tmp,(const char*)tmp1);

 col = GetItem("basic/collection","INF_COLLECTION_NAME");
 if( col == NULL ) col = "-none-";
 tmp = GetItem("basic/jobinput","INF_JOB_PROJECT");
 if( tmp == NULL ) tmp = "-none-";
 printf("Job project      : %s (Collection: %s)\n",(const char*)tmp,(const char*)col);

 tmp  = GetStampName();
 tmp1 = GetDriver();

 printf("Site name        : %s (Driver: %s)\n",(const char*)tmp,(const char*)tmp1);

 printf("========================================================\n");

 return(true);
}

//------------------------------------------------------------------------------

bool __hipolyspec CInfoFile::PrintResources(void)
{
 CSmallString tmp;

 tmp = GetItem("specific/resources","INF_ALIAS");
 if( tmp == NULL ){
 printf("Alias            : -none-\n");
    }
  else{
 printf("Alias            : %s\n",(const char*)tmp);
    }

 tmp = GetItem("specific/resources","INF_QUEUE");
 printf("Queue            : %s\n",(const char*)tmp);

 printf("----------------------------------------\n");
 tmp = GetItem("basic/arguments","INF_ARG_NCPU");
 if( tmp == NULL ){
 printf("NCPU requested   : 1\n");
    }
  else{
 printf("NCPU requested   : %s\n",(const char*)tmp);
    }

 tmp = GetItem("specific/resources","INF_NCPU");
 printf("NCPU calculated  : %s\n",(const char*)tmp);

 tmp = GetItem("specific/resources","INF_MAX_CPUS_PER_NODE");
 printf("Max CPUs / node  : %s\n",(const char*)tmp);

 tmp = GetItem("specific/resources","INF_NUM_OF_NODES");
 if( tmp != NULL ){
 printf("Number of nodes  : %s\n",(const char*)tmp);
    }

 tmp = GetItem("specific/resources","INF_PROPERTIES");
 if( tmp == NULL ){
 printf("Properties       : -none-\n");
    }
  else{
 printf("Properties       : %s\n",(const char*)tmp);
    }
 tmp = GetItem("specific/resources","INF_RESOURCES");
 if( tmp == NULL ){
 printf("Resources        : -job match-\n");
    }
  else{
 printf("Resources        : %s\n",(const char*)tmp);
    }
 tmp = GetItem("specific/resources","INF_SYNCMODE");
 printf("Sync mode        : %s\n",(const char*)tmp);
 printf("----------------------------------------\n");

 tmp = GetItem("basic/external","INF_EXTERNAL_START_AFTER");
 if( tmp == NULL ){
 printf("Start after      : -not defined-\n");
    }
  else{
 printf("Start after      : %s\n",(const char*)tmp);
    }

 tmp = GetItem("basic/modules","INF_EXPORTED_MODULES");
 if( tmp == NULL ){
 printf("Exported modules : -none-\n");
    }
  else{
 printf("Exported modules : %s\n",(const char*)tmp);
    }

 tmp = GetItem("basic/jobinput","INF_EXCLUDED_FILES");
 if( tmp == NULL ){
 printf("Excluded files   : -none-\n");
    }
  else{
 printf("Excluded files   : %s\n",(const char*)tmp);
    }

 printf("========================================================\n");

 return(true);
}

//------------------------------------------------------------------------------

bool __hipolyspec CInfoFile::PrintExec(void)
{
 CSmallString tmp;

 tmp = GetItem("start/workdir","INF_MAIN_NODE");
 printf("Main node        : %s\n",(const char*)tmp);
 tmp = GetItem("start/workdir","INF_WORK_DIR");
 printf("Work directory   : %s\n",(const char*)tmp);
 printf("----------------------------------------\n");

 ListNodes();

 printf("========================================================\n");

 return(true);
}

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

bool __hipolyspec CInfoFile::PrintCoInfo(const CSmallString& real_status)
{
 CXMLElement*       p_ele;
 CSmallString       tmp;
 CSmallTimeAndDate  section_time;
 CSmallTimeAndDate  curr_time;
 CSmallTime         diff_time;

 curr_time.GetActualTimeAndDate();

 tmp = "-unknown-";

 p_ele = GetElementByPath("kill",false);
 if( p_ele != NULL ){
    tmp = "killed";
    p_ele->GetAttribute("timedate",section_time);
    }
 else{
    p_ele = GetElementByPath("stop",false);
    if( p_ele != NULL ){
        tmp = "terminated";
        p_ele->GetAttribute("timedate",section_time);
        }
      else{
        p_ele = GetElementByPath("start",false);
        if( p_ele != NULL ){
            tmp = "started";
            p_ele->GetAttribute("timedate",section_time);
            }
          else{
            p_ele = GetElementByPath("submit",false);
            if( p_ele != NULL ){
                tmp = "submitted";
                p_ele->GetAttribute("timedate",section_time);
                }
              else{
                p_ele = GetElementByPath("basic",false);
                if( p_ele != NULL ){
                    tmp = "prepared";
                    p_ele->GetAttribute("timedate",section_time);
                    }
                }
            }
        }
    }

 diff_time = curr_time - section_time;

 CSmallString jt;
 jt = "single";

 tmp = GetItem("basic/jobinput","INF_JOB_TYPE");
 if( tmp == "precycle" ){
    jt = "recycle";
    }

 int tot = 1;
 int cur = 1;

 p_ele = GetElementByPath("basic/recycle",false);
 if( p_ele != NULL ){
    int start,stop;
    tmp = GetItem("basic/recycle","INF_RECYCLE_START");
    start = tmp.ToInt();
    tmp = GetItem("basic/recycle","INF_RECYCLE_STOP");
    stop = tmp.ToInt();
    tmp = GetItem("basic/recycle","INF_RECYCLE_CURRENT");
    cur = tmp.ToInt();
    tot = stop - start + 1;
    }

 CSmallString sdiff_time;
 sdiff_time = diff_time.GetSTimeAndDay();

 int fin = cur - 1;
 if( real_status == "F" ){
    fin += 1;
    sdiff_time = "";
    }
 int com = fin * 100 / tot;

//     Last change       Duration     Type     Dimm   Comp
// ------------------- ------------ -------- -------- ----

 printf("%19s %12s %8s  %3d/%3d %3d%%",
        (const char*)section_time.GetSDateAndTime(),
        (const char*)sdiff_time,
        (const char*)jt,cur,tot,com);

 return(true);
}

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

