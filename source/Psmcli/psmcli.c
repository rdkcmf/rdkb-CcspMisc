/*
 * If not stated otherwise in this file or this component's Licenses.txt file the
 * following copyright and licenses apply:
 *
 * Copyright 2015 RDK Management
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
*/

/**********************************************************************
   Copyright [2014] [Cisco Systems, Inc.]
 
   Licensed under the Apache License, Version 2.0 (the "License");
   you may not use this file except in compliance with the License.
   You may obtain a copy of the License at
 
       http://www.apache.org/licenses/LICENSE-2.0
 
   Unless required by applicable law or agreed to in writing, software
   distributed under the License is distributed on an "AS IS" BASIS,
   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
   See the License for the specific language governing permissions and
   limitations under the License.
**********************************************************************/

/* psmcli - Tool to read and write object-values to persistant storage (PS) */

/* INCLUDES */
#include <stdio.h>              // for fprintf, sprintf, printf
#include <stdlib.h>             // for open, close, exit, _exit, getpid
#include <string.h>             // for strcmp, strcat, strcpy, strlen

#include "ccsp_message_bus.h"   // for CCSP_Message_Bus_Init/Exit
#include "ccsp_memory.h"        // for AnscAllocate/FreeMemory
#include "ccsp_psm_helper.h"    // for PSM_Get/Set_Record_Value2 
#include "ccsp_psmcli.h"        // for extern psmcli_debug_level psmcli_debug_print

#include <signal.h>             // for sigaction

#include "ccsp_trace.h"         // for CcspTraceXYZ

#include "safec_lib_common.h"

// #define PSMCLI_TESTING_LOCAL
#ifndef PSMCLI_TESTING_LOCAL
    #include "ccsp_base_api.h"      // for definitions of CCSP_INVALID_PSMCLI_CMD and CCSP_MSG_BUS_CFG
    static char const * const psmcli_debug_file_name    = "/nvram/psmcli_debug_level";
#else 
    #define CCSP_INVALID_PSMCLI_CMD  209
    #define CCSP_MSG_BUS_CFG         "/home/rutian/work/intel_usg/CcspCommonLibrary/boards/pc/ccsp_msg.cfg"
    static char const * const psmcli_debug_file_name    = "./psmcli_debug_level";
#endif

#define TYPE_STRING_SIZE 16

#ifdef INCLUDE_BREAKPAD
#include "breakpad_wrapper.h"
#endif

/* DEBUG FLAG */
#ifdef _DEBUG
// #define _DEBUG_LOCAL  // to add more debug messages
#endif

/* EXTERNAL */
// for psmcli debug
// defined in "ccsp_psmcli.h"
extern psmcli_debug_level psmcli_debug_print; 

/* CONSTANTS */
#define PSMCLI_SUCCESS                              1
#define PSMCLI_FAIL                                 0
#define PSMCLI_CMD_LEN_MAX                          16
#define PSMCLI_SUBSYSTEM_PREFIX_DEFAULT             "eRT."
#define PSMCLI_STRLEN_MAX                           256
#define TYPE_FORMAT_CCSP                            1
#define TYPE_FORMAT_STRING                          2
#define NUM_CCSP_TYPES                          ( sizeof(ccsp_type_table) / sizeof(ccsp_type_table[0]) )

/*Structure defined to get the log level type from the given Log Names */

typedef struct ccsp_type_struct {
  char     *ccspName;
  unsigned int      ccspType;
} CCSP_TYPE_STRUCT;

CCSP_TYPE_STRUCT ccsp_type_table[] = {
	{ "int",			ccsp_int },
	{ "string",			ccsp_string },
	{ "uint",			ccsp_unsignedInt },
	{ "bool",			ccsp_boolean },
	{ "datetime",		ccsp_dateTime },
	{ "ccsp_base64",	ccsp_base64 },
	{ "long",			ccsp_long },
	{ "ulong",			ccsp_unsignedLong },
	{ "float",			ccsp_float },
	{ "double",			ccsp_double },
	{ "byte",			ccsp_byte }
};


// Help / usage menu. 
// Do not modify the order, add more items to the end before NULL
static char const *help_usage_desc[] = 
{
    "Help / Usage:",
    "psmcli help",
    "psmcli [subsys <prefix> | nosubsys] get <obj1 name> <obj2 name> …",
    "psmcli [subsys <prefix> | nosubsys] getdetail <obj1 name> <obj 2 name> …",
    "psmcli [subsys <prefix> | nosubsys] get –e <env var1> <obj1 name> <env var2> <obj2 name> …",
    "psmcli [subsys <prefix> | nosubsys] getdetail –e <env var1> <obj1 name> <env var2> <obj2 name> …",
    "psmcli [subsys <prefix> | nosubsys] set <obj1 name> <obj1 value> <obj2 name> <obj2 value> …",
    "psmcli [subsys <prefix> | nosubsys] setdetail <obj1 datatype> <obj1 name> <obj1 value> <obj2 datatype> <obj2 name> <obj2 value> … ",
    "psmcli [subsys <prefix> | nosubsys] del <obj1 name> <obj2 name> …",
    "psmcli [subsys <prefix> | nosubsys] getinstcnt <obj1 name> <obj2 name> <obj3 name> …",
    "psmcli [subsys <prefix> | nosubsys] getallinst <obj name>",
    "NOTE: default sub-system prefix is \"eRT.\", if neither subsys nor nosubsys is used.",
    NULL
};

/* TYPEDEFS */
// Commands jump table structure 
typedef struct CmdsTable {
    char cmd[PSMCLI_CMD_LEN_MAX];
    unsigned int (*process_cmd)(int const argCnt, const char * const argVars[], char const * const busHandle);
} cmdsTable_s;

/* FUNCTION PROTOTYPES */
// no external interface defined, so all functions should be local
static void help_usage();
#ifndef INCLUDE_BREAKPAD
static void ccsp_exception_handler(int sig, siginfo_t *info, void *context);
static void enable_ccsp_exception_handlers();
#endif
static unsigned int get_type_info(unsigned int *ccspType, char **typeString, int const typeFormat);
static psmcli_debug_level psmcli_get_debug_level(char const *file_name);
// static inline void process_show();
// static int psmcli_bus_name_in_use(char const *component_id, char  const *config_file); 

unsigned int process_get(int const argCnt, char const * const argVars[], char const * const busHandle);
unsigned int process_getdetail(int const argCnt, char const * const argVars[], char const * const busHandle);
unsigned int process_get_e(int const argCnt, char const * const argVars[], char const * const busHandle);
unsigned int process_getdetail_e(int const argCnt, char const * const argVars[], char const * const busHandle);
unsigned int process_set(int const argCnt, char const * const argVars[], char const * const busHandle);
unsigned int process_setdetail(int const argCnt, char const * const argVars[], char const * const busHandle);
unsigned int process_del(int const argCnt, char const * const argVars[], char const * const busHandle);
unsigned int process_getallinst(int const argCnt, char const * const argVars[], char const * const busHandle);
unsigned int process_getinstcnt(int const argCnt, char const * const argVars[], char const * const busHandle);

/* GLOBAL VAR */
static char const * const psmcli_component_id       = "ccsp.psmclient";
static char subsys_prefix[PSMCLI_STRLEN_MAX]        = {0};
static char prog_name[PSMCLI_STRLEN_MAX]            = "PsmCli";
static char cmdLine[PSMCLI_STRLEN_MAX]              = {0};

