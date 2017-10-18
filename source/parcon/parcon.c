#include <stdio.h>
#include <string.h>
#include <sys/file.h>
#define PCMD_LIST "/tmp/.pcmd"
#define LOG_FILE "/rdklogs/logs/Parcon.txt"
#define TRUE 1
#define FALSE 0

int validate_mac(char * physAddress)
{
	if(physAddress[2] == ':')
		if(physAddress[5] == ':')
			if(physAddress[8] == ':')
				if(physAddress[11] == ':')
					if(physAddress[14] == ':')
						return TRUE;
					
					
	return FALSE;
}

int main( int argc, char *argv[] )  {
int count = 1;
   printf("argc = %d\n",argc);
   FILE * fp;
   char errbuf[100] = {0};
   system("echo ----------------- >> "LOG_FILE);
   system("echo parcon_entry >> "LOG_FILE"; date >> "LOG_FILE);
   if(argc == 1)
   {
	   system("echo Cleaning the block list >> "LOG_FILE);
   }
   fp = fopen (PCMD_LIST, "w+");
   if(fp != NULL)
   {
        if(flock(fileno(fp), LOCK_EX) == -1)
		printf("Error while locking the file\n");
	fprintf(fp, "%d\n", argc-1);
	   while(count < argc)
	   {
              if(validate_mac(argv[count]))
		{
	      		fprintf(fp, "%s\n", argv[count]);
		}
		else
		{
		    memset(errbuf, 0, sizeof(errbuf));
		    sprintf(errbuf,"echo Error: Invalid input Mac address %s >> " LOG_FILE,argv[count] );
		    system(errbuf);
		}
	      	count++;	
	   }
           system("echo Got the device list, Restarting firewall >> "LOG_FILE);
           system("sysevent set firewall-restart");
           fflush(fp);
           flock(fileno(fp), LOCK_UN);
           fclose(fp);
   }
   else
   {
      system("echo Error: Not able to create" PCMD_LIST " >> "LOG_FILE);
   }

   system("echo parcon_exit >> "LOG_FILE"; date >> "LOG_FILE);
   system("echo ----------------- >> "LOG_FILE);
}
