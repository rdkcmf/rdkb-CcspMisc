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
#if !(_COSA_BCM_MIPS_ || _COSA_DRG_TPG_ || CONFIG_CISCO)
#include <autoconf.h>
#endif
#include "cJSON.h"

#ifdef _PLATFORM_RASPBERRYPI_
#include "ccsp_vendor.h"
#endif

#if defined(_COSA_BCM_MIPS_)
#include "dpoe_hal.h"
#endif

#define PARODUS_UPSTREAM              "tcp://127.0.0.1:6666"
#define DEVICE_PROPS_FILE             "/etc/device.properties"
#define MODULE 			      "PARODUS"
#define MAX_BUF_SIZE 		      1024
#define MAX_VALUE_SIZE 		      32
#define LOG_ERROR                     0
#define LOG_INFO                      1
#define LogInfo(...)                  _START_LOG(LOG_INFO, __VA_ARGS__)
#define LogError(...)                 _START_LOG(LOG_ERROR, __VA_ARGS__)
#define WEBPA_CFG_FILE		      "/nvram/webpa_cfg.json"
#define WEBPA_CFG_FIRMWARE_VER	      "oldFirmwareVersion"
#define SSL_CERT_BUNDLE               "/etc/ssl/certs/ca-certificates.crt"
#define WEBPA_TOKEN_APPLICATION       "/usr/ccsp/parodus/parodus_token.sh"
#define WEBPA_SERVER_URL              "fabric.webpa.comcast.net"
#define WEBPA_CFG_SERVER_URL          "ServerIP"

#ifdef CONFIG_CISCO
#define CONFIG_VENDOR_NAME  "Cisco"
#endif

#if (_COSA_BCM_MIPS_ || _COSA_DRG_TPG_)
#define CONFIG_VENDOR_NAME "ARRIS Group, Inc."
#endif

/*----------------------------------------------------------------------------*/
/*                             Function Prototypes                            */
/*----------------------------------------------------------------------------*/
static void get_url(char *parodus_url, char *seshat_url, char *build_type);
static int addParodusCmdToFile(char *command);
static void _START_LOG(int level, const char *msg, ...);
static void getValueFromCfgJson( char *key, char **value, cJSON **out);
static int  writeToJson(char *data);
static void getValuesFromPsmDb(char *names[], char **values,int count);
static void getValuesFromSysCfgDb(char *names[], char **values,int count);
static int setValuesToPsmDb(char *names[], char **values,int count);
static int syncXpcParamsOnUpgrade(char *lastRebootReason, char *firmwareVersion);
static void free_sync_db_items(int paramCount,char *psmValues[],char *sysCfgValues[]);
static char *pathPrefix  = "eRT.com.cisco.spvtg.ccsp.webpa.";
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
#if defined(_COSA_BCM_MIPS_)
	dpoe_mac_address_t tDpoe_Mac;
#else
	CMMGMT_CM_DHCP_INFO dhcpinfo;
#endif
	char parodus_url[64] = {'\0'};
        char seshat_url[64] = {'\0'};
	char build_type[16] = {'\0'};
	char *webpaUrl = NULL;
	cJSON *out = NULL;
        char command[1024]={'\0'};
        unsigned long bootTime=0;
        struct sysinfo s_info;
        struct timeval currentTime;
       	int cmdUpdateStatus = -1;
        int upTime=0, syncStatus= -1;
	char *paramList[] = {"X_COMCAST-COM_CMC","X_COMCAST-COM_CID","X_COMCAST-COM_SyncProtocolVersion"};
	int paramCount = 0, i = 0;
	char *psmValues[MAX_VALUE_SIZE] = {'\0'};

        if ( platform_hal_PandMDBInit() == 0)
        {
                LogInfo("PandMDB initiated successfully\n");
        }
        else
        {
                LogError("Failed to initiate DB\n");
        }
        
        if ( cm_hal_InitDB() == 0)
        {
                LogInfo("cm_hal DB initiated successfully\n");
        }
        else
        {
                LogError("Failed to initiate cm_hal DB\n");
        }

	if ( platform_hal_GetModelName(modelName) == 0)
	{
		LogInfo("modelName returned from hal::%s\n", modelName);
	}
        else 
        {
        	LogError("Unable to get ModelName\n");
        	
    	}

	if ( platform_hal_GetSerialNumber(serialNumber) == 0)
	{
		LogInfo("serialNumber returned from hal:%s\n", serialNumber);
	}
        else 
        {
        	LogError("Unable to get SerialNumber\n");
    	}
    	
    	if ( platform_hal_GetFirmwareName(firmwareVersion, 64) == 0)
	{
		LogInfo("firmwareVersion returned from hal:%s\n", firmwareVersion);
	}
        else 
        {
        	LogError("Unable to get FirmwareName\n");
    	}
    	
    	if(strlen(CONFIG_VENDOR_NAME) > 0)
    	{
         	strcpy(manufacturer, CONFIG_VENDOR_NAME);
         	LogInfo("Manufacturer Name is %s\n", manufacturer);

        }
        else
        {
         	LogError("Unable to get Manufacturer Name\n");
        }
    	
    	
    	if (syscfg_init() != 0)
        {
        	LogError("syscfg init failure\n");
        	strcpy(lastRebootReason, "unknown");
        }
        else
        {
		syscfg_get( NULL, "X_RDKCENTRAL-COM_LastRebootReason", lastRebootReason, sizeof(lastRebootReason));
        	LogInfo("lastRebootReason is %s\n", lastRebootReason);
        }