// Commands jump table
// Do not alter the sequence, add new commands at the end
static const cmdsTable_s cmdsTable[] = {
    { "get", process_get },
    { "getdetail", process_getdetail },
    { "get -e", process_get_e },
    { "getdetail -e", process_getdetail_e },
    { "set", process_set },
    { "setdetail", process_setdetail },
    { "del", process_del },
    { "getallinst", process_getallinst },
    { "getinstcnt", process_getinstcnt }
};
    
/* IMPLMENTATION */
int main(int argc, char**argv)
{
    char    cmdConcat[PSMCLI_CMD_LEN_MAX]    = {0};
    char    *pCfg                            = CCSP_MSG_BUS_CFG;
    void    *bus_handle                      = NULL;
    unsigned int     tmpLen                  = 0;
    int     cmdTableLen                      = sizeof(cmdsTable) / sizeof(cmdsTable_s);
    int     ret                              = 0;
    char    component_id[256]                = {0} ;
    int     local_argc                       = argc;
    char    **local_argv                     = argv;
    int     i                                = 0;
    errno_t rc                               = -1;
    int     ind                              = -1;

#ifdef INCLUDE_BREAKPAD
    breakpad_ExceptionHandler();
#else
    enable_ccsp_exception_handlers();
#endif

    sprintf(prog_name, "PsmCli.pid%d", getpid());

    // check debug print levell
    psmcli_debug_print = psmcli_get_debug_level(psmcli_debug_file_name);

    // save command line
    for(i=0; i<(argc-1) && (strlen(argv[i])+strlen(cmdLine)+2)<=PSMCLI_STRLEN_MAX; i++) {
        rc = strcat_s(cmdLine, sizeof(cmdLine), argv[i]);
        if( rc != EOK )
        {
           ERR_CHK(rc);
           exit( 1 );
        }

        rc = strcat_s(cmdLine, sizeof(cmdLine), " ");
		if( rc != EOK )
        {
           ERR_CHK(rc);
           exit( 1 );
        }
    }
    if(( i == argc-1 ) && (strlen(argv[i])+strlen(cmdLine)+1)<=PSMCLI_STRLEN_MAX)
    {
        rc = strcat_s(cmdLine, sizeof(cmdLine), argv[argc-1]);
        if( rc != EOK )
        {
           ERR_CHK(rc);
           exit( 1 );
        }
    }

#ifdef PSMCLI_TESTING_LOCAL
    // Echo input
    CcspTraceDebug(("<%s>: invocation = '%s'", prog_name, cmdLine));
#endif

    // Check if the number of args is >= 2
    if(argc < 3) {

        help_usage();

        if(argc == 2) {
			rc = strcmp_s( "help", strlen( "help" ), argv[1], &ind );
			ERR_CHK(rc);
			if(( !ind ) && ( rc == EOK ))
			{
			   exit(0);
			}
			else
			{
			   exit(CCSP_ERR_INVALID_ARGUMENTS);
			}
        } else {
            exit(CCSP_ERR_INVALID_ARGUMENTS);
        }
    }
    
    // try to set the subsystem prefix
    rc = strcmp_s( "nosubsys", strlen("nosubsys"), argv[1], &ind );
    ERR_CHK(rc);
    if (( ind == 0 ) && ( rc == EOK ))  {
        if(argc < 4) { // must be followed by a cmd
 	    help_usage();
	    exit(CCSP_ERR_INVALID_ARGUMENTS);
        }
        subsys_prefix[0] = '\0';  
        local_argc = argc - 1;
        local_argv = argv + 1;
    }
    else {
        rc = strcmp_s( "subsys", strlen("subsys"), argv[1], &ind );
        ERR_CHK(rc);
        if (( ind == 0 ) && ( rc == EOK ))  {
        if(argc < 5) { // must be followed by a string and then a cmd
 	    help_usage();
	    exit(CCSP_ERR_INVALID_ARGUMENTS);
        }
        else {
            rc = strcpy_s(subsys_prefix, sizeof(subsys_prefix), argv[2]);
            if( rc != EOK )
            {
               ERR_CHK(rc);
               exit( 1 );
            }

            subsys_prefix[255] = '\0';  // in case it is not terminated
            local_argc = argc - 2;
            local_argv = argv + 2;
        }
       }
       else { // no subsys nor nosubsys specified, use default prefix
           rc = strcpy_s(subsys_prefix, sizeof(subsys_prefix), PSMCLI_SUBSYSTEM_PREFIX_DEFAULT);
           if( rc != EOK )
           {
              ERR_CHK(rc);
              exit( 1 );
           }
           local_argc = argc;
           local_argv = argv;
       }
    }

    // try to get an unique name for the connection
    sprintf(component_id, "%s.pid%d", psmcli_component_id, getpid()); 

    // Assuming the component_id generated with pid is unique, 
    // So skip the checking to increase speed. RTian 6/19/2013
    /*
    {

    // #define PSMCLI_SUBSYSTEM_PREFIX_GEN_RAND_MAX        997     // a prime close to 1000
    // #define PSMCLI_SUBSYSTEM_PREFIX_GEN_NTRY_MAX        20
    // #include <time.h>

        int nTry = 0, ret = 0;

        srand(time(NULL)*getpid());  // seed the random number generator

        do {
            // each section in component id separated by . cannot start with a number!!! 
            sprintf
                (
                    component_id, 
                    "%s.pid%d.sub%d", 
                    psmcli_component_id, 
                    getpid(), 
                    rand()%PSMCLI_SUBSYSTEM_PREFIX_GEN_RAND_MAX
                ); 
        
            // check if the id is in use already
            ret = psmcli_bus_name_in_use(component_id, pCfg); 
            if(ret < 0 || ret == 1) { nTry++; }
            else break;  // if(ret == 0)

        } while (nTry < PSMCLI_SUBSYSTEM_PREFIX_GEN_NTRY_MAX);

        if (nTry == PSMCLI_SUBSYSTEM_PREFIX_GEN_NTRY_MAX)  
            CcspTraceWarning(("<%s> Error: cannot generate unique id.  Id to be used is %s\n", prog_name, component_id));
    }
    */

    //    CcspTraceDebug(("<%s>: unique component_id = %s\n", prog_name, component_id));

    // Connect to  Dbus and get bus_handle
    // we begin the initiation of dbus    
#ifdef DBUS_INIT_SYNC_MODE
    ret = CCSP_Message_Bus_Init_Synced(component_id, 
                                       pCfg, 
                                       &bus_handle, 
                                       (CCSP_MESSAGE_BUS_MALLOC)Ansc_AllocateMemory_Callback, 
                                       Ansc_FreeMemory_Callback);
#else
    ret = CCSP_Message_Bus_Init(component_id, 
                                pCfg, 
                                &bus_handle, 
                                (CCSP_MESSAGE_BUS_MALLOC)Ansc_AllocateMemory_Callback, 
                                Ansc_FreeMemory_Callback);
#endif
    
    if ( ret == -1 )
    {
        // Dbus connection error
        // Comment below
        CcspTraceWarning(("<%s> Error: DBUS connection error, returned %d,  exitting w/ %d = CCSP_MESSAGE_BUS_CANNOT_CONNECT\n", 
                        prog_name, ret, CCSP_MESSAGE_BUS_CANNOT_CONNECT));
        exit(CCSP_MESSAGE_BUS_CANNOT_CONNECT);
    }

    //    CcspTraceDebug(("<%s>: Message_Bus_Init ok.\n", prog_name));

    // Check if commands are "get -e" or "getdetail -e"
    if((strlen(local_argv[2]) == strlen("-e")) && 
       (!strncmp(local_argv[2], "-e", strlen("-e")))) {
    	// "get -e" or "getdetail -e" command
    	tmpLen = strlen(local_argv[1]);
        
    	// Concatenate command and option
    	if(tmpLen <= (PSMCLI_CMD_LEN_MAX - strlen(" -e"))) {
            rc = strcpy_s(cmdConcat, sizeof( cmdConcat ), local_argv[1]);
            if( rc != EOK )
            {
               ERR_CHK(rc);
               exit( 1 );
            }
            rc = strcat_s(cmdConcat, sizeof(cmdConcat), " -e");
            if( rc != EOK )
            {
               ERR_CHK(rc);
               exit( 1 );
            }

            cmdConcat[PSMCLI_CMD_LEN_MAX-1] = '\0';
            
            i = 0;
            // Search command jumop tables
            while(i < cmdTableLen) {
                if(!strncmp(cmdsTable[i].cmd, cmdConcat, PSMCLI_CMD_LEN_MAX)) {
                    flockfile(stdout);
                    ret = cmdsTable[i].process_cmd(local_argc, (char const * const *)local_argv, bus_handle);
                    funlockfile(stdout);
                    break;
                }
                i++;
            }
            
            if(i == cmdTableLen) {
                CcspTraceWarning(("<%s>: unknown cmd %s, exiting with %d = CCSP_INVALID_PSMCLI_CMD\n", 
                                prog_name, cmdConcat, CCSP_INVALID_PSMCLI_CMD));
                ret = CCSP_INVALID_PSMCLI_CMD;
                goto  EXIT;
            }
            
    	} else {
            CcspTraceWarning(("<%s> Error: Invalid command usage, exiting w/ %d = CCSP_ERR_INVALID_ARGUMENTS\n", 
                            prog_name, CCSP_ERR_INVALID_ARGUMENTS));
            ret = CCSP_ERR_INVALID_ARGUMENTS;
            goto  EXIT;
    	}
        // Check for other commands - get, getdetail, set, setdetail, del
    } else {
    	
    	// Search commands jump table
    	i = 0;
    	while(i < cmdTableLen) {
            if(!strncmp(cmdsTable[i].cmd, local_argv[1], PSMCLI_CMD_LEN_MAX)) {
                flockfile(stdout);
                ret = cmdsTable[i].process_cmd(local_argc, (char const * const *)local_argv, bus_handle);
                funlockfile(stdout);
                break;
            }
            i++;
    	}
        
    	if(i == cmdTableLen) {
            CcspTraceWarning(("<%s>: unknown cmd %s, exiting with %d = CCSP_INVALID_PSMCLI_CMD\n", 
                            prog_name, local_argv[1], CCSP_INVALID_PSMCLI_CMD));
            ret = CCSP_INVALID_PSMCLI_CMD;
            goto  EXIT;
    	}
    }

EXIT:
    CCSP_Message_Bus_Exit(bus_handle);

#ifdef PSMCLI_TESTING_LOCAL
    // Echo return value
    CcspTraceDebug(("<%s>: final return value = %d\n", prog_name, ret));
#endif

    exit(ret);
    // return ret;
}
#ifndef INCLUDE_BREAKPAD
static int is_core_dump_opened(void)
{
    FILE *fp;
    char path[256];
    char line[1024];
    char *start, *tok, *sp;
#define TITLE   "Max core file size"

    snprintf(path, sizeof(path), "/proc/%d/limits", getpid());
    if ((fp = fopen(path, "rb")) == NULL)
        return 0;

    while (fgets(line, sizeof(line), fp) != NULL) {
        if ((start = strstr(line, TITLE)) == NULL)
            continue;

        start += strlen(TITLE);
        if ((tok = strtok_r(start, " \t\r\n", &sp)) == NULL)
            break;

        fclose(fp);

	return (tok[0] == '0' && tok[1] == '\0') ? 0 : 1;
    }

    fclose(fp);
    return 0;
}

