/*
 * If not stated otherwise in this file or this component's LICENSE file the
 * following copyright and licenses apply:
 *
 * Copyright 2020 Sky
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "dhcp_client_utils.h"
#include "udhcpc_client_utils.h"
#include <syscfg/syscfg.h>
#include <string.h>

/*
 * freeArgs ()
 * @description: This function is a utility call to free the input array after execution of execv() call.
 * @params     : argv - output array to free()
 * @return     : no return
 *
 */
static void freeArgs(char **argv)
{
   int  i=0;
   while (argv[i] != NULL)
   {
     if ((argv[i]) != NULL)
     {
         free((argv[i]));
         (argv[i]) = NULL;
      }
      i++;
   }

   if (argv != NULL)
   {
      free(argv);
      argv = NULL;
   }
}

/*
 * parseArgs ()
 * @description: This function is a utility call to construct an array to pass to execv() call.
 * @params     : cmd - input program to run eg: "/sbin/udhcpc"
 *               args - input arguments to pass to the program eg: "-i erouter0"
 *               argv - output array to pass to execv() call
 * @return     : returns the pid of the program started.
 *
 */

static int parseArgs(const char *cmd, const char *args, char ***argv)
{
    int numArgs=3, i, len, argIndex=0;
    bool inSpace= TRUE;
    const char *cmdStr;
    char **array;

    len = (args == NULL) ? 0 : strlen(args);

    for (i=0; i < len; i++)
    {
        if ((args[i] == ' ') && (!inSpace))
        {
            numArgs++;
            inSpace = TRUE;
        }
        else
        {
            inSpace = FALSE;
        }
    }

    array = (char **) malloc ((numArgs) * sizeof(char *));
    if (array == NULL)
    {
        return -1;
    }

    /* locate the command name, last part of string */
    cmdStr = strrchr(cmd, '/');
    if (cmdStr == NULL)
    {
        cmdStr = cmd;
    }
    else
    {
        cmdStr++;
    }

    array[argIndex] = calloc (1, strlen(cmdStr) + 1);

    if (array[argIndex] == NULL)
    {
        DBG_PRINT("memory allocation of %d failed", strlen(cmdStr) + 1);
        freeArgs(array);
        return FAILURE;
    }
    else
    {
        strcpy(array[argIndex], cmdStr);
        argIndex++;
    }

    inSpace = TRUE;
    for (i=0; i < len; i++)
    {
        if ((args[i] == ' ') && (!inSpace))
        {
            numArgs++;
            inSpace = TRUE;
        }
        else if ((args[i] != ' ') && (inSpace))
        {
            int startIndex, endIndex;

            startIndex = i;
            endIndex = i;
            while ((endIndex < len) && (args[endIndex] != ' '))
            {
                endIndex++;
            }

            array[argIndex] = calloc(1,endIndex - startIndex + 1);
            if (array[argIndex] == NULL)
            {
                DBG_PRINT("memory allocation of %d failed", endIndex - startIndex + 1);
                freeArgs(array);
                return FAILURE;
            }

            memcpy(array[argIndex], &args[startIndex], endIndex - startIndex );

            argIndex++;

            inSpace = FALSE;
        }
    }
    array[argIndex] = NULL;
    (*argv) = array;

    return SUCCESS;
}

/*
 * start_exe ()
 * @description: This function start udhcpc client program or any other executable and return pid.
 * @params     : exe - program to run eg: "udhcpc"
 *               args - arguments to pass to the program eg: "-i erouter0"
 * @return     : returns the pid of the program started.
 *
 */