#if defined(_COSA_BCM_MIPS_)
	if( dpoe_getOnuId(&tDpoe_Mac) == 0)
	{
		sprintf(deviceMac, "%02x:%02x:%02x:%02x:%02x:%02x",tDpoe_Mac.macAddress[0], tDpoe_Mac.macAddress[1],
                tDpoe_Mac.macAddress[2], tDpoe_Mac.macAddress[3], tDpoe_Mac.macAddress[4],tDpoe_Mac.macAddress[5]);
		LogInfo("deviceMac is %s\n", deviceMac);
	}
#else
         if (cm_hal_GetDHCPInfo(&dhcpinfo) == 0)
         {
         	LogInfo("MACAddress = %s\n", dhcpinfo.MACAddress);
         	strcpy(deviceMac, dhcpinfo.MACAddress);
         	LogInfo("deviceMac is %s\n", deviceMac);

         }
#endif
         else
         {
         	LogError("Unable to get MACAdress\n");
         }
         
         
        if(!bootTime)
        {
                if(sysinfo(&s_info))
                {
                        LogError("Failure in sysinfo fetch.\n");
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
                LogError("Unable to get bootTime\n");
	}

         LogInfo("Fetch parodus url from device.properties file\n");
	 get_url(parodus_url, seshat_url, build_type);
	 LogInfo("parodus_url returned is %s\n", parodus_url);
         LogInfo("seshat_url returned is %s\n", seshat_url);
	 LogInfo("build_type returned is %s\n", build_type);

	if(strncmp(build_type, "dev", strlen(build_type)+1) == 0)
	{
		getValueFromCfgJson( WEBPA_CFG_SERVER_URL, &webpaUrl, &out);
		LogInfo("webpaUrl fetched from webpa_cfg.json is %s\n", webpaUrl);
		if(out != NULL)
		{
			cJSON_Delete(out);
		}	
		
		
	}
	if(webpaUrl == NULL)
	{	
		LogInfo("Setting webpaUrl to default server IP\n");
		webpaUrl = strdup(WEBPA_SERVER_URL);
	} 		
	 
	 LogInfo("Framing command for parodus\n");

#ifdef ENABLE_SESHAT
	snprintf(command, sizeof(command),
	"/usr/bin/parodus --hw-model=%s --hw-serial-number=%s --hw-manufacturer=%s --hw-last-reboot-reason=%s --fw-name=%s --boot-time=%lu --hw-mac=%s --webpa-ping-time=180 --webpa-interface-used=erouter0 --webpa-url=%s --webpa-backoff-max=9 --parodus-local-url=%s --partner-id=comcast --ssl-cert-path=%s --seshat-url=%s --webpa-token=%s", modelName, serialNumber, manufacturer, lastRebootReason, firmwareVersion, bootTime, deviceMac, webpaUrl, ((NULL != parodus_url) ? parodus_url : PARODUS_UPSTREAM), SSL_CERT_BUNDLE, seshat_url, WEBPA_TOKEN_APPLICATION);
#else
        snprintf(command, sizeof(command),
	"/usr/bin/parodus --hw-model=%s --hw-serial-number=%s --hw-manufacturer=%s --hw-last-reboot-reason=%s --fw-name=%s --boot-time=%lu --hw-mac=%s --webpa-ping-time=180 --webpa-interface-used=erouter0 --webpa-url=%s --webpa-backoff-max=9 --parodus-local-url=%s --partner-id=comcast --ssl-cert-path=%s --webpa-token=%s", modelName, serialNumber, manufacturer, lastRebootReason, firmwareVersion, bootTime, deviceMac, webpaUrl, ((NULL != parodus_url) ? parodus_url : PARODUS_UPSTREAM), SSL_CERT_BUNDLE, WEBPA_TOKEN_APPLICATION);
#endif

	LogInfo("parodus command formed is: %s\n", command);
	
	cmdUpdateStatus = addParodusCmdToFile(command);
	if(cmdUpdateStatus == 0)
	{
		LogInfo("Added parodus cmd to file\n");
	}
	else
	{
		LogError("Error in adding parodus cmd to file\n");
	}

	if(webpaUrl != NULL)
	{
		free(webpaUrl);
		webpaUrl = NULL;
	}
		
	syncStatus = syncXpcParamsOnUpgrade(lastRebootReason, firmwareVersion);
	if(syncStatus == 0)
	{
		LogInfo("DB synced successfully on firmware upgrade\n");
	}
	else if(syncStatus == -2)
	{
		LogInfo("PARODUS: Failed to sync DB during Firmware Upgrade\n");
	}
	else
	{
		LogInfo("DB sync is not required or failed to sync!!\n");
	}
	paramCount = sizeof(paramList)/sizeof(paramList[0]);
	getValuesFromPsmDb(paramList, psmValues, paramCount);
	LogInfo("DB details are %s = %s %s = %s %s = %s\n",paramList[0],psmValues[0],paramList[1],psmValues[1],paramList[2],psmValues[2]);
	for(i=0;i<paramCount;i++)
	{
		if(psmValues[i])
		{
			free(psmValues[i]);
		}
	}
	return 0;
}