static void ccsp_exception_handler(int sig, siginfo_t *info, void *context)
{
    UNREFERENCED_PARAMETER(context);
    int fd1 = -1;
    pid_t pid = getpid();
    char mapsFile[32]     = {0};
    char cmdFile[32]      = {0};
    char cmdName[32]      = {0}; 
    time_t rawtime;
    struct tm * timeinfo;
    errno_t rc = -1;

    sprintf( mapsFile, "/proc/%d/maps",    pid );
    sprintf( cmdFile,  "/proc/%d/cmdline", pid );

    /* Get current time */
    time ( &rawtime );
    timeinfo = localtime ( &rawtime );   
    
    /* Get command name */
    fd1 = open( cmdFile, O_RDONLY );
    if( fd1 != -1 ) /*RDKB-7441, CID-33230, validating the file handle*/
    {
        /*CID: 72093 Ignoring number of bytes read*/
        if (read(fd1, cmdName, sizeof(cmdName)-1 ) < 0) {
            fprintf( stderr, "ccsp_exception_handler: File read error");
        }
        close(fd1);
    }

    /* dump general information */
    fprintf( stderr, "\n!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!" );
    fprintf( stderr, "\n!!!!!!!!!!!!!!!!!!!!!!!!!!!! Exception Caught !!!!!!!!!!!!!!!!!!!!!!!!!!!!" );
    fprintf( stderr, "\n!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!" );
    fprintf( stderr, "\n\nSignal info:\n"
                        "\tTime: %s "
                        "\tProcess name: <%s>\n"
                        "\tPID: %d\n"
                        "\tFault Address: %p\n"
                        "\tSignal: %d \n"
                        "\tSignal Code: %d\n"
                        "\tLast errno: %d:%s \n"
                        "\tLast error (by signal): %d\n", 
                        asctime (timeinfo),
                        cmdName, 
                        pid,
                        info->si_addr,
                        sig,
                        info->si_code,
                        errno, strerror(errno),
                        info->si_errno );

    fprintf( stderr, "\nThe cmd line is :%s.\n", cmdLine );
    //    fprintf( stderr, "\nThe latest Line number is:%d.\n", runSteps);
    
    /* Output maps information in order to locate crash pointer */
    fd1 = open( mapsFile, O_RDONLY );
    if( fd1 != -1 ) /*RDKB-7441, CID-33230, validating the file handle*/
    {
        unsigned char    buf[ 512 ] = {0};
        unsigned int     readBytes = 0;

        /* Read the maps file */
        fprintf(stderr, "\n/proc/%d/maps:\n", pid);
        while( (readBytes = read( fd1, buf, 510 ) ) > 0 )
        {
            fprintf(stderr, "%s", buf);
            rc = memset_s(buf, sizeof(buf), 0, sizeof(buf));
            ERR_CHK(rc);
        }

        close(fd1);
    }

    fprintf( stderr, "\n!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!" );
    fprintf( stderr, "\n!!!!!!!!!!!!!!!!!!!!!!!!!!!! Dump Ending!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!" );
    fprintf( stderr, "\n!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n");
    fflush(stderr);

    _exit(1);
}

