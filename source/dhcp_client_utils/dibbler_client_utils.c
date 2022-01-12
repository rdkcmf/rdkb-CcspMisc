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

#include "dibbler_client_utils.h"

#ifdef DHCPV6_CLIENT_DIBBLER

#define LOCALHOST         "127.0.0.1"
#define MAPT_SYSEVENT_NAME "mapt_evt_handler"

extern token_t dhcp_sysevent_token;
extern int dhcp_sysevent_fd;

static void convert_option16_to_hex(char **optionVal)
{
    if(*optionVal == NULL)
    {
        return;
    }
    
    char enterprise_number_string[5] = {'\0'};
    int enterprise_number;
    int enterprise_number_len = 4;
    char temp[10] ={0};

    // we receive option value in  [enterprise_number(4 bytes) + vendor-class-data field] format. Parse enterprise_number and covnert to int.
    strncpy(enterprise_number_string, *optionVal, enterprise_number_len);
    enterprise_number = atoi(enterprise_number_string);

    //lenth to store option in hex (0x...) format
    // 2 (length for "0x") + length to store option value in %02X (2 * (len of null + length to store data_field_length  + len of *optionVal + len of null)) + 1 (null)
    int optlen = 2 + 2 * (1 + 1 + strlen(*optionVal) + 1) + 1;
    char * option16 = malloc (optlen);

    memset (option16, 0 , optlen);

    //convert and store the values in hex format
    snprintf(option16, 11, "0x%08X", enterprise_number);

    //append null
    snprintf(temp, 3, "%02X", '\0');
    strncat(option16,temp,3);

    int data_field_length = (int)strlen(*optionVal+enterprise_number_len);

    //append length of data_field_length+null
    sprintf(temp, "%02X", (data_field_length+1));
    strncat(option16,temp,3);

    for(int i=0; i<=data_field_length; i++)
    {
        snprintf(temp, 3, "%02X", (*optionVal)[enterprise_number_len+i]);
        strncat(option16,temp,3);
    }
    free(*optionVal);
    *optionVal = option16;

    return;
}

/*
 * dibbler_get_options ()
 * @description: This function will construct conf file with all the dibbler GET options
 * @params     : req_opt_list - input list of DHCPv6 GET options, send_opt_list - input list of DHCPv6 send options
 * @return     : SUCCESS if config file written successfully, else returns FAILURE
 *
 */
static int dibbler_get_options (dhcp_opt_list * req_opt_list, dhcp_opt_list * send_opt_list)
{
    FILE * fin;
    FILE * fout;
    char * line = NULL;
    size_t len = 0;
    ssize_t read;
    char mapt_config[BUFLEN_16] = {0};
    bool optionTempFound = 0;
    bool option20Found = 0;

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
                            /* To add RFC.Feature.MAP-T.Enable flag checking */
                            snprintf (args, BUFLEN_128, "\n        option 00%d hex \n", opt_list->dhcp_opt);
                            fputs(args, fout);
                        }
                        else
                        {
                            snprintf (args, BUFLEN_128, "\n        option 00%d hex \n", opt_list->dhcp_opt);
                            fputs(args, fout);
                        }
                        opt_list = opt_list->next;
                    }

                    //send option list
                    opt_list = send_opt_list;
                    while (opt_list)
                    {
                        memset (&args, 0, BUFLEN_128);

                        if (opt_list->dhcp_opt == DHCPV6_OPT_16)
                        {
                            convert_option16_to_hex(&opt_list->dhcp_opt_val);
                            snprintf (args, BUFLEN_128, "\n\toption 00%d hex %s\n", opt_list->dhcp_opt, opt_list->dhcp_opt_val);
                            fputs(args, fout);
                        }
                        else if (opt_list->dhcp_opt == DHCPV6_OPT_15)
                        {
                            snprintf (args, BUFLEN_128, "\n\toption 00%d string \"%s\"\n", opt_list->dhcp_opt, opt_list->dhcp_opt_val);
                            fputs(args, fout);
                        }
                        else if (opt_list->dhcp_opt == DHCPV6_OPT_20)
                        {
                            option20Found = 1;
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
        if(option20Found)
        {
            snprintf (args, BUFLEN_128, "\n%s\n", "reconfigure-accept 1");
            fputs(args, fout);
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

    if (params == NULL)
    {
        DBG_PRINT("%s %d: Invalid args..\n", __FUNCTION__, __LINE__);
        return FAILURE;
    }

    DBG_PRINT("%s %d: Constructing REQUEST and SEND option args to dibbler.\n", __FUNCTION__, __LINE__);
    if (dibbler_get_options(req_opt_list,send_opt_list) != SUCCESS)
    {
        DBG_PRINT("%s %d: Unable to get DHCPv6 REQ OPT.\n", __FUNCTION__, __LINE__);
        return FAILURE;
    }

    DBG_PRINT("%s %d: Constructing other option args to dibbler.\n", __FUNCTION__, __LINE__);
    if (dibbler_get_other_args(params) != SUCCESS)
    {
        DBG_PRINT("%s %d: Unable to get DHCPv6 SEND OPT.\n", __FUNCTION__, __LINE__);
        return FAILURE;
    }

    DBG_PRINT("%s %d: Starting dibbler.\n", __FUNCTION__, __LINE__);
    pid_t ret = start_exe(DIBBLER_CLIENT_PATH, DIBBLER_CLIENT_RUN_CMD);
    if (ret <= 0)
    {
        DBG_PRINT("%s %d: unable to start dibbler-client %d.\n", __FUNCTION__, __LINE__, ret);
        return FAILURE;
    }

    //dibbler-client will demonize a child thread during start, so we need to collect the exited main thread
    if (collect_waiting_process(ret, DIBBLER_CLIENT_TERMINATE_TIMEOUT) != SUCCESS)
    {
        DBG_PRINT("%s %d: unable to collect pid for %d.\n", __FUNCTION__, __LINE__, ret);
    }

    DBG_PRINT("%s %d: Started dibbler-client. returning pid..\n", __FUNCTION__, __LINE__);
    return get_process_pid (DIBBLER_CLIENT);

}
#endif  // DHCPV6_CLIENT_DIBBLER	
