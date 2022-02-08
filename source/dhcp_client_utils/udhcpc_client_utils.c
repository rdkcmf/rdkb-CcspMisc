/*
 * If not stated otherwise in this file or this component's LICENSE file the
 * following copyright and licenses apply:
 *
 * Copyright 2020 RDK Management
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

#ifdef DHCPV4_CLIENT_UDHCPC

/*
 * ascii_to_hex ()
 * @description: returns allocated space with hex value of the input argument passed.
 * @params     : buff - buffer contents to convert to hex
 *               buff_len - length of input string
 * @return     : allocated memory containing the hex value of the buffer passed
 * NOTE        : free() required to clear allocated space this function allocates
 *
 */
static char * ascii_to_hex (char * buff, int buff_len)
{
    if ((buff == NULL) || (buff_len == 0))
    {
        DBG_PRINT("Invalid args..\n");
        return NULL;
    }

    int len = (buff_len * 2) + 1;
    char * hex_buff = malloc (len);
    if (hex_buff == NULL)
    {
        DBG_PRINT ("malloc() failure\n");
        return NULL;
    }
    memset (hex_buff, 0, len);

    int i = 0;
    for (i = 0; i < buff_len; i++)
    {
        sprintf(&hex_buff[i*2], "%02X", buff[i]);
    }

    return hex_buff;

}

/*
 * udhcpc_get_req_options ()
 * @description: This function will construct a buffer with all the udhcpc REQUEST options
 * @params     : buff - output buffer to pass all REQUEST options
 *               req_opt_list - input list of DHCP REQUEST options
 * @return     : return a buffer that has -O <REQ-DHCP-OPT>
 *
 */
static int udhcpc_get_req_options (char * buff, dhcp_opt_list * req_opt_list)
{

    if (buff == NULL)
    {
        DBG_PRINT("%s %d: Invalid args..\n", __FUNCTION__, __LINE__);
        return FAILURE;
    }

    if (req_opt_list == NULL)
    {
        DBG_PRINT("%s %d: No req option sent to udhcpc.\n", __FUNCTION__, __LINE__);
        return SUCCESS;
    }

    char args [BUFLEN_16] = {0};

    while (req_opt_list)
    {
        memset (&args, 0, BUFLEN_16);
        if (req_opt_list->dhcp_opt == DHCPV4_OPT_42)
        {
            strcpy (args, "-O ntpsrv ");
        }
        else
        {
            snprintf (args, BUFLEN_16, "-O %d ", req_opt_list->dhcp_opt);
        }
        req_opt_list = req_opt_list->next;
        strcat(buff, args);
    }

    DBG_PRINT("%s %d: get req args - %s\n", __FUNCTION__, __LINE__, buff);
    return SUCCESS;

}

/*
 * udhcpc_get_send_options ()
 * @description: This function will construct a buffer with all the udhcpc SEND options
 * @params     : buff - output buffer to pass all SEND options
 *               req_opt_list - input list of DHCP SEND options
 * @return     : return a buffer that has -x <SEND-DHCP-OPT:SEND-DHCP-OPT-VALUE> (or -V <SEND-DHCP-OPT-VALUE> for option60)
 *
 */
static int udhcpc_get_send_options (char * buff, dhcp_opt_list * send_opt_list)
{

    if (buff == NULL)
    {
        DBG_PRINT("%s %d: Invalid args..\n", __FUNCTION__, __LINE__);
        return FAILURE;
    }

    if (send_opt_list == NULL)
    {
        DBG_PRINT("%s %d: No send option sent to udhcpc.\n", __FUNCTION__, __LINE__);
        return SUCCESS;
    }

    char args [BUFLEN_128] = {0};
    while ((send_opt_list != NULL) && (send_opt_list->dhcp_opt_val != NULL))
    {
        memset (&args, 0, BUFLEN_128);
        if (send_opt_list->dhcp_opt == DHCPV4_OPT_60)
        {
            // Option 60 - Vendor Class Identifier has udhcp cmd line arg "-V <option-str>"
            snprintf (args, BUFLEN_128, "-V \"%s\" ", send_opt_list->dhcp_opt_val);
        }
        else
        {
            char * buffer = ascii_to_hex (send_opt_list->dhcp_opt_val, strlen(send_opt_list->dhcp_opt_val));
            if (buffer != NULL)
            {
                snprintf (args, BUFLEN_128, "-x 0x%02X:%s ", send_opt_list->dhcp_opt, buffer);
                free(buffer);
            }
        }
        send_opt_list = send_opt_list->next;
        strcat (buff,args);
    }

    return SUCCESS;
}

/*
 * udhcpc_get_other_args ()
 * @description: This function will construct a buffer with all other udhcpc options
 * @params     : buff - output buffer to pass all SEND options
 *               params - input parameters to udhcpc like interface
 * @return     : return a buffer that has -i, -p, -s, -b/f/n options
 *
 */