static void enable_ccsp_exception_handlers( )
{

    struct sigaction sigact;
    memset(&sigact, 0, sizeof(sigaction));

    sigact.sa_sigaction = ccsp_exception_handler;
    sigact.sa_flags = SA_RESTART | SA_SIGINFO;
    
    if (is_core_dump_opened())
    {
        //        sigaction(SIGUSR1,  &sigact, 0L); 
        //        CcspTraceInfo(("Core dump is opened, do not catch signal\n"));
    }
    else
    {
        //        CcspTraceInfo(("Core dump is NOT opened, backtrace if possible\n"));
    
        sigaction(SIGINT,  &sigact, 0L); 
        sigaction(SIGSEGV, &sigact, 0L); 
        sigaction(SIGILL,  &sigact, 0L); 
        sigaction(SIGBUS,  &sigact, 0L); 
        sigaction(SIGQUIT, &sigact, 0L);         

        /*
        sigaction(SIGTERM,  &sigact, 0L); 
        sigaction(SIGCHLD,  &sigact, 0L); 
        sigaction(SIGUSR1,  &sigact, 0L); 
        sigaction(SIGUSR2,  &sigact, 0L); 
        sigaction(SIGFPE,  &sigact, 0L); 
        sigaction(SIGHUP,  &sigact, 0L); 
        */
    }

        return;
}
#endif
static psmcli_debug_level psmcli_get_debug_level(char const *file_name) {

    psmcli_debug_level ret = PSMCLI_DEBUG_PRINT_NONE;
    int c;  /// single char, so limited to 10 levels
    FILE *fd = fopen(file_name, "r");

    /* CID: 69724 Unsigned compared with neg
     * CID: 55411 Truncated stdio return value
     * CID: 71698 Operands don't affect result
     */
    if(fd == NULL || (c = fgetc(fd)) == EOF || c < '0')
        ret = PSMCLI_DEBUG_PRINT_NONE;
    else
        ret = c - '0';

    if(fd) fclose(fd);
    return ret;
}

// No longer needed if component name has pid info RTian 6/19/2013
/*
// #include <pthread.h>
// #include <dbus/dbus.h>
// check if a component id is in use
static int psmcli_bus_name_in_use(char const *component_id, char const *config_file)
{
    FILE                     *fp;
    CCSP_MESSAGE_BUS_INFO    *bus_info;
    char                     address[256];
    int                      count = 0;
    int                      ret_val = 0;
    errno_t                      rc = -1;

    char const func_name[] = "psmcli_bus_name_in_use";

    if(!config_file) {
        fprintf(stderr, "<%s>[%s]: empty config file name\n", prog_name, func_name);
        return (-1);
    }
    
    fp = fopen(config_file, "r");
    if (!fp)
    {
        fprintf(stderr, "<%s>[%s]: cannot open %s\n", prog_name, func_name, config_file);
        return (-1);
    }

    bus_info =(CCSP_MESSAGE_BUS_INFO*) AnscAllocateMemory(sizeof(CCSP_MESSAGE_BUS_INFO));
    if(!bus_info)
    {
        fprintf(stderr, "<%s>[%s]: cannot allocate memory\n", prog_name, func_name);
        fclose(fp);
        return (-1);
    }

    rc = memset_s(bus_info, sizeof(CCSP_MESSAGE_BUS_INFO), 0, sizeof(CCSP_MESSAGE_BUS_INFO));
    ERR_CHK(rc);

    if(component_id) {
        rc = strcpy_s(bus_info->component_id, sizeof(bus_info->component_id), component_id);
        if( rc != EOK )
        {
           ERR_CHK(rc);
           fclose(fp);
           return (-1);
        }

        bus_info->component_id[sizeof(bus_info->component_id)-1] = '\0';
    }
    else {
        fprintf(stderr, "<%s>[%s]: no compenent_id to check\n", prog_name, func_name);
        ret_val = (-1);
        goto CLEANUP;
    }

    // assume the first address is our primary connection
    while (fgets(address, sizeof(address), fp) && count < CCSP_MESSAGE_BUS_MAX_CONNECTION )
    {
        DBusError error;
        DBusConnection *conn;
        int ret;
        char *str = address;

        //	CCSP_Message_Bus_Strip(address);
        // to overwrite line feed (0xa) or cr return (0xd) with '\0'
        while(*str) {
	  if(*str == 0xa || *str == 0xd) { 
            *str = '\0';
            break;
	  }
	  str++;
	}
        if(*address == 0) break;  // empty line

#ifdef _DEBUG_LOCAL
        fprintf(stderr, "<%s>[%s]: socket = '%s'\n", prog_name, func_name, address); 
#endif
        
        dbus_error_init (&error);
        conn = dbus_connection_open_private (address, &error);
        if(dbus_error_is_set (&error)) {
            fprintf(stderr, "<%s>[%s]: error in opening connection %s: %s\n", 
                    prog_name, func_name, address, error.message);
            dbus_error_free (&error);
            if(conn != NULL) {
                dbus_connection_close(conn);
                dbus_connection_unref(conn);
            }
            ret_val = (-1);
            break;
        }

        dbus_bus_register(conn, &error);
        if(dbus_error_is_set (&error)) {
            fprintf(stderr, "<%s>[%s] Error in registering connection to bus at %s: %s\n",
                    prog_name, func_name, address, error.message);
            dbus_error_free (&error);
            dbus_connection_close(conn);
            dbus_connection_unref(conn);
            ret_val = (-1);
            break;
        }

        ret = dbus_bus_request_name (conn, 
                                     component_id,
                                     DBUS_NAME_FLAG_ALLOW_REPLACEMENT|DBUS_NAME_FLAG_REPLACE_EXISTING|DBUS_NAME_FLAG_DO_NOT_QUEUE,
                                     &error);
        if (dbus_error_is_set (&error)) {
            fprintf(stderr, "<%s>[%s] Error in requesting name %s: %s\n", 
                    prog_name, func_name, component_id, error.message);
            dbus_error_free (&error);
            dbus_connection_close(conn);
            dbus_connection_unref(conn);
            ret_val = (-1);
            break;
        }
        else if (ret != DBUS_REQUEST_NAME_REPLY_PRIMARY_OWNER && ret !=DBUS_REQUEST_NAME_REPLY_ALREADY_OWNER) {
#ifdef _DEBUG_LOCAL
            fprintf(stderr, "<%s>[%s]: Name request returned %d: someones own the name %s \n", 
                    prog_name, func_name, ret, component_id);
#endif
            dbus_connection_close(conn);
            dbus_connection_unref(conn);
            ret_val = 1;
            break;
        }

        // name is ok, keep the connections
        //  dbus_connection_close(conn);
        //  dbus_connection_unref(conn);
        
        count++;
    }
    
 CLEANUP:
    fclose(fp);
    AnscFreeMemory((char*)bus_info);

#ifdef _DEBUG_LOCAL
    fprintf(stderr, "<%s>[%s]: function finished returing %d\n", prog_name, func_name, ret_val);
#endif    
    return ret_val;
}
*/

// Prints usage of psmcli tool
void help_usage()
{
    int i = 0;
    while (help_usage_desc[i] != NULL) {
        printf("\t%s\n", help_usage_desc[i]);
        i++;
    }
}

