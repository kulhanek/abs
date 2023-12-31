// ===============================================================================
// INF - Advanced Module System
// -------------------------------------------------------------------------------
//    Copyright (C) 2004,2005,2008 Petr Kulhanek, kulhanek@chemi.muni.cz
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
// ===============================================================================


#include "config-cli-opts.hpp"
#include <ErrorSystem.hpp>
#include <SmallTimeAndDate.hpp>
#include <TerminalStr.hpp>
#include <Shell.hpp>
#include <ctype.h>
#include <ABSConfig.hpp>
#include <Site.hpp>
#include <AliasList.hpp>
#include <PluginDatabase.hpp>
#include <User.hpp>
#include <QueueList.hpp>
#include <CategoryUUID.hpp>
#include <list>
#include <BatchServers.hpp>
#include <SiteController.hpp>
#include <Site.hpp>

using namespace std;

//------------------------------------------------------------------------------

CConfigCliOpts Options;

//------------------------------------------------------------------------------

int Init(int argc, char* argv[]);
bool Run(void);
bool Finalize(void);

//-----------------------------------------------

void PrintHeader(void);
void MainMenu(void);
void PrintMainMenu(void);
bool QuitProgram(void);

//-----------------------------------------------

int Question(const char *p_quest);
void GetTextValue(const char *p_epilog,CSmallString& value);

//-----------------------------------------------

void PrintTorqueMenu(void);
void TorqueMenu(void);
void SetEmailAbort(void);
void SetEmailStop(void);
void SetEmailStart(void);
void SetIgnoreRuntimeFiles(void);
void SetRerunableJobs(void);
void SetConfirmSubmit(void);
void RestoreSystemABSConfig(void);

// ----------------------------------------------

void PrintAliasMenu(void);
void AliasMenu(void);
void ListAliases(CXMLElement* p_ele);
void CreateOrModifyAlias(void);
void DeleteAlias(void);
void RestoreSystemAliases(void);

// ----------------------------------------------

void PrintAutoJobs();

//------------------------------------------------------------------------------

bool TorqueSetupChanged = false;
bool AliasSetupChanged = false;

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

int main(int argc, char* argv[])
{
    int result = Init(argc,argv);

    switch(result) {
    case SO_EXIT:
        return(0);
    case SO_CONTINUE:
        if( Run() == false ) {
            fprintf(stderr,"\n");
            ErrorSystem.PrintLastError("abs-config");;
            if( Options.GetOptVerbose() == false ) fprintf(stderr,"\n");
            Finalize();
            return(1);
        }
        Finalize();
        return(0);
    case SO_USER_ERROR:
        Finalize();
        return(2);
    case SO_OPTS_ERROR:
    default:
        return(3);
    }
}

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

int Init(int argc, char* argv[])
{
    // encode program options, all check procedures are done inside of CABFIntOpts
    int result = Options.ParseCmdLine(argc,argv);

    // should we exit or was it error?
    if( result != SO_CONTINUE ) return(result);

    return(SO_CONTINUE);
}

//------------------------------------------------------------------------------

bool Run(void)
{
    // check if site is active
    if( SiteController.GetActiveSite() == NULL ) {
        ES_ERROR("no site is active");
        return(false);
    }

    if( ABSConfig.LoadConfig() == false ){
        ES_ERROR("unable to load ABSConfig config");
        return(false);
    }

    CTerminalStr vout;

    // user must be initializaed before ABSConfig.IsUserTicketValid()
    User.InitUser();

    // check if user has valid ticket
    if( ABSConfig.IsUserTicketValid(vout) == false ){
        ES_TRACE_ERROR("user does not have valid ticket");
        return(false);
    }

    BatchServers.GetQueues();
    QueueList.RemoveDisabledQueues();
    QueueList.RemoveStoppedQueues();
    QueueList.RemoveInaccesibleQueues(User);
    QueueList.RemoveNonexecutiveQueues();
    QueueList.SortByName();

    // load site config
    // FIXME
//    if( Site.LoadConfig() == false) {
//        ES_ERROR("unable to load site config");
//        return(false);
//    }

    if( AliasList.LoadConfig() == false ){
        ES_ERROR("unable to load aliases");
        return(false);
    }

    // enter to menu driven configuration setup
    PrintHeader();
    MainMenu();

    return(true);
}

//------------------------------------------------------------------------------

bool Finalize(void)
{
    if( Options.GetOptVerbose() ) {
        ErrorSystem.PrintErrors(stderr);
        fprintf(stderr,"\n");
    }
    return(true);
}

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

void PrintHeader(void)
{
    printf("\n");
    printf("             ***  ABS Configuration Centre ***              \n");
    printf("          ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~         \n");
}

