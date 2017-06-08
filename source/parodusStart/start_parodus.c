 /*
 * If not stated otherwise in this file or this component's Licenses.txt file the
 * following copyright and licenses apply:
 *
 * Copyright 2017 RDK Management
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
   Copyright [2017] [Comcast, Corp.]
 
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
/**
 * @file start_parodus.c
 *
 * @description This is a C application to start parodus process.
 *
 */
#include <stdio.h>
#include <string.h>
#include <sys/sysinfo.h>
#include <time.h>
#include <stdarg.h>
#include <stdlib.h>
#include <ccsp/platform_hal.h>
#include <ccsp/cm_hal.h>
#include <autoconf.h>

#define PARODUS_UPSTREAM              "tcp://127.0.0.1:6666"
#define DEVICE_PROPS_FILE             "/etc/device.properties"
#define CLIENT_PORT_NUM     	      6667
#define MODULE 			      "PARODUS"
#define MAX_BUF_SIZE 		      1024
#define LogInfo(...)                  _START_LOG(__VA_ARGS__)

#ifdef CONFIG_CISCO
#define CONFIG_VENDOR_NAME  "Cisco"
#endif
/*----------------------------------------------------------------------------*/
/*                             Function Prototypes                            */
/*----------------------------------------------------------------------------*/
static void get_url(char *parodus_url, char *client_url, char *seshat_url);
static int addParodusCmdToFile(char *command);
static void _START_LOG(const char *msg, ...);

/*----------------------------------------------------------------------------*/
/*                             External Functions                             */
/*----------------------------------------------------------------------------*/

int main()
{
	char modelName[64]={'\0'};
	char serialNumber[64]={'\0'};
	char firmwareVersion[64]={'\0'};
	char lastRebootReason[64]={'\0'};
	char deviceMac[64]={'\0'};
	char manufacturer[64]={'\0'};
	CMMGMT_CM_DHCP_INFO dhcpinfo;
	char parodus_url[64] = {'\0'};
        char client_url[64] = {'\0'};
        char seshat_url[64] = {'\0'};
        char command[1024]={'\0'};
        unsigned long bootTime=0;
        struct sysinfo s_info;
        struct timeval currentTime;
       	int cmdUpdateStatus = -1;
        int upTime=0;	

        if ( platform_hal_PandMDBInit() == 0)
        {
                LogInfo("PandMDB initiated successfully\n");
        }
        else
        {
                LogInfo("Failed to initiate DB\n");
        }
        
        if ( cm_hal_InitDB() == 0)
        {
                LogInfo("cm_hal DB initiated successfully\n");
        }
        else
        {
                LogInfo("Failed to initiate cm_hal DB\n");
        }

	if ( platform_hal_GetModelName(modelName) == 0)
	{
		LogInfo("modelName returned from hal::%s\n", modelName);
	}
        else 
        {
        	LogInfo("Unable to get ModelName\n");
        	
    	}

	if ( platform_hal_GetSerialNumber(serialNumber) == 0)
	{
		LogInfo("serialNumber returned from hal:%s\n", serialNumber);
	}
        else 
        {
        	LogInfo("Unable to get SerialNumber\n");
    	}
    	
    	if ( platform_hal_GetFirmwareName(firmwareVersion, 64) == 0)
	{
		LogInfo("firmwareVersion returned from hal:%s\n", firmwareVersion);
	}
        else 
        {
        	LogInfo("Unable to get FirmwareName\n");
    	}
    	
    	if(strlen(CONFIG_VENDOR_NAME) > 0)
    	{
         	strcpy(manufacturer, CONFIG_VENDOR_NAME);
         	LogInfo("Manufacturer Name is %s\n", manufacturer);

        }
        else
        {
         	LogInfo("Unable to get Manufacturer Name\n");
        }
    	
    	
    	if (syscfg_init() != 0)
        {
        	LogInfo("syscfg init failure\n");
        	strcpy(lastRebootReason, "unknown");
        }
        else
        {
		syscfg_get( NULL, "X_RDKCENTRAL-COM_LastRebootReason", lastRebootReason, sizeof(lastRebootReason));
        	LogInfo("lastRebootReason is %s\n", lastRebootReason);
        }
        
         if (cm_hal_GetDHCPInfo(&dhcpinfo) == 0)
         {
         	LogInfo("MACAddress = %s\n", dhcpinfo.MACAddress);
         	strcpy(deviceMac, dhcpinfo.MACAddress);
         	LogInfo("deviceMac is %s\n", deviceMac);

         }
         else
         {
         	LogInfo("Unable to get MACAdress\n");
         }
         
         
        if(!bootTime)
        {
                if(sysinfo(&s_info))
                {
                        LogInfo("Failure in sysinfo fetch.\n");
                }
		else
		{
	                upTime = s_info.uptime;
                	gettimeofday(&currentTime, NULL);
                	bootTime = currentTime.tv_sec - upTime;
		}
        }

        if(bootTime != 0)
        {
                LogInfo("bootTime is %d\n", bootTime);
        }
        else
        {
                LogInfo("Unable to get bootTime\n");
	}

         LogInfo("Fetch parodus url from device.properties file\n");
	 get_url(parodus_url, client_url, seshat_url);
	 LogInfo("parodus_url returned is %s\n", parodus_url);
         LogInfo("seshat_url returned is %s\n", seshat_url);
	 
	 
	 LogInfo("Framing command for parodus\n");
		
	snprintf(command, sizeof(command),
	"/usr/bin/parodus --hw-model=%s --hw-serial-number=%s --hw-manufacturer=%s --hw-last-reboot-reason=%s --fw-name=%s --boot-time=%lu --hw-mac=%s --webpa-ping-time=180 --webpa-inteface-used=erouter0 --webpa-url=fabric.webpa.comcast.net --webpa-backoff-max=9 --parodus-local-url=%s --partner-id=comcast --seshat-url=%s", modelName, serialNumber, manufacturer, lastRebootReason, firmwareVersion, bootTime, deviceMac, ((NULL != parodus_url) ? parodus_url : PARODUS_UPSTREAM), seshat_url);

	LogInfo("parodus command formed is: %s\n", command);
	
	cmdUpdateStatus = addParodusCmdToFile(command);
	if(cmdUpdateStatus == 0)
	{
		LogInfo("Added parodus cmd to file\n");
	}
	else
	{
		LogInfo("Error in adding parodus cmd to file\n");
	}

	return 0;
}