// Function: process_get
// Example: ./psmcli get COM.CISCO.CCSP.ETH0.IPv4_ADDR
unsigned int process_get(int const argCnt, char const * const argVars[], char const * const busHandle) {

    int cmd_index = 2;
    int cmd_cnt = argCnt - 2;
    unsigned int ret = 0;
    unsigned int iter = 0;
    unsigned int func_ret = CCSP_SUCCESS;
    char *psmValue = NULL;
    unsigned int psmType = ccsp_string;
    char const func_name[] = "process_get";

    // Loop over the commands and extract the values
    while(cmd_cnt--) {
        ret = 0;
	iter = 0;
	do {
        ret = PSM_Get_Record_Value2((void*)busHandle, 
                                    subsys_prefix, //PSMCLI_SUBSYSTEM_PREFIX,
                                    argVars[cmd_index],
                                    &psmType, &psmValue);
	iter++;
	} while (( psmValue == NULL ) && ( iter < 3 ) );
        if (ret == CCSP_SUCCESS) {
            if(psmValue != NULL) {
                printf("%s\n", psmValue);
                ((CCSP_MESSAGE_BUS_INFO*)busHandle)->freefunc(psmValue);
                psmValue = NULL;
            } else {
                CcspTraceWarning(("<%s>[%s]: '%s' -> psmValue is NULL! return set to %d = CCSP_CR_ERR_INVALID_PARAM\n", 
                                prog_name, func_name, argVars[cmd_index], ret));
                func_ret = ret; 
            }
        } else if (ret == CCSP_CR_ERR_INVALID_PARAM) {
            CcspTraceWarning(("<%s>[%s]: invalid paramter '%s', return set to %d = CCSP_CR_ERR_INVALID_PARAM\n", 
                            prog_name, func_name, argVars[cmd_index], ret));
            func_ret = ret; 
            if(psmValue != NULL) { 
                ((CCSP_MESSAGE_BUS_INFO*)busHandle)->freefunc(psmValue);
                psmValue = NULL; 
            }
            //            break;  // to accommondate more cmd
        } else {
            // Can choose to stop further processing - use break
            CcspTraceWarning(("<%s>[%s]: processing '%s' unsuccessful, return set to %d\n", 
                            prog_name, func_name, argVars[cmd_index], ret));
            func_ret = ret;
            if(psmValue != NULL) { 
                ((CCSP_MESSAGE_BUS_INFO*)busHandle)->freefunc(psmValue);
                psmValue = NULL; 
            }
            //            break; // to accommondate more cmd
        }
        
        cmd_index++;
    }

    /* CID: 52817 Logically dead code - remove the check as psmValue = NULL*/

    //    CcspTraceDebug(("<%s>[%s]: function finished returing %d\n", prog_name, func_name, func_ret));

    return func_ret;
}

// Function: process_getdetail
// Example: ./psmcli getdetail COM.CISCO.CCSP.ETH0.IPv4_ADDR
unsigned int process_getdetail(int const argCnt, char const * const argVars[], char const * const busHandle) {

    int cmd_index = 2;
    int cmd_cnt = argCnt - 2;
    unsigned int ret = 0;
    unsigned int iter = 0;
    unsigned int func_ret = CCSP_SUCCESS;
    char *psmValue = NULL;
    unsigned int psmType = ccsp_string;
    char *typeStr = NULL;
    char const func_name[] = "process_getdetail";

    // Loop over the commands and extract the values
    while(cmd_cnt--) {
        ret = 0;
	iter = 0;
	do {
        ret = PSM_Get_Record_Value2((void*)busHandle, 
                                    subsys_prefix, // PSMCLI_SUBSYSTEM_PREFIX,
                                    argVars[cmd_index],
                                    &psmType, &psmValue);
	iter++;
	} while (( psmValue == NULL ) && ( iter < 3 ) );
        
        if (ret == CCSP_SUCCESS) {
            if (psmValue != NULL) {
                ret = get_type_info(&psmType, &typeStr, TYPE_FORMAT_CCSP);
                if( ret != CCSP_SUCCESS )
                {
                   return ret;
                }

                if(typeStr != NULL) {
                    printf("%s\n%s\n", typeStr, psmValue);
                    AnscFreeMemory(typeStr);
                }
                ((CCSP_MESSAGE_BUS_INFO*)busHandle)->freefunc(psmValue); 
                typeStr = psmValue = NULL;
            } else {
                CcspTraceWarning(("<%s>[%s]: '%s'-> psmValue is NULL, return set to %d = CCSP_CR_ERR_INVALID_PARAM\n", 
                                prog_name, func_name, argVars[cmd_index], ret));
                func_ret = ret;
            }
        } else if (ret == CCSP_CR_ERR_INVALID_PARAM) {
            CcspTraceWarning(("<%s>[%s]: invalid paramter '%s', return set to %d = CCSP_CR_ERR_INVALID_PARAM\n", 
                            prog_name, func_name, argVars[cmd_index], ret));
            func_ret = ret;
            if(psmValue != NULL) { 
                ((CCSP_MESSAGE_BUS_INFO*)busHandle)->freefunc(psmValue); 
                psmValue = NULL; 
            }
            //            break;  // to accommondate cmd_cnt > 1
        } else {
            // Can choose to stop further processing - use break
            CcspTraceWarning(("<%s>[%s]: processing '%s' unsuccessful, returning %d\n", 
                            prog_name, func_name, argVars[cmd_index], ret));
            func_ret = ret;
            if(psmValue != NULL) { 
                ((CCSP_MESSAGE_BUS_INFO*)busHandle)->freefunc(psmValue); 
                psmValue = NULL; 
            }
            //            break;  // to accommondate cmd_cnt > 1
        }
        
        cmd_index++;
    }

     /* CID: 74897 Logically dead code - remove the check as psmValue = NULL*/


    //    CcspTraceDebug(("<%s>[%s]: function finished returing %d\n", prog_name, func_name, func_ret));

    return func_ret;
}

