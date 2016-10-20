#include <stdio.h>
#include <time.h>
#include <string.h>
#ifdef UTC_ENABLE
#include "platform_hal.h"
#endif
#define DATE_MAX_STR_SIZE 26
//#define DATE_FMT "%FT%TZ%z"
#define DATE_FMT "%FT%TZ"
#define DATE_FMT_1 "%T"
char *offset = "-25200";
int ConvLocalToUTC(char* LocalTime, char* UtcTime);

time_t getOffset()
{
    time_t off;
    char a[100];
    char *pTimeOffset = a;
#ifdef UTC_ENABLE
    platform_hal_getTimeOffSet(pTimeOffset);
    off = atoi(pTimeOffset);
#endif
    return off;
}
void getUTCTimeStr(char *pTime)
{
   time_t now_time, now_time_local,off;
   struct tm now_tm_utc, now_tm_local;
   char str_utc[DATE_MAX_STR_SIZE];
   char str_local[DATE_MAX_STR_SIZE];
   char Utc[30];
    
   time(&now_time);
   localtime_r(&now_time, &now_tm_local);
   /* human readable */
   strftime(str_local, DATE_MAX_STR_SIZE, DATE_FMT, &now_tm_local);
   strcpy(pTime,str_local);
}

void getLocalTimeStr(char *pTime)
{
   time_t now_time, now_time_local,off;
   struct tm now_tm_utc, now_tm_local;
   char str_utc[DATE_MAX_STR_SIZE];
   char str_local[DATE_MAX_STR_SIZE];
   char Utc[30];

   time(&now_time);
   now_time = now_time + getOffset();
   localtime_r(&now_time, &now_tm_local);
   /* human readable */
   strftime(str_local, DATE_MAX_STR_SIZE, DATE_FMT, &now_tm_local);
   strcpy(pTime,str_local);
}
int ConvLocalToUTC(char* LocalTime, char* UtcTime)
{
    char LTime[30];
    char TmpTime[30];
    int len = 0;
    /* LOCAL: 2016-09-30T12:34:11Z
   */
    struct tm cal = {};
    struct tm now_tm_utc;
    memset(LTime, 0, sizeof(LTime));
    memset(TmpTime, 0, sizeof(TmpTime));
    len = validateTime(LocalTime);
    getLocalTimeStr(LTime);
    strncpy(TmpTime,LTime,11);
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
    char TmpTime[30];
    int len = 0;
    memset(LTime,0,30);
    memset(TmpTime,0,30);
    /* LOCAL: 2016-09-30T12:34:11Z
   */
    struct tm cal = {};
    struct tm now_tm_utc;
    len = validateTime(UtcTime);
    //getLocalTimeStr(LTime);
    getUTCTimeStr(LTime);
    strncpy(TmpTime,LTime,11);
    strncat(TmpTime, UtcTime, 11);
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
}