/*----------------------------------------------------------------------------*/
/*                             Internal functions                             */
/*----------------------------------------------------------------------------*/

static void get_url(char *parodus_url, char *client_url, char *seshat_url)
{

	FILE *fp = fopen(DEVICE_PROPS_FILE, "r");
	char atom_ip[64] = {'\0'};
	
	if (NULL != fp)
	{
		char str[255] = {'\0'};
		while(fscanf(fp,"%s", str) != EOF)
		{
		    char *value = NULL;
		    
		    if(value = strstr(str, "PARODUS_URL="))
		    {
			value = value + strlen("PARODUS_URL=");
			strncpy(parodus_url, value, (strlen(str) - strlen("PARODUS_URL=")));
		    }
		    
		    if(value = strstr(str, "ATOM_INTERFACE_IP="))
		    {
			value = value + strlen("ATOM_INTERFACE_IP=");
			strncpy(atom_ip, value, (strlen(str) - strlen("ATOM_INTERFACE_IP=")));
		    }
		   
                    if(value = strstr(str, "SESHAT_URL="))
                    {
                        value = value + strlen("SESHAT_URL=");
                        strncpy(seshat_url, value, (strlen(str) - strlen("SESHAT_URL=")));
                    }

		}
	}
	else
	{
		LogInfo("Failed to open device.properties file:%s\n", DEVICE_PROPS_FILE);
	}
	fclose(fp);
	
	if (0 == parodus_url[0])
	{
		LogInfo("parodus_url is not present in device. properties:%s\n", parodus_url);
	
	}
	
	if (0 == atom_ip[0])
	{
		LogInfo("atom_ip is not present in device. properties:%s\n", atom_ip);
	
	}
	
        if (0 == seshat_url[0])
        {
                LogInfo("seshat_url is not present in device. properties:%s\n", seshat_url);

        }

	snprintf(client_url, 64, "tcp://%s:%d", atom_ip, CLIENT_PORT_NUM);
	LogInfo("client_url formed is %s\n", client_url);
	LogInfo("parodus_url formed is %s\n", parodus_url);	
        LogInfo("seshat_url formed is %s\n", seshat_url);

 }
 
static int addParodusCmdToFile(char *command)
{
	FILE *fp;

	LogInfo("Opening parodusCmd file for writing the content\n");
	fp = fopen("/tmp/parodusCmd.cmd", "w");
	if (fp == NULL)
	{
		LogInfo("Cannot open %s in write mode\n", "/tmp/parodusCmd.cmd");
		return -1;
	}
	if (ferror(fp))
	{
		LogInfo("Error while writing parodusCmd.cmd file.\n");
		fclose(fp);
		return -1;
	}

	fprintf(fp, "%s", command);
	fclose(fp);
	return 0;
}
 
static void _START_LOG(const char *msg, ...)
{
	va_list arg_ptr;
	int nbytes;
	char buf[MAX_BUF_SIZE];
	char curtime[128];
   	time_t rawtime;
   	struct tm * timeinfo;
   	time ( &rawtime );
   	timeinfo = localtime ( &rawtime );
    	strftime(curtime, 128, "%y%m%d-%T", timeinfo);
    	
    	va_start(arg_ptr, msg);
    	nbytes = vsnprintf(buf, MAX_BUF_SIZE, msg, arg_ptr);
    	va_end(arg_ptr);
    	
    	if( nbytes >=  MAX_BUF_SIZE )	
	{
	    buf[ MAX_BUF_SIZE - 1 ] = '\0';
	}
	else
	{
	    buf[nbytes] = '\0';
	} 	
	printf("%s : [%s] %s", curtime, MODULE, buf);
}
