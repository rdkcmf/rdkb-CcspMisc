#include <time.h>
#include <stdio.h>
#include <string.h>
#ifdef UTC_ENABLE
#include "platform_hal.h"
#endif
#define DATE_MAX_STR_SIZE 26
//#define DATE_FMT "%FT%TZ%z"

#define DATE_FMT "%T"
#define DATE_FMT_H "%H"
#define DATE_FMT_M "%M"

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
int main( int argc, char *argv[]) {

    time_t now_time, now_time_local,off;
    struct tm now_tm_utc, now_tm_local;
    char str_utc[DATE_MAX_STR_SIZE];
    char str_local[DATE_MAX_STR_SIZE];

    off = getOffset();
    
    time(&now_time);
    now_time_local =  now_time + off;
    localtime_r(&now_time_local, &now_tm_local);
    
     if( argc == 2 ) {
          if(argv[1][0] == 'H')
          {
              strftime(str_local, DATE_MAX_STR_SIZE, DATE_FMT_H, &now_tm_local);
              printf("%s\n", str_local);
          }
     if(argv[1][0] == 'M')
      {
          strftime(str_local, DATE_MAX_STR_SIZE, DATE_FMT_M, &now_tm_local);
          printf("%s\n", str_local);
      }
      
  }
  else if( argc > 2 ) {
     printf("Too many arguments supplied.\n");
  }
  else {
    strftime(str_local, DATE_MAX_STR_SIZE, DATE_FMT, &now_tm_local);
    printf("%s\n", str_local);
  }

   return 0;
}
