/************************************************************************************
  If not stated otherwise in this file or this component's Licenses.txt file the
  following copyright and licenses apply:

  Copyright 2018 RDK Management

  Licensed under the Apache License, Version 2.0 (the "License");
  you may not use this file except in compliance with the License.
  You may obtain a copy of the License at

  http://www.apache.org/licenses/LICENSE-2.0

  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.
**************************************************************************/
#include <stdio.h>
#include <sys/sysinfo.h>
#include <time.h>
#include <string.h>

// This function returns 
// 1 if BootTime information is printed in the log file.
// 0 if BootTime information is not printed in the log file. 
int main(int argc, char *argv[])
{
    struct sysinfo l_sSysInfo;
    struct tm * l_sTimeInfo;
    time_t l_sNowTime;
    int l_iDays, l_iHours, l_iMins, l_iSec;
    char l_cLocalTime[32] = {0};
    FILE *l_fBootLogFile = NULL;
	char l_cLine[128] = {0};
    char BOOT_TIME_LOG_FILE[32] = "/rdklogs/logs/BootTime.log";

    if (argc < 2)
    {   
        printf("Insufficient number of args return\n");
        return 0;
    }   
    sysinfo(&l_sSysInfo);
    time(&l_sNowTime);
    l_sTimeInfo = localtime(&l_sNowTime);

    sprintf(l_cLocalTime, "%02d:%02d:%02d",l_sTimeInfo->tm_hour, l_sTimeInfo->tm_min, l_sTimeInfo->tm_sec);

    if(argv[2] != NULL)
    {
        printf("BootUpTime info need to be logged in this file : %s\n", argv[2]);
        strncpy(BOOT_TIME_LOG_FILE, argv[2], sizeof(BOOT_TIME_LOG_FILE)-1);
    }

    l_fBootLogFile = fopen(BOOT_TIME_LOG_FILE, "a+");
    if (NULL != l_fBootLogFile)
    {
        while(fscanf(l_fBootLogFile,"%s", l_cLine) != EOF)
        {
            if(NULL != strstr(l_cLine, argv[1]))
            {
				printf("BootUpTime info for %s is already present not adding it again \n", argv[1]);
				fclose(l_fBootLogFile);
				return 0;
            }
        }
        fprintf(l_fBootLogFile, "%s [BootUpTime] %s=%ld\n", l_cLocalTime, argv[1],l_sSysInfo.uptime);
        fclose(l_fBootLogFile);
    }   
    else //fopen of bootlog file failed atleast write on the console
    {   
        printf("%s [BootUpTime] %s=%ld\n", l_cLocalTime, argv[1],l_sSysInfo.uptime);
    }  
	return 1; 
}