//------------------------------------------------------------------------------

void PrintMainMenu(void)
{
    printf("\n");
    printf(" Main menu\n");
    printf("------------------------------------------------------------\n");
    printf(" 1   - configure aliases\n");
    printf(" 2   - configure confirmation of job submission, e-mail alerts, etc.\n");
    printf(" 3   - list (auto)detected job types\n");
    printf("------------------------------------------------------------\n");
    printf(" i   - site info\n");
    printf(" s   - save changes\n");
    printf(" p   - print this menu once again\n");
    printf(" q/r - quit program\n");
    printf("\n");
}

//-----------------------------------------------------------------------------

void MainMenu(void)
{
    bool cont = true;
    PrintMainMenu();
    do {
        char buffer[80];
        printf(" Type menu item and press enter: ");
        if( fgets(buffer,79,stdin) == NULL ) continue;
        switch(buffer[0]) {
        case '1':
            AliasMenu();
            PrintMainMenu();
            break;

        case '2':
            TorqueMenu();
            PrintMainMenu();
            break;

        case '3':
            PrintAutoJobs();
            PrintMainMenu();
            break;

        case 'i': {
            CTerminalStr vout;
            vout.Attach(stdout);
            // FIXME
            // Site.PrintShortSiteInfo(vout);
            PrintMainMenu();
            }
            break;

        case 's': {
            bool result = true;
            ErrorSystem.RemoveAllErrors();
            if( TorqueSetupChanged ){
                result &= ABSConfig.SaveUserConfig();
            }
            if( AliasSetupChanged ){
                result &= AliasList.SaveUserConfig();
            }
            if( result == true ) {
                TorqueSetupChanged = false;
                AliasSetupChanged = false;
                printf("\n");
                printf(" >>> User configuration was successfully saved.\n");
                printf("\n");
            } else {
                printf("\n");
                printf(" ERROR: Unable to save your modified setup.\n");
                printf("\n");
                ErrorSystem.PrintErrors();
            }
        }
        break;
        case 'p':
            PrintMainMenu();
            break;
        case 'q':
        case 'r':
            cont = ! QuitProgram();
            printf("\n");
            break;
        default:
            printf("\n");
            printf(" ERROR: Unrecognized menu item, try again ...\n");
            printf("\n");
            break;
        }
    } while( cont == true );

    printf(" ABS configuration terminated on user request.\n");
    printf("\n");
}

//-----------------------------------------------------------------------------

bool QuitProgram(void)
{
    if( (TorqueSetupChanged == false) && (AliasSetupChanged == false)  ) return(true);

    for(;;) {
        printf("\n");
        printf(" WARNING: Configuration was changed!\n");
        printf("          Do you want to save it before program will be terminated (yes/no/skip)? ");
        char buffer[80];
        if( fgets(buffer,79,stdin) == NULL ) continue;

        if( strlen(buffer) > 0 ) {
            if( buffer[strlen(buffer)-1] == '\n' ) buffer[strlen(buffer)-1] = '\0';
        }

        if( strcmp(buffer,"skip") == 0 ) return(false);

        if( strcmp(buffer,"yes") == 0 ) {
            bool result = true;
            ErrorSystem.RemoveAllErrors();
            if( TorqueSetupChanged ){
                result &= ABSConfig.SaveUserConfig();
            }
            if( AliasSetupChanged ){
                result &= AliasList.SaveUserConfig();
            }
            if( result == true ) {
                TorqueSetupChanged = false;
                AliasSetupChanged = false;
                printf(" >>> User configuration was successfully saved.\n");
            } else {
                printf("\n");
                printf(" ERROR: Unable to save your modified setup.\n");
                printf("\n");
                ErrorSystem.PrintErrors();
            }
            return(result);
        }

        if( strcmp(buffer,"no") == 0 ) {
            return(true);
        }

        printf("\n");
        printf(" ERROR: Please specify only 'yes', 'no' or 'skip' keyword ! Try again ...\n");
    }

    return(false);
}

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

int Question(const char *p_quest)
{
    for(;;) {
        printf("\n");
        printf(" %s (yes/no/skip) ? ",p_quest);
        char buffer[80];
        if( fgets(buffer,79,stdin) == NULL ) continue;

        if( strlen(buffer) > 0 ) {
            if( buffer[strlen(buffer)-1] == '\n' ) buffer[strlen(buffer)-1] = '\0';
        }

        if( strcmp(buffer,"skip") == 0 ) return(-1);

        if( strcmp(buffer,"yes") == 0 ) {
            return(1);
        }

        if( strcmp(buffer,"no") == 0 ) {
            return(0);
        }

        printf("\n");
        printf(" ERROR: Please specify only 'yes', 'no' or 'skip' keyword! Try again ...\n");
    }
}

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