pid_t start_exe(char * exe, char * args)
{
    int32_t pid = 0;
    char **argv = NULL;
    int ret = SUCCESS;

    if ((exe == NULL) && (args == NULL))
    {
        DBG_PRINT("%s %d: Invalid arguments..\n", __FUNCTION__, __LINE__);
        return pid;
    }

    DBG_PRINT("buff %s.\n", args);

    if ((ret = parseArgs(exe, args, &argv)) != SUCCESS)
    {
        DBG_PRINT("Failed to parse arguments %d\n",ret);
        return pid;
    }

    if (signal(SIGCHLD, SIG_IGN) == SIG_ERR)
    {
        DBG_PRINT("ERROR: Couldn't set SIGCHLD handler!\n");
        return pid;
    }

    pid = fork();
    if (pid == 0)
    {
        int32_t devNullFd=-1, fd;

        /*
         * This is the child.
         */

        devNullFd = open("/dev/null", O_RDWR);
        if (devNullFd == -1)
        {
            DBG_PRINT("open of /dev/null failed");
            exit(-1);
        }

        close(0);
        fd = devNullFd;
        dup2(fd, 0);

        close(1);
        fd = devNullFd;
        dup2(fd, 1);

        close(2);
        fd = devNullFd;
        dup2(fd, 2);

        if (devNullFd != -1)
        {
            close(devNullFd);
        }

        signal(SIGHUP, SIG_DFL);
        signal(SIGINT, SIG_DFL);
        signal(SIGQUIT, SIG_DFL);
        signal(SIGILL, SIG_DFL);
        signal(SIGTRAP, SIG_DFL);
        signal(SIGABRT, SIG_DFL);  /* same as SIGIOT */
        signal(SIGQUIT, SIG_DFL);
        signal(SIGILL, SIG_DFL);
        signal(SIGTRAP, SIG_DFL);
        signal(SIGABRT, SIG_DFL);  /* same as SIGIOT */
        signal(SIGFPE, SIG_DFL);
        signal(SIGBUS, SIG_DFL);
        signal(SIGSEGV, SIG_DFL);
        signal(SIGSYS, SIG_DFL);
        signal(SIGPIPE, SIG_DFL);
        signal(SIGALRM, SIG_DFL);
        signal(SIGTERM, SIG_DFL);
        signal(SIGUSR1, SIG_DFL);
        signal(SIGUSR2, SIG_DFL);
        signal(SIGCHLD, SIG_DFL);  /* same as SIGCLD */
        signal(SIGPWR, SIG_DFL);
        signal(SIGWINCH, SIG_DFL);
        signal(SIGURG, SIG_DFL);
        signal(SIGIO, SIG_DFL);    /* same as SIGPOLL */
        signal(SIGSTOP, SIG_DFL);
        signal(SIGTSTP, SIG_DFL);
        signal(SIGCONT, SIG_DFL);
        signal(SIGTTIN, SIG_DFL);
        signal(SIGTTOU, SIG_DFL);
        signal(SIGVTALRM, SIG_DFL);
        signal(SIGPROF, SIG_DFL);
        signal(SIGXCPU, SIG_DFL);
        signal(SIGXFSZ, SIG_DFL);

        int err = execv(exe, argv);

        /* We should not reach this line.  If we do, exec has failed. */
        DBG_PRINT("%s %d: execv returned %d failed due to %s.\n", __FUNCTION__, __LINE__, err, strerror(errno));
        exit(-1);
    }

    freeArgs(argv);

    return pid;
}

#if DHCPV4_CLIEN_TI_UDHCPC
static pid_t start_ti_udhcpc (dhcp_params * params)
{
    if (params == NULL)
    {
        DBG_PRINT("%s %d: Invalid args..\n", __FUNCTION__, __LINE__);
        return FAILURE;
    }
}
#endif  // DHCPV4_CLIENT_TI_UDHCPC

/*
 * free_opt_list_data ()
 * @description: This function is called to free all the dynamic list created to hold dhcp options.
 * @params     : opt_list - list to free
 * @return     : no return
 *
 */
void free_opt_list_data (dhcp_opt_list * opt_list)
{
    if (opt_list == NULL)
    {
        return;
    }

    dhcp_opt_list * tmp_node = NULL;

    while (opt_list)
    {
        tmp_node = opt_list;
        opt_list = opt_list->next;
        if (tmp_node->dhcp_opt_val)
        {
            // DHCPv4 send opt will have opt_val
            free(tmp_node->dhcp_opt_val);
        }
        free(tmp_node);
    }

}

