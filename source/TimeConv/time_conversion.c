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
#define _GNU_SOURCE 1
#include <stdio.h>
#include <time.h>
#include <string.h>
#ifdef UTC_ENABLE
#include "platform_hal.h"
#include <stdlib.h>
#include <unistd.h>
#endif
#include "safec_lib_common.h"

#define DATE_MAX_STR_SIZE 26
//#define DATE_FMT "%FT%TZ%z"
#define DATE_FMT "%FT%TZ"
#define DATE_FMT_1 "%R"
#define DATE_FMT_2 "%F"
char *offset = "-25200";

#define LOCAL_TIME_SIZE		30
#define LOCAL_DATE_SIZE		30
#define UTC_DATE_SIZE		30
#define DAYS_RULE_SIZE		64

int ConvLocalToUTC(char* LocalTime, char* UtcTime);
static int getLocalTimeStr(char *pTime, char *pDate);
static int validateTime(char *pTime);

time_t getOffset()
{
    time_t off = 0;
#ifdef UTC_ENABLE
    char a[100]; char cmd[100];
    FILE *fp;
    if(!access("/nvram/ETHWAN_ENABLE", 0))
    {
        snprintf(cmd,sizeof(cmd),"sysevent get ipv4-timeoffset");
        fp = popen(cmd, "r");
        /* CID:60154 Dereference null return value*/
        if (!fp) {
           perror("sysevent get ipv4-timeoffset doesn't exist");
           return off;
        }
        fgets(a,sizeof(a),fp);
        pclose(fp);
        off = atoi(a + 1);
    }
    else
    {
    platform_hal_getTimeOffSet(a);
    off = atoi(a);
    }
#endif
    return off;
}
static int getUTCTimeStr(char *pTime,char *pDate)
{
   time_t now_time;
   struct tm now_tm_local;
   char str_local[DATE_MAX_STR_SIZE];
   char str_date[DATE_MAX_STR_SIZE];
   errno_t rc = -1;

   time(&now_time);
   localtime_r(&now_time, &now_tm_local);
   /* human readable */
   strftime(str_local, DATE_MAX_STR_SIZE, DATE_FMT, &now_tm_local);
   strftime(str_date, DATE_MAX_STR_SIZE, DATE_FMT, &now_tm_local);
   char *pStr_local = str_local;
   rc = strcpy_s(pTime, LOCAL_TIME_SIZE, pStr_local);
   if( rc != EOK )
   {
      ERR_CHK(rc);
      return( 0 );
   }

   char *pStr_date = str_date;
   rc = strcpy_s(pDate, UTC_DATE_SIZE, pStr_date);
   if( rc != EOK )
   {
      ERR_CHK(rc);
      return( 0 );
   }

   return( 1 );
}