void PrintTorqueMenu(void)
{
 printf("\n");
 printf("                Torque driver configuration                 \n");
 printf("             ----------------------------------             \n");
 printf(" Item                                       Current value   \n");
 printf("----------------------------------------- ------------------\n");
 CSmallString     tmp;
 bool            system;
 char             syschar;
 ABSConfig.GetUserConfigItem("INF_CONFIRM_SUBMIT",tmp,system);
 if( system == true ) syschar = '*'; else syschar = ' ';
 printf(" 1. Confirm submit ......................  %s %c\n",(const char*)tmp,syschar);

 ABSConfig.GetUserConfigItem("INF_RERUNABLE_JOBS",tmp,system);
 if( system == true ) syschar = '*'; else syschar = ' ';
 printf(" 2. Rerunable jobs ......................  %s %c\n",(const char*)tmp,syschar);

 ABSConfig.GetUserConfigItem("INF_IGNORE_RUNTIME_FILES",tmp,system);
 if( system == true ) syschar = '*'; else syschar = ' ';
 printf(" 3. Ignore runtime files ................  %s %c\n",(const char*)tmp,syschar);

 ABSConfig.GetUserConfigItem("INF_START_EMAIL",tmp,system);
 if( system == true ) syschar = '*'; else syschar = ' ';
 printf(" 4. Send e-mail when job is started .....  %s %c\n",(const char*)tmp,syschar);

 ABSConfig.GetUserConfigItem("INF_STOP_EMAIL",tmp,system);
 if( system == true ) syschar = '*'; else syschar = ' ';
 printf(" 5. Send e-mail when job is finished ....  %s %c\n",(const char*)tmp,syschar);

 ABSConfig.GetUserConfigItem("INF_ABORT_EMAIL",tmp,system);
 if( system == true ) syschar = '*'; else syschar = ' ';
 printf(" 6. Send e-mail when job is aborted .....  %s %c\n",(const char*)tmp,syschar);
 printf("------------------------------------------------------------\n");
 printf(" o - Restore system setup\n");
 printf("                            (* - system wide configuration) \n");
 printf("------------------------------------------------------------\n");
 printf(" p - Print this menu once again\n");
 printf(" q/r - Return to main menu\n");
 printf("\n");
}

//-----------------------------------------------------------------------------

void TorqueMenu(void)
{
 bool cont = true;
 PrintTorqueMenu();
 do{
    char buffer[80];
     printf(" Type menu item and press enter: ");
    if( fgets(buffer,79,stdin) != NULL ) {
        switch(buffer[0]){
            case '1':
                SetConfirmSubmit();
                PrintTorqueMenu();
                break;
            case '2':
                SetRerunableJobs();
                PrintTorqueMenu();
                break;
            case '3':
                SetIgnoreRuntimeFiles();
                PrintTorqueMenu();
                break;
            case '4':
                SetEmailStart();
                PrintTorqueMenu();
                break;
            case '5':
                SetEmailStop();
                PrintTorqueMenu();
                break;
            case '6':
                SetEmailAbort();
                PrintTorqueMenu();
                break;
            case 'o':
                RestoreSystemABSConfig();
                PrintTorqueMenu();
                break;

            case 'p':
                PrintTorqueMenu();
                break;
            case 'r':
            case 'q':
                cont=false;
                break;
            default:
                printf("\n");
                printf(" ERROR: Unrecognized menu item, try again ...\n");
                printf("\n");
                break;
            }
        }
    }while( cont == true );
 printf("\n");
}

//-----------------------------------------------------------------------------

void SetConfirmSubmit(void)
{
 int result = Question("Do you want to confirm submit");

 switch(result){
    case 0:
        ABSConfig.SetUserConfigItem("INF_CONFIRM_SUBMIT","NO");
        TorqueSetupChanged = true;
        break;
    case 1:
        ABSConfig.SetUserConfigItem("INF_CONFIRM_SUBMIT","YES");
        TorqueSetupChanged = true;
        break;
    case -1:
        // nothing to do
        break;
    }
}

//-----------------------------------------------------------------------------

void SetRerunableJobs(void)
{
 int result = Question("Do you want to permit rerunable jobs");

 switch(result){
    case 0:
        ABSConfig.SetUserConfigItem("INF_RERUNABLE_JOBS","NO");
        TorqueSetupChanged = true;
        break;
    case 1:
        ABSConfig.SetUserConfigItem("INF_RERUNABLE_JOBS","YES");
        TorqueSetupChanged = true;
        break;
    case -1:
        // nothing to do
        break;
    }
}