/*----------------------------------------------------------------------------*/
/*                             Internal functions                             */
/*----------------------------------------------------------------------------*/

static void get_url(char *parodus_url, char *seshat_url, char *build_type)
{

	FILE *fp = fopen(DEVICE_PROPS_FILE, "r");
	
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
		    
                    if(value = strstr(str, "SESHAT_URL="))
                    {
                        value = value + strlen("SESHAT_URL=");
                        strncpy(seshat_url, value, (strlen(str) - strlen("SESHAT_URL=")));
                    }

		     if(value = strstr(str, "BUILD_TYPE="))
                    {
                        value = value + strlen("BUILD_TYPE=");
                        strncpy(build_type, value, (strlen(str) - strlen("BUILD_TYPE="))+1);
                    }
		}
	}
	else
	{
		LogError("Failed to open device.properties file:%s\n", DEVICE_PROPS_FILE);
	}
	fclose(fp);
	
	if (0 == parodus_url[0])
	{
		LogError("parodus_url is not present in device.properties:%s\n", parodus_url);
	
	}
	
        if (0 == seshat_url[0])
        {
                LogError("seshat_url is not present in device.properties:%s\n", seshat_url);

        }

	 if (0 == build_type[0])
        {
                LogError("build_type is not present in device.properties:%s\n", build_type);

        }

	LogInfo("parodus_url formed is %s\n", parodus_url);	
        LogInfo("seshat_url formed is %s\n", seshat_url);
	LogInfo("build_type is %s\n", build_type);

 }
 
static int addParodusCmdToFile(char *command)
{
	FILE *fp;

	LogInfo("Opening parodusCmd file for writing the content\n");
	fp = fopen("/tmp/parodusCmd.cmd", "w");
	if (fp == NULL)
	{
		LogError("Cannot open %s in write mode\n", "/tmp/parodusCmd.cmd");
		return -1;
	}
	if (ferror(fp))
	{
		LogError("Error while writing parodusCmd.cmd file.\n");
		fclose(fp);
		return -1;
	}

	fprintf(fp, "%s", command);
	fclose(fp);
	return 0;
}
 
static void _START_LOG(int level, const char *msg, ...)
{
	static const char *_level[] = { "Error", "Info" };
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
	printf("%s : [mod=%s, lvl=%s] %s", curtime, MODULE, _level[level], buf);
}