static int getLocalTimeStr(char *pTime,char *pDate)
{
   time_t now_time;
   struct tm now_tm_local;
   char str_local[DATE_MAX_STR_SIZE];
   char str_date[DATE_MAX_STR_SIZE];
   errno_t rc = -1;

   time(&now_time);
   now_time = now_time + getOffset();
   localtime_r(&now_time, &now_tm_local);
   /* human readable */
   strftime(str_local, DATE_MAX_STR_SIZE, DATE_FMT, &now_tm_local);
   strftime(str_date, DATE_MAX_STR_SIZE, DATE_FMT_2, &now_tm_local);
   char *pStr_local = str_local;
   rc = strcpy_s(pTime, LOCAL_TIME_SIZE, pStr_local);
   if( rc != EOK )
   {
      ERR_CHK(rc);
      return( 0 );
   }
   char *pStr_date = str_date;
   rc = strcpy_s(pDate, LOCAL_DATE_SIZE, pStr_date);
   if( rc != EOK )
   {
      ERR_CHK(rc);
      return( 0 );
   }

   return( 1 );
}
int ConvLocalToUTC(char* LocalTime, char* UtcTime)
{
    char LTime[LOCAL_TIME_SIZE] = { 0 };
    char TmpTime[30] = { 0 };
    int len = 0;
	char LDate[LOCAL_DATE_SIZE] = { 0 };
	char UtcDate[UTC_DATE_SIZE] = { 0 };
	errno_t rc = -1;
	int ind = -1;
	int result = 0;
    /* LOCAL: 2016-09-30T12:34:11Z
   */
    struct tm cal = {0};
    struct tm now_tm_utc = {0};

    len = validateTime(LocalTime);
    result = getLocalTimeStr(LTime,LDate);
    if( result == 0 )
    {
		/* This function returns the result of strcmp() values.
		 * But for safec changes, we modified to return the error codes,
		 * when there is any failure in the safec string functions.
		 *
		 * Since the caller of this api is in different component,
		 * We need to sync with them and modify this api prototype,
		 * For returning the proper return types.
		 */
        return(-1);
    }

    char *pLTime = LTime;
    rc = strcpy_s(TmpTime, sizeof(TmpTime), pLTime);
	if( rc != EOK )
	{
		ERR_CHK(rc);

		/* This function returns the result of strcmp() values.
		 * But for safec changes, we modified to return the error codes,
		 * when there is any failure in the safec string functions.
		 *
		 * Since the caller of this api is in different component,
		 * We need to sync with them and modify this api prototype,
		 * For returning the proper return types.
		 */
		return(-1);
	}

    if(len == 4)
    {
        rc = strncat_s(TmpTime, sizeof(TmpTime), "0", 1);
		if( rc != EOK )
		{
		   ERR_CHK(rc);

		   /* This function returns the result of strcmp() values.
		    * But for safec changes, we modified to return the error codes,
		    * when there is any failure in the safec string functions.
		    *
		    * Since the caller of this api is in different component,
		    * We need to sync with them and modify this api prototype,
		    * For returning the proper return types.
		    */
		   return(-1);
		}

        len++;
    }
    rc = strncat_s(TmpTime, sizeof(TmpTime), LocalTime, strlen(LocalTime));
	if( rc != EOK )
	{
		ERR_CHK(rc);

		/* This function returns the result of strcmp() values.
		 * But for safec changes, we modified to return the error codes,
		 * when there is any failure in the safec string functions.
		 *
		 * Since the caller of this api is in different component,
		 * We need to sync with them and modify this api prototype,
		 * For returning the proper return types.
		 */
		return(-1);
	}

    if(len == 5)
        {
            rc = strncat_s(TmpTime, sizeof(TmpTime), ":00Z", 4);
			if( rc != EOK )
			{
			   ERR_CHK(rc);

			   /* This function returns the result of strcmp() values.
			    * But for safec changes, we modified to return the error codes,
			    * when there is any failure in the safec string functions.
			    *
			    * Since the caller of this api is in different component,
			    * We need to sync with them and modify this api prototype,
			    * For returning the proper return types.
			    */
			   return(-1);
			}
        }
    else
    {
	   rc = strncat_s(TmpTime, sizeof(TmpTime), "Z", 1);
	   if( rc != EOK )
	   {
		   ERR_CHK(rc);

		   /* This function returns the result of strcmp() values.
		    * But for safec changes, we modified to return the error codes,
		    * when there is any failure in the safec string functions.
		    *
		    * Since the caller of this api is in different component,
		    * We need to sync with them and modify this api prototype,
		    * For returning the proper return types.
		    */
		   return(-1);
	   }
    }

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

	rc = strcmp_s( UtcDate, strlen(UtcDate), LDate, &ind);
	ERR_CHK(rc);

	printf("\n cmp = %d \n", ind);
	return ind;
}
static int validateTime(char *pTime)
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
    char LTime[LOCAL_TIME_SIZE] = { 0 };
	char LDate[LOCAL_DATE_SIZE] = { 0 };
    char TmpTime[30] = { 0 };
	char UtcDate[UTC_DATE_SIZE] = { 0 };
    int  len = 0;
	errno_t rc = -1;
	int ind = -1;
	int result = 0;

    /* LOCAL: 2016-09-30T12:34:11Z
   */
    struct tm cal = {0};
    struct tm now_tm_utc = {0};
    len = validateTime(UtcTime);
    //getLocalTimeStr(LTime);
    result = getUTCTimeStr(LTime,UtcDate);
    if( result == 0 )
    {
        /* This function returns the result of strcmp() values.
         * But for safec changes, we modified to return the error codes,
         * when there is any failure in the safec string functions.
         *
         * Since the caller of this api is in different component,
         * We need to sync with them and modify this api prototype,
         * For returning the proper return types.
         */
        return(-1);
    }

    char *pLTime = LTime;
    rc = strcpy_s(TmpTime, sizeof(TmpTime), pLTime);
    if( rc != EOK )
    {
        ERR_CHK(rc);

        /* This function returns the result of strcmp() values.
         * But for safec changes, we modified to return the error codes,
         * when there is any failure in the safec string functions.
         *
         * Since the caller of this api is in different component,
         * We need to sync with them and modify this api prototype,
         * For returning the proper return types.
         */
        return(-1);
    }

    rc = strncat_s(TmpTime, sizeof(TmpTime), UtcTime, strlen(UtcTime));
    if( rc != EOK )
    {
        ERR_CHK(rc);

        /* This function returns the result of strcmp() values.
         * But for safec changes, we modified to return the error codes,
         * when there is any failure in the safec string functions.
         *
         * Since the caller of this api is in different component,
         * We need to sync with them and modify this api prototype,
         * For returning the proper return types.
         */
        return(-1);
    }
    if(len == 4)
    {
        rc = strncat_s(TmpTime, sizeof(TmpTime), "0",1);
        if( rc != EOK )
        {
           ERR_CHK(rc);

           /* This function returns the result of strcmp() values.
            * But for safec changes, we modified to return the error codes,
            * when there is any failure in the safec string functions.
            *
            * Since the caller of this api is in different component,
            * We need to sync with them and modify this api prototype,
            * For returning the proper return types.
            */
           return(-1);
        }

        len++;
    }

    if(len == 5)
	{
	    rc = strncat_s(TmpTime, sizeof(TmpTime), ":00Z", 4);
		if( rc != EOK )
        {
           ERR_CHK(rc);

           /* This function returns the result of strcmp() values.
            * But for safec changes, we modified to return the error codes,
            * when there is any failure in the safec string functions.
            *
            * Since the caller of this api is in different component,
            * We need to sync with them and modify this api prototype,
            * For returning the proper return types.
            */
           return(-1);
        }
	}
    else
    {
       rc = strncat_s(TmpTime, sizeof(TmpTime), "Z", 1);
	    if( rc != EOK )
        {
           ERR_CHK(rc);

           /* This function returns the result of strcmp() values.
            * But for safec changes, we modified to return the error codes,
            * when there is any failure in the safec string functions.
            *
            * Since the caller of this api is in different component,
            * We need to sync with them and modify this api prototype,
            * For returning the proper return types.
            */
           return(-1);
        }
	   
    }


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

	rc = strcmp_s( LDate, strlen(LDate), UtcDate, &ind);
	ERR_CHK(rc);

	printf("\nUTC Date: %s\n", UtcDate);
	printf("\nLocal Date: %s\n", LDate);
    printf("\n cmp = %d \n", ind); 

    return ind;
}
char *str[] = {"Mon","Tue","Wed","Thu","Fri","Sat","Sun"};