//-----------------------------------------------------------------------------

void SetIgnoreRuntimeFiles(void)
{
 int result = Question("Do you want to ignore runtime files");

 switch(result){
    case 0:
        ABSConfig.SetUserConfigItem("INF_IGNORE_RUNTIME_FILES","NO");
        TorqueSetupChanged = true;
        break;
    case 1:
        ABSConfig.SetUserConfigItem("INF_IGNORE_RUNTIME_FILES","YES");
        TorqueSetupChanged = true;
        break;
    case -1:
        // nothing to do
        break;
    }
}

//-----------------------------------------------------------------------------

void SetEmailStart(void)
{
 int result = Question("Do you want to be notified by e-mail when job is started");

 switch(result){
    case 0:
        ABSConfig.SetUserConfigItem("INF_START_EMAIL","NO");
        TorqueSetupChanged = true;
        break;
    case 1:
        ABSConfig.SetUserConfigItem("INF_START_EMAIL","YES");
        TorqueSetupChanged = true;
        break;
    case -1:
        // nothing to do
        break;
    }
}

//-----------------------------------------------------------------------------

void SetEmailStop(void)
{
 int result = Question("Do you want to be notified by e-mail when job is finished");

 switch(result){
    case 0:
        ABSConfig.SetUserConfigItem("INF_STOP_EMAIL","NO");
        TorqueSetupChanged = true;
        break;
    case 1:
        ABSConfig.SetUserConfigItem("INF_STOP_EMAIL","YES");
        TorqueSetupChanged = true;
        break;
    case -1:
        // nothing to do
        break;
    }
}

//-----------------------------------------------------------------------------

void SetEmailAbort(void)
{
 int result = Question("Do you want to be notified by e-mail when job is aborted");

 switch(result){
    case 0:
        ABSConfig.SetUserConfigItem("INF_ABORT_EMAIL","NO");
        TorqueSetupChanged = true;
        break;
    case 1:
        ABSConfig.SetUserConfigItem("INF_ABORT_EMAIL","YES");
        TorqueSetupChanged = true;
        break;
    case -1:
        // nothing to do
        break;
    }
}

//-----------------------------------------------------------------------------

void RestoreSystemABSConfig(void)
{
 int result = Question("Do you want to restore system setup for pbs driver");

 switch(result){
    case 0:
        break;
    case 1:
        ABSConfig.RemoveUserConfig();
        TorqueSetupChanged = true;
        break;
    case -1:
        // nothing to do
        break;
    }
}

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

void PrintAliasMenu(void)
{
    printf("\n");
    printf(" Aliases menu\n");
    printf("------------------------------------------------------------\n");
    printf(" l - list aliases                                           \n");
    printf(" c - create or modify user alias                            \n");
    printf(" d - delete user alias                                      \n");
    printf(" o - restore system aliases (user aliases will be deleted!) \n");
    printf("------------------------------------------------------------\n");
    printf(" p - Print this menu once again\n");
    printf(" q/r - Return to main menu\n");
    printf("\n");
}

//-----------------------------------------------------------------------------

void AliasMenu(void)
{
    bool cont = true;
    PrintAliasMenu();
    do{
        char buffer[80];
        printf(" Type menu item and press enter: ");
        if( fgets(buffer,79,stdin) != NULL ){
        switch(buffer[0]){
                case 'l':{
                    CTerminalStr tout;
                    tout.Attach(stdout);
                    tout << std::endl;
                    AliasList.PrintInfos(tout);
                    PrintAliasMenu();
                    }
                    break;
                case 'c':
                    CreateOrModifyAlias();
                    PrintAliasMenu();
                    break;
                case 'd':
                    DeleteAlias();
                    PrintAliasMenu();
                    break;
                case 'o':
                    RestoreSystemAliases();
                    PrintAliasMenu();
                    break;
                case 'p':
                    PrintAliasMenu();
                    break;
                case 'r':
                case 'q':
                    cont=false;
                    break;
                default:
                    printf("\n");
                    printf(" ERROR: Unrecognized menu item, try again ...\n");
                    printf("\n");
                    break;
            }
        }
    } while ( cont == true );
    printf("\n");
}

//-----------------------------------------------------------------------------