// Function: process_get_e
// Example: ./psmcli get -e ETH0_IPV4ADDR COM.CISCO.CCSP.ETH0.IPv4_ADDR
unsigned int process_get_e(int const argCnt, char const * const argVars[], char const * const busHandle) {

    //printf("Command Arguments = %s\t%s\n", argVars[1], argVars[2]);
    // Check if arguments are in pairs
    int cmd_index = 3; 
    int cmd_cnt = argCnt - 3;
    unsigned int ret = 0;
    unsigned int iter = 0;
    unsigned int func_ret = CCSP_SUCCESS;
    char *psmValue = NULL;
    unsigned int psmType = ccsp_string;
    char const func_name[] = "process_get_e";

    if ((cmd_cnt % 2) != 0) {
    	CcspTraceWarning(("<%s>[%s]: arg count = %d is not even, returning %d CCSP_ERR_INVALID_ARGUMENTS\n", 
                        prog_name, func_name, cmd_cnt, CCSP_ERR_INVALID_ARGUMENTS));
    	return CCSP_ERR_INVALID_ARGUMENTS;
    }
    
    cmd_cnt = cmd_cnt/2;

#ifdef PSMCLI_TESTING_LOCAL
    if (psmcli_debug_print >= PSMCLI_DEBUG_PRINT_SUBROUTINE) { 
        int i=0;
        CcspTraceDebug(("<%s>[%s]: cmd_cnt=%d, '", prog_name, func_name, cmd_cnt));
        for(i=0; i<(cmd_cnt-1); i++) { CcspTraceDebug(("%s %s ", argVars[cmd_index+i*2], argVars[cmd_index+i*2+1])); }
        CcspTraceDebug(("%s %s\'\n", argVars[cmd_index+(cmd_cnt-1)*2], argVars[cmd_index+(cmd_cnt-1)*2+1]));
    } 
#endif

    // Loop over the commands and extract the values
    while(cmd_cnt--) {

        //        CcspTraceDebug(("<%s>[%s]: PSM_Get2 query='%s'\n", prog_name, func_name, argVars[cmd_index+1]));

        ret = 0;
	iter = 0;
	do {
        ret = PSM_Get_Record_Value2((void*)busHandle, 
                                    subsys_prefix, //PSMCLI_SUBSYSTEM_PREFIX,
                                    argVars[cmd_index+1],
                                    &psmType, 
                                    &psmValue);
	iter++;
	} while (( psmValue == NULL ) && ( iter < 3 ) );


#ifdef PSMCLI_TESTING_LOCAL
        CcspTraceDebug(("<%s>[%s]: ret=%d", prog_name, func_name, ret));
        if (psmValue!=NULL) { CcspTraceDebug((", psmValue='%s'\n", psmValue)); }
        else { CcspTraceDebug((", psmValue = NULL\n")); }
#endif

        if (ret == CCSP_SUCCESS) {
            if(psmValue != NULL) {
                printf("%s=\"%s\"\n",argVars[cmd_index], psmValue);
                //                CcspTraceDebug(("<%s>[%s]: freeing psmValue at address 0x%x\n", prog_name, func_name, (unsigned int)psmValue));
                ((CCSP_MESSAGE_BUS_INFO*)busHandle)->freefunc(psmValue); 
                psmValue = NULL;
            } else {
                CcspTraceWarning(("<%s>[%s]: '%s'-> psmValue is NULL, return set to %d = CCSP_CR_ERR_INVALID_PARAM\n", 
                                prog_name, func_name, argVars[cmd_index+1], ret));
                func_ret = ret;
            }
        } else if (ret == CCSP_CR_ERR_INVALID_PARAM) {
            CcspTraceWarning(("<%s>[%s]: invalid paramter '%s', return set to %d = CCSP_CR_ERR_INVALID_PARAM\n", 
                            prog_name, func_name, argVars[cmd_index+1], ret));
            func_ret = ret;
            if(psmValue != NULL) { 
                //                CcspTraceDebug(("<%s>[%s]: freeing psmValue at address 0x%x\n", prog_name, func_name, (unsigned int)psmValue));
                ((CCSP_MESSAGE_BUS_INFO*)busHandle)->freefunc(psmValue);
                psmValue = NULL; 
            }
            //            break;  // to accommondate more cmd
        } else {
            // Can choose to stop further procesing - use break
           
            //            CcspTraceDebug(("<%s>[%s]: processing '%s' unsuccessful, returning %d\n", 
            //                            prog_name, func_name, argVars[cmd_index+1], ret));

            func_ret = ret;
            if(psmValue != NULL) { 
                //                CcspTraceDebug(("<%s>[%s]: freeing psmValue at address 0x%x\n", prog_name, func_name, (unsigned int)psmValue));
                ((CCSP_MESSAGE_BUS_INFO*)busHandle)->freefunc(psmValue); 
                psmValue = NULL; 
            }
            //            break;  // to accommondate more cmd
        }
        
        cmd_index += 2;
    }

     /* CID: 67777 Logically dead code - remove the check as psmValue = NULL*/
    
    //    CcspTraceDebug(("<%s>[%s]: function finished returing %d\n", prog_name, func_name, func_ret));

    return func_ret;
}

// Function: process_getdetail_e
// Example: ./psmcli getdetail -e ETH0_IPV4ADDR COM.CISCO.CCSP.ETH0.IPv4_ADDR
unsigned int process_getdetail_e(int const argCnt, char const * const argVars[], char const * const busHandle) {

    //printf("Command Arguments = %s\t%s\n", argVars[1], argVars[2]);
    // Check if arguments are in pairs
    int cmd_index = 3; 
    int cmd_cnt = argCnt - 3;
    unsigned int ret = 0;
    unsigned int iter = 0;
    unsigned int func_ret = CCSP_SUCCESS;
    char *psmValue = NULL;
    unsigned int psmType = ccsp_string;
    char *typeStr = NULL;
    char *typeStrEnv = NULL;
    char const func_name[] = "process_getdetail_e";
    errno_t rc = -1;
    unsigned int mem_alloc_size = 0;

    if ((cmd_cnt % 2) != 0) {
    	CcspTraceWarning(("<%s>[%s]: arg count = %d is not even, returning %d CCSP_ERR_INVALID_ARGUMENTS\n", 
                        prog_name, func_name, cmd_cnt, CCSP_ERR_INVALID_ARGUMENTS));
    	return CCSP_ERR_INVALID_ARGUMENTS;
    }

    cmd_cnt = cmd_cnt/2;
    // Loop over the commands and extract the values
    while(cmd_cnt--) {
        ret = 0;
	iter = 0;
	do {
        ret = PSM_Get_Record_Value2((void*)busHandle, 
                                    subsys_prefix, //PSMCLI_SUBSYSTEM_PREFIX,
                                    argVars[cmd_index+1],
                                    &psmType, &psmValue);
	iter++;
	} while (( psmValue == NULL ) && ( iter < 3 ) );
        
        if (ret == CCSP_SUCCESS) {
            if (psmValue != NULL) {
                ret = get_type_info(&psmType, &typeStr, TYPE_FORMAT_CCSP);
                if( ret != CCSP_SUCCESS )
                {
                   return ret;
                }

                mem_alloc_size = strlen(argVars[cmd_index]) + strlen("_TYPE") + 1;
                typeStrEnv = AnscAllocateMemory(mem_alloc_size);
                if ( typeStrEnv == NULL )
                {
                   return CCSP_ERR_MEMORY_ALLOC_FAIL;
                }
                
                rc = strcpy_s(typeStrEnv, mem_alloc_size, argVars[cmd_index]);
                if( rc != EOK )
                {
                   ERR_CHK(rc);
                   return CCSP_FAILURE;
                }

                rc = strcat_s(typeStrEnv, mem_alloc_size, "_TYPE");
                if( rc != EOK )
                {
                   ERR_CHK(rc);
                   return CCSP_FAILURE;
                }

                printf("%s=\"%s\"\n",typeStrEnv, typeStr);
                printf("%s=\"%s\"\n",argVars[cmd_index], psmValue);
                AnscFreeMemory(typeStrEnv);
                AnscFreeMemory(typeStr);
                ((CCSP_MESSAGE_BUS_INFO*)busHandle)->freefunc(psmValue); 
                typeStrEnv = typeStr = psmValue = NULL;
            } else {
                CcspTraceWarning(("<%s>[%s]: '%s'-> psmValue is NULL, return set to %d = CCSP_CR_ERR_INVALID_PARAM\n", 
                                prog_name, func_name, argVars[cmd_index+1], ret));
                func_ret = ret;
            }
        } else if (ret == CCSP_CR_ERR_INVALID_PARAM) {
            CcspTraceWarning(("<%s>[%s]: invalid paramter '%s', return set to %d = CCSP_CR_ERR_INVALID_PARAM\n", 
                            prog_name, func_name, argVars[cmd_index+1], ret));
            func_ret = ret;
            if(psmValue != NULL) { 
                ((CCSP_MESSAGE_BUS_INFO*)busHandle)->freefunc(psmValue); 
                psmValue = NULL; 
            }
            //            break;  // to accommondate more cmd
        } else {
            // Can choose to stop further procesing - use break
            //            CcspTraceDebug(("<%s>[%s]: processing '%s' unsuccessful, return set to %d\n", 
            //                            prog_name, func_name, argVars[cmd_index+1], ret));
            func_ret = ret;
            if(psmValue != NULL) { 
                ((CCSP_MESSAGE_BUS_INFO*)busHandle)->freefunc(psmValue); 
                psmValue = NULL; 
            }
            //            break;  // to accommondate more cmd
        }

        cmd_index += 2;
    }

    /* CID: 74897 Logically dead code - remove the check as psmValue = NULL*/    

    //    CcspTraceDebug(("<%s>[%s]: function finished returing %d\n", prog_name, func_name, func_ret));

    return func_ret;
}