static int udhcpc_get_other_args (char * buff, dhcp_params * params)
{
     if ((buff == NULL) || (params == NULL))
    {
        DBG_PRINT("%s %d: Invalid args..\n", __FUNCTION__, __LINE__);
        return FAILURE;
    }

    // Add -i <ifname>
    if (params->ifname != NULL)
    {
        char ifname_opt[BUFLEN_16] = {0};
        snprintf (ifname_opt, sizeof(ifname_opt), "-i %s ", params->ifname);
        strcat (buff, ifname_opt);

        // Add -p <pidfile>
        char pidfile[BUFLEN_32] = {0};
        snprintf (pidfile, sizeof(pidfile), UDHCP_PIDFILE_PATTERN , params->ifname);
        strcat (buff, pidfile);
    }

    if (params->ifType == WAN_LOCAL_IFACE)
    {
        // Add -s <servicefile>
        char servicefile[BUFLEN_32] = {0};
#ifdef UDHCPC_SCRIPT_FILE
        snprintf (servicefile, sizeof(servicefile), "-s %s ", UDHCPC_SERVICE_SCRIPT_FILE);
#else
        snprintf (servicefile, sizeof(servicefile), "-s %s ", UDHCPC_SERVICE_EXE);
#endif
        strcat (buff, servicefile);
    }

    // Add udhcpc process behavior
#ifdef UDHCPC_RUN_IN_FOREGROUND
    // udhcpc will run in foreground
    strcat (buff, "-f ");
#elif UDHCPC_RUN_IN_BACKGROUND
    // udhcpc will run in background if lease not obtained
    strcat (buff, "-b ");
#elif UDHCPC_EXIT_AFTER_LEAVE_FAILURE
    // exit if lease is not obtained
    strcat (buff, "-n ");
#endif
    // send release before exit
    strcat (buff, "-R ");

    return SUCCESS;
}

/*
 * start_udhcpc ()
 * @description: This function will build udhcpc request/send options and start udhcpc client program.
 * @params     : params - input parameter to pass interface specific arguments
 *               req_opt_list - list of DHCP REQUEST options
 *               send_opt_list - list of DHCP SEND options
 * @return     : returns the pid of the udhcpc client program else return error code on failure
 *
 */
pid_t start_udhcpc (dhcp_params * params, dhcp_opt_list * req_opt_list, dhcp_opt_list * send_opt_list)
{
    if (params == NULL)
    {
        DBG_PRINT("%s %d: Invalid args..\n", __FUNCTION__, __LINE__);
        return FAILURE;
    }

    char buff [BUFLEN_512] = {0};

    DBG_PRINT("%s %d: Constructing REQUEST option args to udhcpc.\n", __FUNCTION__, __LINE__);
    if ((req_opt_list != NULL) && (udhcpc_get_req_options(buff, req_opt_list)) != SUCCESS)
    {
        DBG_PRINT("%s %d: Unable to get DHCPv4 REQ OPT.\n", __FUNCTION__, __LINE__);
        return FAILURE;
    }

    DBG_PRINT("%s %d: Constructing SEND option args to udhcpc.\n", __FUNCTION__, __LINE__);
    if ((send_opt_list != NULL) && (udhcpc_get_send_options(buff, send_opt_list) != SUCCESS))
    {
        DBG_PRINT("%s %d: Unable to get DHCPv4 SEND OPT.\n", __FUNCTION__, __LINE__);
        return FAILURE;
    }

    DBG_PRINT("%s %d: Constructing other option args to udhcpc.\n", __FUNCTION__, __LINE__);
    if (udhcpc_get_other_args(buff, params) != SUCCESS)
    {
        DBG_PRINT("%s %d: Unable to get DHCPv4 SEND OPT.\n", __FUNCTION__, __LINE__);
        return FAILURE;
    }

    DBG_PRINT("%s %d: Starting udhcpc.\n", __FUNCTION__, __LINE__);

    return start_exe(UDHCPC_CLIENT_PATH, buff);

}

/*
 * stop_udhcpc ()
 * @description: This function will stop udhcpc instance that is running for interface name passed in params.ifname
 * @params     : params - input parameter to pass interface specific arguments
 * @return     : returns the SUCCESS or FAILURE
 *
 */
int stop_udhcpc (dhcp_params * params)
{
    if ((params == NULL) || (params->ifname == NULL))
    {
        DBG_PRINT("%s %d: Invalid args..\n", __FUNCTION__, __LINE__);
        return FAILURE;
    }

    pid_t pid = 0;
    char cmdarg[BUFLEN_32] = {0};

    snprintf(cmdarg, sizeof(cmdarg), "%s", params->ifname);
    pid = get_process_pid(UDHCPC_CLIENT, cmdarg);

    if (pid <= 0)
    {
        DBG_PRINT("%s %d: unable to get pid of %s\n", __FUNCTION__, __LINE__, UDHCPC_CLIENT);
        return FAILURE;
    }

    if (signal_process(pid, SIGUSR2) != RETURN_OK)
    {
        DBG_PRINT("%s %d: unable to send signal to pid %d\n", __FUNCTION__, __LINE__, pid);
        return FAILURE;
    }

    return collect_waiting_process(pid, UDHCPC_TERMINATE_TIMEOUT);

}
#endif  // DHCPV4_CLIENT_UDHCPC