static int add_dhcpv4_opt_to_list (dhcp_opt_list ** opt_list, int opt, char * opt_val)
{

    if ((opt_list == NULL) || (opt <= 0) ||(opt >= DHCPV4_OPT_END) )
    {
        return RETURN_ERR;
    }

    dhcp_opt_list * new_dhcp_opt = malloc (sizeof(dhcp_opt_list));
    if (new_dhcp_opt == NULL)
    {
        return RETURN_ERR;
    }
    memset (new_dhcp_opt, 0, sizeof(dhcp_opt_list));
    new_dhcp_opt->dhcp_opt = opt;
    new_dhcp_opt->dhcp_opt_val = opt_val;

    if (*opt_list != NULL)
    {
        new_dhcp_opt->next = *opt_list;
    }
    *opt_list = new_dhcp_opt;

    return RETURN_OK;

}


static int get_dhcpv4_opt_list (dhcp_opt_list ** req_opt_list, dhcp_opt_list ** send_opt_list)
{
    char wanoe_enable[BUFLEN_16] = {0};

    if ((req_opt_list == NULL) || (send_opt_list == NULL))
    {
        DBG_PRINT ("%s %d: Invalid args..\n", __FUNCTION__, __LINE__);
        return FAILURE;
    }

    //syscfg for eth_wan_enabled
    if (syscfg_get(NULL, "eth_wan_enabled", wanoe_enable, sizeof(wanoe_enable)) == 0)
    {
        if (strcmp(wanoe_enable, "true") == 0)
        {
            add_dhcpv4_opt_to_list(req_opt_list, DHCPV4_OPT_122, NULL);
            add_dhcpv4_opt_to_list(send_opt_list, DHCPV4_OPT_125, NULL);
        }
    }
    else
    {
        DBG_PRINT("Failed to get eth_wan_enabled \n");
    }

    if (platform_hal_GetDhcpv4_Options(req_opt_list, send_opt_list) == FAILURE)
    {
        DBG_PRINT("%s %d: failed to get option list from platform hal\n", __FUNCTION__, __LINE__);
        return FAILURE;
    }

    return SUCCESS;

}


/*
 * start_dhcpv4_client ()
 * @description: This API will build dhcp request/send options and start dhcp client program.
 * @params     : input parameter to pass interface specific arguments
 * @return     : returns the pid of the dhcp client program else return error code on failure
 *
 */
pid_t start_dhcpv4_client (dhcp_params * params)
{
    if (params == NULL)
    {
        DBG_PRINT("%s %d: Invalid args..\n", __FUNCTION__, __LINE__);
        return 0;
    }

    syscfg_init();
    pid_t pid = FAILURE;

    // init part
    dhcp_opt_list * req_opt_list = NULL;
    dhcp_opt_list * send_opt_list = NULL;

    DBG_PRINT("%s %d: Collecting DHCP GET/SEND Request\n", __FUNCTION__, __LINE__);
    if (get_dhcpv4_opt_list(&req_opt_list, &send_opt_list) == FAILURE)
    {
        DBG_PRINT("%s %d: failed to get option list from platform hal\n", __FUNCTION__, __LINE__);
        return pid;
    }

    // building args and starting dhcpv4 client
    DBG_PRINT("%s %d: Starting DHCP Clients\n", __FUNCTION__, __LINE__);
#ifdef DHCPV4_CLIENT_UDHCPC
    pid =  start_udhcpc (params, req_opt_list, send_opt_list);
#elif DHCPV4_CLIEN_TI_UDHCPC
    pid =  start_ti_udhcpc (params);
#endif

    //exit part
    DBG_PRINT("%s %d: freeing all allocated resources\n", __FUNCTION__, __LINE__);
    free_opt_list_data (req_opt_list);
    free_opt_list_data (send_opt_list);
    return pid;

}
