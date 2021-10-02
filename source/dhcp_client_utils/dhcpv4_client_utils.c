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
#include <syscfg/syscfg.h>
#include <string.h>

/*
 * start_exe ()
 * @description: This function start udhcpc client program and return pid.
 * @params     : exe - program to run eg: "udhcpc"
 * @return     : returns the pid of the program started.
 *
 */
int start_exe(char * cmd)
{

    if (cmd == NULL)
    {
        DBG_PRINT("%s %d: Invalid args..\n", __FUNCTION__, __LINE__);
        return FAILURE;
    }

    // run the client program
    if (system(cmd) == -1)
    {
        DBG_PRINT("%s %d: system() call failed due to %s\n", __FUNCTION__, __LINE__, strerror(errno));
        return FAILURE;
    }

    return SUCCESS;
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
