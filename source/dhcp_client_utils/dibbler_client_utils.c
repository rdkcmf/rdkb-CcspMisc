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

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include<string.h>
#include "dhcp_client_utils.h"
#include "dibbler_client_utils.h"
#include <sysevent/sysevent.h>

#ifdef DHCPV6_CLIENT_DIBBLER

#define LOCALHOST         "127.0.0.1"
#define MAPT_SYSEVENT_NAME "mapt_evt_handler"

extern token_t dhcp_sysevent_token;
extern int dhcp_sysevent_fd;


/*
 * start_process ()
 * @description: This function start dibbler client program or any other executable and return pid.
 * @params     : exe - program to run eg: "dibbler-client"
 * @return     : returns the pid of the program started.
 *
 */
static pid_t start_process(char * exe)
{
    int32_t pid = 0;

    if ((exe == NULL))
    {
        DBG_PRINT("%s %d: Invalid arguments..\n", __FUNCTION__, __LINE__);
        return pid;
    }

    char *const parmList[] = {exe, "start", NULL};

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

        int err = execv(exe, parmList);

        /* We should not reach this line.  If we do, exec has failed. */
        DBG_PRINT("%s %d: execv returned %d failed due to %s.\n", __FUNCTION__, __LINE__, err, strerror(errno));
        exit(-1);
    }

    return pid;
}

static int dibbler_get_req_options (dhcp_opt_list * req_opt_list)
{
    FILE * fin;
    FILE * fout;
    char * line = NULL;
    size_t len = 0;
    ssize_t read;
    char mapt_config[BUFLEN_16] = {0};
    bool optionTempFound = 0;

    if (req_opt_list == NULL)
    {
        DBG_PRINT("%s %d: No req option sent to dibbler.\n", __FUNCTION__, __LINE__);
        return FAILURE;
    }
    DBG_PRINT("%s %d: \n", __FUNCTION__, __LINE__);
    char args [BUFLEN_128] = {0};
    char buf [BUFLEN_16] = {0};

    fout = fopen(DIBBLER_TEMPLATE_CONFIG_FILE, "wb");
    fin = fopen(DIBBLER_TMP_CONFIG_FILE, "r");
    if (fin && fout)
    {
        while ((read = getline(&line, &len, fin)) != -1)
        {
            if (strstr(line, "option"))
            {
                if (!optionTempFound)
                {
                    optionTempFound = 1;
                    dhcp_opt_list * opt_list = req_opt_list;
                    while (opt_list)
                    {
                        memset (&args, 0, BUFLEN_128);
                        memset (&buf, 0, BUFLEN_16);
                        memset (&mapt_config, 0, BUFLEN_16);

                        if (opt_list->dhcp_opt == DHCPV6_OPT_23)
                        {
                            snprintf (args, BUFLEN_128, "\n %s \n", "        option dns-server");
                            fputs(args, fout);
                        }
                        else if (opt_list->dhcp_opt == DHCPV6_OPT_95)
                        {
                            sysevent_get(dhcp_sysevent_fd, dhcp_sysevent_token, "mapt_config_flag", mapt_config, sizeof(mapt_config));
                            if (strncmp(mapt_config, "true", 4) == 0)
                            {
                                snprintf (args, BUFLEN_128, "\n        option 00%d hex \n", opt_list->dhcp_opt);
                                fputs(args, fout);
                            }
                        }
                        else
                        {
                            snprintf (args, BUFLEN_128, "\n        option 00%d hex \n", opt_list->dhcp_opt);
                            fputs(args, fout);
                        }
                        opt_list = opt_list->next;
                    }
                }
            }
            else
            {
                fputs(line, fout);
            }
        }

	unlink(DIBBLER_CONFIG_FILE);
	symlink(DIBBLER_TEMPLATE_CONFIG_FILE, DIBBLER_CONFIG_FILE);
        //memset (&args, 0, BUFLEN_128);
        //snprintf (args, BUFLEN_128, "ln -fs %s %s ", DIBBLER_TEMPLATE_CONFIG_FILE, DIBBLER_CONFIG_FILE);
        //system(args);

        fclose(fin);
        fclose(fout);
        if (line)
        {
            free(line);
        }
    }

    DBG_PRINT("%s %d: sucessfully Updated %s file with Request Options \n", __FUNCTION__, __LINE__, DIBBLER_TEMPLATE_CONFIG_FILE);
    return SUCCESS;

}