// Function: process_set
// Example: ./psmcli set COM.CISCO.CCSP.ETH0.IPv4_ADDR 192.168.100.2
unsigned int process_set(int const argCnt, char const * const argVars[], char const * const busHandle) {

    //printf("Command Arguments = %s\n", argVars[1]);
    int cmd_index = 2;
    int cmd_cnt = argCnt - 2;
    unsigned int ret = 0;
    unsigned int func_ret = CCSP_SUCCESS;
    char *psmValue = NULL;
    unsigned int psmType = ccsp_string;
    char const func_name[] = "process_set";
    
    if ((cmd_cnt % 2) != 0) {
    	CcspTraceWarning(("<%s>[%s]: arg count = %d is not even, returning %d CCSP_ERR_INVALID_ARGUMENTS\n", 
                        prog_name, func_name, cmd_cnt, CCSP_ERR_INVALID_ARGUMENTS));
    	return CCSP_ERR_INVALID_ARGUMENTS;
    }

    /* Use the same type as the object's current type or use string in case of
       new object name */
    cmd_cnt = cmd_cnt/2;
    while(cmd_cnt--) {
        ret = 0;
        ret = PSM_Get_Record_Value2((void*)busHandle, 
                                    subsys_prefix, // PSMCLI_SUBSYSTEM_PREFIX,
                                    argVars[cmd_index],
                                    &psmType, &psmValue);
        
        if (ret == CCSP_SUCCESS && psmValue != NULL) {
            ((CCSP_MESSAGE_BUS_INFO*)busHandle)->freefunc(psmValue); 
            psmValue = NULL;

            // Set the new value, Reuse type info
            ret = 0;
            ret = PSM_Set_Record_Value2((void*)busHandle,
                                        subsys_prefix, // PSMCLI_SUBSYSTEM_PREFIX,
                                        argVars[cmd_index],
                                        psmType,
                                        argVars[cmd_index+1]);
            
            // Comment below
            if (ret == CCSP_SUCCESS) {
                printf("%d\n", CCSP_SUCCESS);
            } else {
                CcspTraceWarning(("<%s>[%s]: processing setting '%s' to '%s' unsuccessful, return set to %d\n", 
                                prog_name, func_name, argVars[cmd_index], argVars[cmd_index+1], ret));
                func_ret = ret;
                //                break;  // to accommondate more cmd
            }
            
        } else {
            // New object name, use default type = ccsp_string
            // Set the new value
            psmType = ccsp_string;
            ret = 0;
            ret = PSM_Set_Record_Value2((void*)busHandle,
                                        subsys_prefix, // PSMCLI_SUBSYSTEM_PREFIX,
                                        argVars[cmd_index],
                                        psmType,
                                        argVars[cmd_index+1]);
            
            if (ret == CCSP_SUCCESS) {
                printf("%d\n", CCSP_SUCCESS);
            } else {
                CcspTraceWarning(("<%s>[%s]: processing setting '%s' to '%s' unsuccessful, return set to %d\n", 
                                prog_name, func_name, argVars[cmd_index], argVars[cmd_index+1], ret));
                func_ret = ret;
                //                break;  // to accommondate more cmd
            }
        }
        
        cmd_index+=2;
    }

    if(psmValue != NULL) ((CCSP_MESSAGE_BUS_INFO*)busHandle)->freefunc(psmValue); 

    //    CcspTraceDebug(("<%s>[%s]: function finished returing %d\n", prog_name, func_name, func_ret));

    return func_ret;
}

// Function: process_setdetail
// Example: ./psmcli setdetail string COM.CISCO.CCSP.ETH0.IPv4_ADDR 192.168.100.2
unsigned int process_setdetail(int const argCnt, char const * const argVars[], char const * const busHandle) {

    //printf("Command Arguments = %s\n", argVars[1]);
    int cmd_index = 2;
    int cmd_cnt = argCnt - 2;
    unsigned int ret = 0;
    unsigned int func_ret = CCSP_SUCCESS;
    unsigned int psmType = ccsp_string;
    char const func_name[] = "process_setdetail";

    if ((cmd_cnt % 3) != 0) {
    	CcspTraceWarning(("<%s>[%s]: arg count = %d is not a multiple of 3, returning %d CCSP_ERR_INVALID_ARGUMENTS\n", 
                        prog_name, func_name, cmd_cnt, CCSP_ERR_INVALID_ARGUMENTS));
    	return CCSP_ERR_INVALID_ARGUMENTS;
    }
    
    // setdetail shall override the current object type and set the new specified type
    cmd_cnt = cmd_cnt/3;
    while(cmd_cnt--) {
        
    	ret = get_type_info(&psmType, (char**)&argVars[cmd_index], TYPE_FORMAT_STRING);
    	if( ret != CCSP_SUCCESS )
    	{
    	   return ret;
    	}

    	if(psmType != ccsp_none) {
            
            ret = 0;
            ret = PSM_Set_Record_Value2((void*)busHandle,
        	            		subsys_prefix, //	PSMCLI_SUBSYSTEM_PREFIX,
                                        argVars[cmd_index+1],
                                        psmType,
                                        argVars[cmd_index+2]);
            
            if (ret == CCSP_SUCCESS) {
                printf("%d\n", CCSP_SUCCESS);
            } else {
                CcspTraceWarning(("<%s>[%s]: processing setting '%s' to '%s' unsuccessful, return set to %d\n", 
                                prog_name, func_name, argVars[cmd_index+1], argVars[cmd_index+2], ret));
                func_ret = ret;
                //                break;  // to accommondate more cmd
            }
        } else {
            CcspTraceWarning(("<%s>[%s]: unsupported datatype '%s', return set to %d = CCSP_CR_ERR_UNSUPPORTED_DATATYPE\n", 
                            prog_name, func_name, argVars[cmd_index], CCSP_CR_ERR_UNSUPPORTED_DATATYPE));
            func_ret = CCSP_CR_ERR_UNSUPPORTED_DATATYPE;
            //                break;  // to accommondate more cmd
        }

        cmd_index+=3;
    }

    //    CcspTraceDebug(("<%s>[%s]: function finished returing %d\n", prog_name, func_name, func_ret));

    return func_ret;
}