void CreateOrModifyAlias(void)
{
    CSmallString    name;
    CSmallString    queue;
    CSmallString    res;

    printf("\n");
    printf(" Create or modify alias\n");
    printf("------------------------------------------------------------\n");

    CTerminalStr tout;
    tout.Attach(stdout);

    printf("\n");
    printf("##### Allowed queues ...");
    QueueList.PrintInfos(tout);

    printf("\n");
    printf("##### Allowed destinations ...\n");
    printf(" queue\n");
    printf(" queue[@S]\n");
    printf(" queue[@server]\n");

    printf("\n");
    printf("##### Allowed resource tokens ...\n ");
    // FIXME
    // ABSConfig.PrintAllowedResTokens(tout);
    printf("\n\n");
    printf(" Multiple resources: resource1=value1,resource2=value2,...\n");

    printf("\n");
    GetTextValue(" Name of alias (or empty string to return back): ",name);
    if( name == NULL ) return;

    GetTextValue(" Type destination : ",queue);
    GetTextValue(" Type resources   : ",res);

    if( AliasList.AddAlias(tout,name,queue,res,false) == true ){
        printf("\n");
        printf(" The alias was successfully created!\n");
        AliasSetupChanged = true;
    } else {
        printf("\n");
        printf(" The alias was not created!\n");
    }
}

//-----------------------------------------------------------------------------

void DeleteAlias(void)
{
    CSmallString  name;

    printf("\n");
    printf(" Delete alias\n");
    printf("------------------------------------------------------------\n");
    GetTextValue(" Name of alias (or empty string to return back): ",name);
    if( name == NULL ) return;

    if( AliasList.FindAlias(name) == NULL ){
        printf("\n");
        printf(" ERROR: Alias '%s' is not defined therefore it cannot be deleted!\n", (const char*)name);
        printf("\n");
        return;
    }

    AliasList.RemoveAlias(name);
    AliasSetupChanged = true;
}

//-----------------------------------------------------------------------------

void RestoreSystemAliases(void)
{
    int result = Question("Do you want to restore system aliases");

    switch(result){
        case 0:
            break;
        case 1:
            AliasList.ClearAllAliases();
            AliasList.LoadSystemConfig();
            printf(" The system aliases were restored\n");
            printf("\n");
            AliasSetupChanged = true;
            break;
        case -1:
            // nothing to do
            break;
    }
}

//-----------------------------------------------------------------------------

void GetTextValue(const char *p_epilog,CSmallString& value)
{
    char buffer[255];
    printf("%s",p_epilog);
    value = "";
    if( fgets(buffer,254,stdin) == NULL ) return;
    if( strlen(buffer) > 0 ){
        if( buffer[strlen(buffer)-1] == '\n' ) buffer[strlen(buffer)-1] = '\0';
    }
    value = buffer;
}

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

struct SJobType {
    string type;
    string mode;
    string descr;
    CUUID  uuid;
};

bool JobTypeCompare(const SJobType& left,const SJobType& right)
{
    if( left.mode < right.mode ) return(true);
    if( left.mode == right.mode) {
        return( left.type < right.type );
    }
    return(false);
}

//------------------------------------------------------------------------------

void PrintAutoJobs(void)
{
    printf("\n");
    printf("# List of (auto)detected job types ...\n");
    printf("\n");
    printf("#   Job Type      Mode   Short description\n");
    printf("# ------------- -------- -------------------------------------------------------\n");

    CSimpleIteratorC<CPluginObject> I(PluginDatabase.GetObjectList());
    CPluginObject* p_pobj;

    list<SJobType> jobtypes;

    while( (p_pobj = I.Current()) != NULL ){
        I++;
        if( p_pobj->GetCategoryUUID() != JOB_TYPE_CAT ) continue;
        CSmallString type;
        if( PluginDatabase.FindObjectConfigValue(p_pobj->GetObjectUUID(),"_type",type) == false ){
            continue;
        }
        CSmallString mode;
        if( PluginDatabase.FindObjectConfigValue(p_pobj->GetObjectUUID(),"_mode",mode) == false ){
            continue;
        }
        SJobType jobtype;
        jobtype.type = type;
        jobtype.mode = mode;
        jobtype.descr = p_pobj->GetDescription();
        jobtype.uuid = p_pobj->GetObjectUUID();

        jobtypes.push_back(jobtype);
    }

    jobtypes.sort(JobTypeCompare);

    list<SJobType>::iterator it = jobtypes.begin();
    list<SJobType>::iterator ie = jobtypes.end();

    while( it != ie ){
        SJobType jobtype = *it;
        printf("%-15s %8s %s\n",jobtype.type.c_str(),jobtype.mode.c_str(),jobtype.descr.c_str());
        it++;
    }

    printf("\n");
    printf("explicit mode = it must be specified explicitly in the job input file\n");
    printf("implicit mode = the job type is determined from the job input file or the name of the job input file\n");

}

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================


