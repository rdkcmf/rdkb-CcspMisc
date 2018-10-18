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
#include <time.h>
#include <string.h>
#ifdef UTC_ENABLE
#include "platform_hal.h"
#endif
#define DATE_MAX_STR_SIZE 26
//#define DATE_FMT "%FT%TZ%z"
#define DATE_FMT "%FT%TZ"
#define DATE_FMT_1 "%R"
#define DATE_FMT_2 "%F"
char *offset = "-25200";
int ConvLocalToUTC(char* LocalTime, char* UtcTime);

time_t getOffset()
{
    time_t off = 0;
    char a[100];
    char *pTimeOffset = a;
#ifdef UTC_ENABLE
    platform_hal_getTimeOffSet(pTimeOffset);
    off = atoi(pTimeOffset);
#endif
    return off;
}
void getUTCTimeStr(char *pTime,char *pDate)
{
   time_t now_time, now_time_local,off;
   struct tm now_tm_utc, now_tm_local;
   char str_utc[DATE_MAX_STR_SIZE];
   char str_local[DATE_MAX_STR_SIZE];
   char str_date[DATE_MAX_STR_SIZE];
   char Utc[30];
    
   time(&now_time);
   localtime_r(&now_time, &now_tm_local);
   /* human readable */
   strftime(str_local, DATE_MAX_STR_SIZE, DATE_FMT, &now_tm_local);
   strftime(str_date, DATE_MAX_STR_SIZE, DATE_FMT, &now_tm_local);
   strcpy(pTime,str_local);
   strcpy(pDate,str_date);

}

void getLocalTimeStr(char *pTime,char *pDate)
{
   time_t now_time, now_time_local,off;
   struct tm now_tm_utc, now_tm_local;
   char str_utc[DATE_MAX_STR_SIZE];
   char str_local[DATE_MAX_STR_SIZE];
   char str_date[DATE_MAX_STR_SIZE];
   char Utc[30];

   time(&now_time);
   now_time = now_time + getOffset();
   localtime_r(&now_time, &now_tm_local);
   /* human readable */
   strftime(str_local, DATE_MAX_STR_SIZE, DATE_FMT, &now_tm_local);
   strftime(str_date, DATE_MAX_STR_SIZE, DATE_FMT_2, &now_tm_local);
   strcpy(pTime,str_local);
   strcpy(pDate,str_date);
}
int ConvLocalToUTC(char* LocalTime, char* UtcTime)
{
    char LTime[30];
    char TmpTime[30];
    int len = 0;
	char LDate[30];
	char UtcDate[30];
    int ret = 0;
    /* LOCAL: 2016-09-30T12:34:11Z
   */
    struct tm cal = {};
    struct tm now_tm_utc;
    memset(LTime, 0, sizeof(LTime));
    memset(TmpTime, 0, sizeof(TmpTime));
    memset(LDate, 0, sizeof(LDate));
    memset(UtcDate, 0, sizeof(UtcDate));
    len = validateTime(LocalTime);
    getLocalTimeStr(LTime,LDate);
    strncpy(TmpTime,LTime,11);
    if(len == 4)
    {
        strncat(TmpTime,"0",1);
        len++;
    }
    strncat(TmpTime, LocalTime, 11);

    if(len == 5)
        {
            strncat(TmpTime, ":00Z", 4);
        }
    else
    strncat(TmpTime, "Z", 1);

    strptime(TmpTime, DATE_FMT, &cal);
   // Tell mktime to figure out the daylight saving time
   cal.tm_isdst = -1;
   // Convert struct tm to time_t
   time_t t = mktime(&cal);
   t = t - getOffset();
   // Convert time_t to localtime
   localtime_r(&t, &now_tm_utc);
   /* human readable */
   strftime(UtcTime, DATE_MAX_STR_SIZE, DATE_FMT_1, &now_tm_utc);
   strftime(UtcDate, DATE_MAX_STR_SIZE, DATE_FMT_2, &now_tm_utc);
   	printf("\nUTC Date: %s\n", UtcDate);
	printf("\nLocal Date: %s\n", LDate);

	ret = strcmp(UtcDate,LDate);
	printf("\n cmp = %d \n",ret);
	return ret;
}
int validateTime(char *pTime)
{
    int TimeLen = strlen(pTime);
    if(6 > TimeLen)
    {
        return TimeLen;
    }
    else if(9 > TimeLen)
    {
        return TimeLen;
    }
    else
    {
         printf("Invalid time input\n");
         return 0;    
    }
}