/*
 * dibbler_get_send_options ()
 * @description: This function will construct a buffer with all the dibbler SEND options
 * @params     : req_opt_list - input list of DHCP SEND options
 * @return     : return a buffer that has <SEND-DHCP-OPT SEND-DHCP-OPT-VALUE VALUE-TYPE> (or <SEND-DHCP-OPT> <SEND-DHCP-OPT-VALUE> for option23)
 *
 */
static int dibbler_get_send_options (dhcp_opt_list * send_opt_list)
{
    if (send_opt_list == NULL)
    {
        DBG_PRINT("%s %d: No send option sent to dibbler.\n", __FUNCTION__, __LINE__);
        return FAILURE;
    }

    while ((send_opt_list != NULL) && (send_opt_list->dhcp_opt_val != NULL))
    {
        DBG_PRINT("%s %d: dhcpv6 send options %d \n", __FUNCTION__, __LINE__, send_opt_list->dhcp_opt);
        send_opt_list = send_opt_list->next;
    }

    return SUCCESS;
}

/*
 * dibbler_get_other_args ()
 * @description: This function will construct a buffer with all other dibbler options
 * @params     : params - input parameters to udhcpc like interface
 * @return     : no other options
 *
 */
static int dibbler_get_other_args (dhcp_params * params)
{
    DBG_PRINT("%s %d: Not Supporting Any Other Options, Interface %s \n", __FUNCTION__, __LINE__, params->ifname);
    return SUCCESS;
}

/*
 * start_dibbler ()
 * @description: This function will build udhcpc request/send options and start dibbler client program.
 * @params     : params - input parameter to pass interface specific arguments
 *               v4_req_opt_list - list of DHCP REQUEST options
 *               v4_send_opt_list - list of DHCP SEND options
 * @return     : returns the pid of the udhcpc client program else return error code on failure
 *
 */
pid_t start_dibbler (dhcp_params * params, dhcp_opt_list * req_opt_list, dhcp_opt_list * send_opt_list)
{
    pid_t pid = 0;

    if (params == NULL)
    {
        DBG_PRINT("%s %d: Invalid args..\n", __FUNCTION__, __LINE__);
        return FAILURE;
    }

    DBG_PRINT("%s %d: Constructing REQUEST option args to dibbler.\n", __FUNCTION__, __LINE__);
    if (dibbler_get_req_options(req_opt_list) != SUCCESS)
    {
        DBG_PRINT("%s %d: Unable to get DHCPv6 REQ OPT.\n", __FUNCTION__, __LINE__);
    }

    DBG_PRINT("%s %d: Constructing SEND option args to dibbler.\n", __FUNCTION__, __LINE__);
    if (dibbler_get_send_options(send_opt_list) != SUCCESS)
    {
        DBG_PRINT("%s %d: Unable to get DHCPv6 SEND OPT.\n", __FUNCTION__, __LINE__);
    }

    DBG_PRINT("%s %d: Constructing other option args to dibbler.\n", __FUNCTION__, __LINE__);
    if (dibbler_get_other_args(params) != SUCCESS)
    {
        DBG_PRINT("%s %d: Unable to get DHCPv6 SEND OPT.\n", __FUNCTION__, __LINE__);
    }

    DBG_PRINT("%s %d: Starting dibbler.\n", __FUNCTION__, __LINE__);
    pid = start_process(DIBBLER_CLIENT_PATH);
    DBG_PRINT("%s %d: pid of dibbler is %d.\n", __FUNCTION__, __LINE__, pid);

    return pid;

}
#endif  // DHCPV6_CLIENT_DIBBLER