static int pick_days(char *days_rule,int *Days_Mod)
{
   int i = 0;
   char day_str[64] = { 0 };
   errno_t rc = -1;

   for(i = 0;i<7;i++)
   {
       if(Days_Mod[i] == 1)
	   {
           rc = strcat_s(day_str, sizeof(day_str), str[i]);
           if( rc != EOK )
           {
              ERR_CHK(rc);
              return( 0 );
           }

           rc = strcat_s(day_str, sizeof(day_str), ",");
           if( rc != EOK )
           {
              ERR_CHK(rc);
              return( 0 );
           }
	   }
   }
   day_str[strlen(day_str)-1] = '\0';
   printf("rules %s\n",day_str);
   char *pDay_str = day_str;
   rc = strcpy_s(days_rule, DAYS_RULE_SIZE, pDay_str);
   if( rc != EOK )
   {
      ERR_CHK(rc);
      return( 0 );
   }

   return( 1 );
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

void ModifyDay(int cflag,int *Days_Mod,int *Days)
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
    char days[64] = { 0 };
    char days_rule[DAYS_RULE_SIZE];
    errno_t rc = -1;
    int result = 0;

	if(( sBDays == NULL ) || ( eBDays == NULL ) )
		return ( -1 );

    rc = strcpy_s(days, sizeof(days), sBDays);
    if( rc != EOK )
    {
       ERR_CHK(rc);
       return ( -1 );
    }

	if(sRet == eRet)
	{
		if(sRet == 0)
		{
	         printf("No change in both date\n");
	         rc = strcpy_s(eBDays, DAYS_RULE_SIZE, sBDays);
	         if( rc != EOK )
	         {
	            ERR_CHK(rc);
	            return ( -1 );
	         }
		}
		else
		{
		    printf("change in both date\n");
		    printf("day - %s\n",days);
            scan_days(days,Days);
            ModifyDay(sRet,Days_Mod,Days);
            result = pick_days(days_rule,Days_Mod);
			if( result == 0 )
            {
               return ( -1 );
            }

            rc = memset_s(sBDays,DAYS_RULE_SIZE,0,DAYS_RULE_SIZE);
            ERR_CHK(rc);
            char *pDays_rule = days_rule;
            rc = strcpy_s(sBDays, DAYS_RULE_SIZE, pDays_rule);
            if( rc != EOK )
            {
               ERR_CHK(rc);
               return ( -1 );
            }
            rc = strcpy_s(eBDays, DAYS_RULE_SIZE, pDays_rule);
            if( rc != EOK )
            {
               ERR_CHK(rc);
               return ( -1 );
            }
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
             result = pick_days(days_rule,Days_Mod);
			 if( result == 0 )
             {
                return ( -1 );
             }

             rc = memset_s(sBDays,DAYS_RULE_SIZE,0,DAYS_RULE_SIZE);
             ERR_CHK(rc);
             char *pDays_rule = days_rule;
             rc = strcpy_s(sBDays, DAYS_RULE_SIZE, pDays_rule);
             if( rc != EOK )
             {
                ERR_CHK(rc);
                return ( -1 );
             }
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
            result = pick_days(days_rule,Days_Mod);
            if( result == 0 )
            {
               return ( -1 );
            }

            rc = memset_s(eBDays,DAYS_RULE_SIZE,0,DAYS_RULE_SIZE);
            ERR_CHK(rc);
            char *p_days_rule = days_rule;
            rc = strcpy_s(eBDays, DAYS_RULE_SIZE, p_days_rule);
            if( rc != EOK )
            {
               ERR_CHK(rc);
               return ( -1 );
            }
		}
	return 1;
	}
}