int ConvUTCToLocal( char* UtcTime, char* LocalTime)
{
    char LTime[30];
	char LDate[30];
    char TmpTime[30];
	char UtcDate[30];
    int  len = 0;
    int  ret = 0;
    memset(LTime,0,30);
    memset(TmpTime,0,30);
	memset(LDate,0,30);
    memset(UtcDate,0,30);
    /* LOCAL: 2016-09-30T12:34:11Z
   */
    struct tm cal = {};
    struct tm now_tm_utc;
    len = validateTime(UtcTime);
    //getLocalTimeStr(LTime);
    getUTCTimeStr(LTime,UtcDate);

    strncpy(TmpTime,LTime,11);

    strncat(TmpTime, UtcTime, 11);
    if(len == 4)
    {
        strncat(TmpTime,"0",1);
        len++;
    }

    if(len == 5)
	{
	    strncat(TmpTime, ":00Z", 4);
	}
    else
    strncat(TmpTime, "Z", 1);


   // Read string into struct tm
   strptime(TmpTime, DATE_FMT, &cal);
   cal.tm_isdst = -1;
   // Convert struct tm to time_t
   time_t t = mktime(&cal);
   t = t + getOffset();
   // Convert time_t to localtime
   localtime_r(&t, &now_tm_utc);
   /* human readable */
    strftime(LocalTime, DATE_MAX_STR_SIZE, DATE_FMT_1, &now_tm_utc);
    strftime(LDate, DATE_MAX_STR_SIZE, DATE_FMT_2, &now_tm_utc);
   	ret = strcmp(LDate,UtcDate);
	printf("\nUTC Date: %s\n", UtcDate);
	printf("\nLocal Date: %s\n", LDate);
    printf("\n cmp = %d \n",ret); 
    return ret;
}
char *str[] = {"Mon","Tue","Wed","Thu","Fri","Sat","Sun"};

void pick_days(char *days_rule,int *Days_Mod)
{
   int i = 0;
   char day_str[64];
   memset(day_str,0,64);
   for(i = 0;i<7;i++)
   {
       if(Days_Mod[i] == 1)
	   {
           strcat(day_str,str[i]);
           strcat(day_str,",");
	   }
   }
   day_str[strlen(day_str)-1] = '\0';
   printf("rules %s\n",day_str);
   strcpy(days_rule,day_str);

}

void scan_days(char* days,int *Days)
{
printf("scan day %s\n",days);
Days[0] = 0;
Days[1] = 0;
Days[2] = 0;
Days[3] = 0;
Days[4] = 0;
Days[5] = 0;
Days[6] = 0;
    if(strstr(days,"Mon"))
	{            
        Days[0] = 1;
	}
    if(strstr(days,"Tue"))
	{
        Days[1] = 1;
	}
    if(strstr(days,"Wed"))
	{
        Days[2] = 1;
	}
    if(strstr(days,"Thu"))
	{
        Days[3] = 1;
	}
    if(strstr(days,"Fri"))
	{
		Days[4] = 1;
	}
    if(strstr(days,"Sat"))
	{
		Days[5] = 1;
	}
    if(strstr(days,"Sun"))
	{
        Days[6] = 1;
	}
}

ModifyDay(int cflag,int *Days_Mod,int *Days)
{
    int i = 0;
    if(cflag == 1)
	{
		for(i = 0;i<7;i++)
		{
			if(i < 6)
		     {
				 Days_Mod[i+1] = Days[i];
             }
            else
             {
				 Days_Mod[0] = Days[i];
             }
		}
	}
	else if(cflag == -1)
	{
		for(i = 0;i<7;i++)
		{
			if(i < 6)
			  {
				  Days_Mod[i] = Days[i+1];
			  }
			  else
			  {
				  Days_Mod[i] = Days[0];
			  }	   
		}
	}
	else
	{
		for(i = 0;i<7;i++)
		{
			Days_Mod[i] = Days[i];		   
		}	
	}
}

int split_BlockDays(int sRet, int eRet, char *sBDays, char *eBDays)
{
    int Days[7] = {0};
    int Days_Mod[7] = {0};
    char days[64];
    char days_rule[64];
    int i =0;
	
    memset(days,0,64);
    memset(Days,0,7);
    memset(Days_Mod,0,7);
    strcpy(days,sBDays);
	if(sRet == eRet)
	{
		if(sRet == 0)
		{
	         printf("No change in both date\n");
		     strcpy(eBDays,sBDays);
		}
		else
		{
		    printf("change in both date\n");
		    printf("day - %s\n",days);
            scan_days(days,Days);
            ModifyDay(sRet,Days_Mod,Days);
            pick_days(days_rule,Days_Mod);
            memset(sBDays,0,64);
		    strcpy(sBDays,days_rule);
		    strcpy(eBDays,days_rule);
		}
	return 0;
	}
   else
	{
		if(sRet == 0)
		{
	             printf("No change in start date\n");
		}
		else 
		{
		     printf("change in start date\n");
			 printf("day - %s\n",days);
             scan_days(days,Days);
             ModifyDay(sRet,Days_Mod,Days);
             pick_days(days_rule,Days_Mod);
             memset(sBDays,0,64);
		     strcpy(sBDays,days_rule);
		}
		if(eRet == 0)
		{
	         printf("No change in end date\n");
		}
		else
		{
		    printf("change in end date\n");
            scan_days(days,Days);
   			ModifyDay(eRet,Days_Mod,Days);
            pick_days(days_rule,Days_Mod);
            memset(eBDays,0,64);
		    strcpy(eBDays,days_rule);
		}
	return 1;
	}
}