void getValueFromCfgJson( char *key, char **value, cJSON **out)
{
	char *data = NULL;
	cJSON *cfgValObj = NULL;
	cJSON *json = NULL;
	FILE *fileRead;
	int len;
	fileRead = fopen( WEBPA_CFG_FILE, "r+" );    
	if( fileRead == NULL ) 
	{
	    LogError( "Error opening file in read mode\n" );
	    return;
	}
	
	fseek( fileRead, 0, SEEK_END );
	len = ftell( fileRead );
	fseek( fileRead, 0, SEEK_SET );
	data = ( char* )malloc( len + 1 );
        if (data != NULL) {
	    fread( data, 1, len, fileRead );
        } else {
	    LogError("malloc() failed\n");
	}

	fclose( fileRead );

	if( data != NULL ) 
	{
	    json = cJSON_Parse( data );
	    if( !json ) 
	    {
	        LogError( "json parse error: [%s]\n", cJSON_GetErrorPtr() );
	    } 
	    else 
	    {
	    	cfgValObj = cJSON_GetObjectItem( json, key );
	        if( cfgValObj != NULL) 
	        {
	         	char *valFromJson = cJSON_GetObjectItem( json, key)->valuestring;

			if (valFromJson != NULL && strlen(valFromJson) > 0) 
			{
				*value = strdup(valFromJson);
			}	
			else
			{
				*value = NULL;
			}			
	
	       	 }
        	else
        	{
        		LogError("%s not available in webpa_cfg.json file\n", key);	
        	}
		
		*out = json;
	    }
	 free(data);
	 data = NULL;  
	    
	}

}


static int writeToJson(char *data)
{
    FILE *fp;
    fp = fopen(WEBPA_CFG_FILE, "w");
    if (fp == NULL) 
    {
        LogError("Failed to open file %s\n", WEBPA_CFG_FILE);
        return -1;
    }
    
    fwrite(data, strlen(data), 1, fp);
    fclose(fp);
    return 0;
}

static void getValuesFromPsmDb(char *names[], char **values,int count)
{
    FILE* out = NULL;
    char command[MAX_BUF_SIZE]={'\0'};
    char buf[MAX_BUF_SIZE] = {'\0'};
    char tempBuf[MAX_BUF_SIZE] ={'\0'};
    int offset = 0, i=0, index=0;
    char temp[MAX_VALUE_SIZE] = {'\0'};

    for(i=0; i<count; i++)
    {
        offset += snprintf(tempBuf + offset, sizeof(tempBuf) - offset, " %dX %s%s", i, pathPrefix, names[i]);
    }

    snprintf(command, sizeof(command),"psmcli get -e%s", tempBuf);
    LogInfo("command : %s\n",command);

    out = popen(command, "r");
    if(out)
    {
        for(i=0; i<count; i++)
        {
            fgets(buf, sizeof(buf), out);
	    if(strlen(buf) > 0)
	    {
                *strrchr(buf, '"') = '\0';
                values[i] = (char *) malloc(sizeof(char)* MAX_VALUE_SIZE);
                sscanf(buf, "%dX=\"%s\n", &index, temp);
		if(index == i)
		{
		    strcpy(values[i], temp);
		}
	        else
	        {
		    free(values[i]);
		    values[i] = NULL;
	        }
	    }
	    if(feof(out))
	    {
		LogInfo("End of file reached\n");
		break;
	    }
        }
        pclose(out);
    }
    else
    {
        LogError("Failed to execute command\n");
    }
}

static int setValuesToPsmDb(char *names[], char **values,int count)
{
    FILE* out = NULL;
    char command[MAX_BUF_SIZE]={'\0'};
    char buf[MAX_BUF_SIZE] = {0};
    int i = 0, ret=0;
    char tempBuf[MAX_BUF_SIZE] ={0};
    int offset = 0;

    for(i=0; i<count; i++)
    {
        offset += snprintf(tempBuf + offset, sizeof(tempBuf) - offset, " %s%s %s", pathPrefix,names[i], values[i]);
    }

    snprintf(command, sizeof(command),"psmcli set%s", tempBuf);
    LogInfo("command : %s\n",command);
    out = popen(command, "r");
    if(out)
    {
        for(i=0; i<count; i++)
        {
            fgets(buf, sizeof(buf), out);
            sscanf(buf, "%d\n", &ret);
            if(ret != 100)
            {
                LogError("Failed to setValuesToPsmDb\n");
                pclose(out);
                return -1;
            }
	    if(feof(out))
	    {
		LogInfo("End of file reached\n");
		break;
	    }
        }
        pclose(out);
    }
    else
    {
        LogError("Failed to execute command\n");
        return -1;
    }
    return 0;
}

static void getValuesFromSysCfgDb(char *names[], char **values,int count)
{
    int i = 0;
    for(i=0; i<count; i++)
    {
    	char temp[MAX_VALUE_SIZE] ={'\0'};
        if(syscfg_get( NULL, names[i], temp, MAX_VALUE_SIZE) == 0)
        {
	    	values[i] = (char *) malloc(sizeof(char)* MAX_VALUE_SIZE);
	    	strcpy(values[i], temp);
        }
    }
}