// Function: process_del
// Example: ./psmcli del COM.CISCO.CCSP.ETH0.IPv4_ADDR
unsigned int process_del(int const argCnt, char const * const argVars[], char const * const busHandle) {

    //printf("Command Arguments = %s\n", argVars[1]);
    int cmd_index = 2;
    int cmd_cnt = argCnt - 2;
    unsigned int func_ret = CCSP_SUCCESS;
    unsigned int ret = 0;
    char const func_name[] = "process_del";

    // Loop over the commands and extract the values
    while(cmd_cnt--) {
        ret = 0;
        ret = PSM_Del_Record((void*)busHandle, 
                             subsys_prefix, // PSMCLI_SUBSYSTEM_PREFIX, 
                             argVars[cmd_index]);
        
        if ( ret == CCSP_SUCCESS ) {
            printf("%d", CCSP_SUCCESS);
        } else {
            CcspTraceWarning(("<%s>[%s]: error deleting '%s', return set to %d\n",
                            prog_name, func_name, argVars[cmd_index], ret));
            func_ret = ret;
            //                break;  // to accommondate more cmd
        }
        
        cmd_index++;
    }

    //    CcspTraceDebug(("<%s>[%s]: function finished returing %d\n", prog_name, func_name, func_ret));

    return func_ret;
}

// Function: process_getallinst
// Get all object instance names
unsigned int process_getallinst(int const argCnt, char const * const argVars[], char const * const busHandle) {
    
    int cmd_index = 2;
    int cmd_cnt = argCnt - 2;
    unsigned int func_ret = CCSP_SUCCESS;
    unsigned int instanceCnt = 0;
    unsigned int *instanceList = NULL;
    unsigned int i = 0;
    char const func_name[] = "process_getallinst";

    if(cmd_cnt != 1) {
    	CcspTraceWarning(("<%s>[%s]: arg count = %d is not 1, returning %d CCSP_ERR_INVALID_ARGUMENTS\n", 
                        prog_name, func_name, cmd_cnt, CCSP_ERR_INVALID_ARGUMENTS));
        return CCSP_ERR_INVALID_ARGUMENTS;
    } 
    
    func_ret = PsmGetNextLevelInstances((void*)busHandle, 
                                        subsys_prefix, // PSMCLI_SUBSYSTEM_PREFIX, 
                                        argVars[cmd_index], 
                                        &instanceCnt, 
                                        &instanceList);

    if(func_ret == CCSP_SUCCESS && instanceList != NULL) {
        for(i = 0; i < instanceCnt; i++) {
            printf("%d\n", instanceList[i]);
        }
        ((CCSP_MESSAGE_BUS_INFO*)busHandle)->freefunc(instanceList); 
        instanceList = NULL;
    } else {
        CcspTraceWarning(("<%s>[%s]: function unsuccessful, returning %d\n",
                        prog_name, func_name, func_ret));
    }


    if(instanceList != NULL) ((CCSP_MESSAGE_BUS_INFO*)busHandle)->freefunc(instanceList);

    //    CcspTraceDebug(("<%s>[%s]: function finished returing %d\n", prog_name, func_name, func_ret));

    return func_ret; 
}

// Function: process_getallinst
// Get number of instances of an object
unsigned int process_getinstcnt(int const argCnt, char const * const argVars[], char const * const busHandle) {

    int cmd_index = 2;
    int cmd_cnt = argCnt - 2;
    unsigned int func_ret = CCSP_SUCCESS;
    unsigned int ret = 0;
    unsigned int instanceCnt = 0;
    unsigned  int *instanceList = NULL;
    char const func_name[] = "process_getinstcnt";

    while(cmd_cnt--) {
        ret = 0;
        ret = PsmGetNextLevelInstances((void*)busHandle, 
                                       subsys_prefix, // PSMCLI_SUBSYSTEM_PREFIX, 
                                       argVars[cmd_index], 
                                       &instanceCnt, 
                                       &instanceList);
        
        if ( ret == CCSP_SUCCESS) {
            printf("%d\n", instanceCnt);
            if(instanceList != NULL) {
                ((CCSP_MESSAGE_BUS_INFO*)busHandle)->freefunc(instanceList); 
                instanceList = NULL;
            }
        } else {
            CcspTraceWarning(("<%s>[%s]: function unsuccessful, return set to %d\n",
                            prog_name, func_name, ret));
            func_ret = ret;
            //                break;  // to accommondate more cmd
        }
        
        cmd_index++;
    }

    if(instanceList != NULL) ((CCSP_MESSAGE_BUS_INFO*)busHandle)->freefunc(instanceList); 

    //    CcspTraceDebug(("<%s>[%s]: function finished returing %d\n", prog_name, func_name, func_ret));

    return func_ret;
}

static unsigned int ccspType_from_Name( char *name, unsigned int *ccspType )
{
	errno_t rc = -1;
	int ind = -1;
	unsigned int i;

	if(( name == NULL ) || ( ccspType == NULL ))
		return CCSP_FAILURE;

	for( i=0; i<NUM_CCSP_TYPES; i++ )
	{
	   rc = strcmp_s( name, strlen(name), ccsp_type_table[i].ccspName, &ind );
	   ERR_CHK(rc);

	   if(( ind == 0 ) && ( rc == EOK ))
	   {
	      *ccspType = ccsp_type_table[i].ccspType;
		  return CCSP_SUCCESS;
	   }
	}

	// Unrecognized type
	*ccspType = ccsp_none;

	return CCSP_SUCCESS;
}

static unsigned int typeString_from_ccspType( unsigned int ccspType, char *typeString )
{
	errno_t rc = -1;
	unsigned int i;

	if( typeString == NULL )
		return CCSP_FAILURE;

	for( i=0; i<NUM_CCSP_TYPES; i++ )
	{
	   if( ccsp_type_table[i].ccspType == ccspType )
	   {
	      rc = strcpy_s(typeString, TYPE_STRING_SIZE, ccsp_type_table[i].ccspName);
	      if( rc != EOK )
	      {
	         ERR_CHK( rc );
	         return CCSP_FAILURE;
	      }

	      return CCSP_SUCCESS;
	   }
	}

	rc = strcpy_s(typeString, TYPE_STRING_SIZE, "unknown");
	if( rc != EOK )
	{
	   ERR_CHK( rc );
	   return CCSP_FAILURE;
	}

	return CCSP_SUCCESS;
}


// Function: get_type_info
// Convert from ccsp type to string version or vice-versa
static unsigned int get_type_info(unsigned int *ccspType, char **typeString, int const typeFormat) {

	unsigned int ret = CCSP_SUCCESS;

    if(typeFormat == TYPE_FORMAT_CCSP)
    {
        *typeString = (char*)AnscAllocateMemory(TYPE_STRING_SIZE);
        if( *typeString == NULL )
        {
           return CCSP_ERR_MEMORY_ALLOC_FAIL;
        }

        ret = typeString_from_ccspType( *ccspType, *typeString );
        if( ret != CCSP_SUCCESS )
        {
           free( *typeString );
           return ret;
        }
    }
    else
    {
       ret = ccspType_from_Name( *typeString, ccspType );
       return ret;
    }

	return CCSP_SUCCESS;
}