static int syncXpcParamsOnUpgrade(char *lastRebootReason, char *firmwareVersion)
{
	int paramCount = 0, status = 0, i = 0;
	cJSON *out = NULL;
	char *cfgJson_firmware = NULL;
    char *paramList[] = {"X_COMCAST-COM_CMC","X_COMCAST-COM_CID","X_COMCAST-COM_SyncProtocolVersion"};
	char *psmValues[MAX_VALUE_SIZE] = {'\0'};
	char *sysCfgValues[MAX_VALUE_SIZE] = {'\0'};
	int configUpdateStatus = -1;
	char *cJsonOut =NULL;

	paramCount = sizeof(paramList)/sizeof(paramList[0]);
	getValueFromCfgJson( WEBPA_CFG_FIRMWARE_VER, &cfgJson_firmware, &out);
		
	LogInfo(" Returned json content is: %s\n", cJSON_Print(out));
	if(out != NULL)
	{
		LogInfo("cfgJson_firmware fetched from webpa_cfg.json is %s\n", cfgJson_firmware);
		cJSON_ReplaceItemInObject(out, WEBPA_CFG_FIRMWARE_VER, cJSON_CreateString(firmwareVersion));
		
		cJsonOut = cJSON_Print(out);
		LogInfo("Updated json content is %s\n", cJsonOut);
		configUpdateStatus = writeToJson(cJsonOut);

		if(configUpdateStatus == 0)
		{
			LogInfo("Updated current Firmware version to config file\n");
		}
		else
		{
			LogError("Error in updating current Firmware version to config file\n");
		}
		if(cJsonOut != NULL)
		{
			free(cJsonOut);
			cJsonOut = NULL;
		}

		cJSON_Delete(out);
	}

	else
	{
		LogError("Error in fetching data from webpa_cfg.json file\n");
	}

	
	getValuesFromPsmDb(paramList, psmValues, paramCount);
	for(i = 0; i<paramCount; i++)
	{
	        if(psmValues[i] == NULL)
	        {
	        	LogInfo("PsmDb-> value is NULL for %s\n",paramList[i]);
	        	free_sync_db_items(paramCount, psmValues, sysCfgValues);
	        	return -1;
	        }
	        else
	        {
	        	LogInfo("PsmDb-> %s value is %s\n",paramList[i], psmValues[i]);
	        }
	}
	
	/* To check if it is an upgrade from release image to parodus ON */
	
	if((strcmp(lastRebootReason,"Software_upgrade")==0 || ((strlen(cfgJson_firmware)>0) && (strcmp(firmwareVersion, cfgJson_firmware))!=0)) && (atoi(psmValues[0]) ==0 && strcmp(psmValues[1], "0")==0 && strcmp(psmValues[2] ,"0")==0))
	{
		LogInfo("sync for bbhm and syscfg is required. Proceeding with DB sync..\n");
		getValuesFromSysCfgDb(paramList, sysCfgValues, paramCount);
		for(i=0; i<paramCount; i++)
		{
			if(sysCfgValues[i] == NULL)
	        	{
	        		LogInfo("SysCfgDb-> value is NULL for %s\n", paramList[i]);
	        		free_sync_db_items(paramCount, psmValues, sysCfgValues);
	        		return -2;
	        	}
	        	else
	        	{
	        		LogInfo("SysCfgDb-> %s value is %s\n", paramList[i], sysCfgValues[i]);
	        	}
		}
		
		status = setValuesToPsmDb(paramList, sysCfgValues, paramCount);
		if(status == 0)
		{
		        LogInfo("Successfully set values to PSM DB\n");
    	}
    	else
    	{
    			LogError("Failed to set values to PSM DB\n");
    			free_sync_db_items(paramCount, psmValues, sysCfgValues);
    			return -2;
    	}
	
		if(cfgJson_firmware != NULL)
		{
			free(cfgJson_firmware);
			cfgJson_firmware = NULL;
		}

    		
	}
	else
	{
		LogInfo("Sync for bbhm and syscfg is not required\n");
		free_sync_db_items(paramCount, psmValues, sysCfgValues);
		return -1;
	}
		
	free_sync_db_items(paramCount, psmValues, sysCfgValues);
	return 0;
}

static void free_sync_db_items(int paramCount, char *psmValues[], char *sysCfgValues[])
{
	int i;
	for(i = 0; i<paramCount; i++)
	{
		if(psmValues[i] != NULL)
		{
	       		free(psmValues[i]);
	       		psmValues[i] = NULL;
	       	}
	       	
	       	if(sysCfgValues[i] != NULL)
		{
	       		free(sysCfgValues[i]);
	       		sysCfgValues[i] = NULL;
	       	}
	}

}
