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
#include<stdio.h>
#include<sys/inotify.h>
#include<limits.h>
#include<pthread.h>
#include "secure_wrapper.h"
#define DHCP_LEASE_FILE "/nvram/dnsmasq.leases"

#define EVENT_SIZE  ( sizeof (struct inotify_event) )
#define BUF_LEN (10 * (EVENT_SIZE + NAME_MAX + 1))

int IsFileExists(const char *fname)
{
    FILE *file;
    if (file = fopen(fname, "r"))
    {
        fclose(file);
        return 1;
    }
    return 0;
}

void* MonitorDHCPLeaseFile(void *arg)
{
    int inotifyFd, inotifywd, j;
    int  numRead=0;
    char buf[BUF_LEN];
    int ret=0;
    char *p;
    struct inotify_event *event;
    inotifyFd = inotify_init();                 /* Create inotify instance */
    if (inotifyFd == -1)
    {
            printf("Failed to initialize inotify\n", __FUNCTION__ );
            return;
     }
     ret = IsFileExists(DHCP_LEASE_FILE);
     while(ret != 1)
     {
	     sleep(5);
	     ret = IsFileExists(DHCP_LEASE_FILE);
     }

     v_secure_system("/etc/utopia/service.d/service_lan/dhcp_lease_sync.sh");      
     inotifywd = inotify_add_watch(inotifyFd, DHCP_LEASE_FILE, IN_MODIFY);
      if (inotifywd == -1)
     {
          printf("inotify_add_watch failed\n", __FUNCTION__ );
            return -1;
      }

     for (;;)
    {                                  /* Read events forever */
   	     numRead = read(inotifyFd, buf, BUF_LEN);


	     if (numRead < 0)
	    {
		    printf(" Error returned is = numRead %d  \n", numRead);
	    }

	     /* Process all of the events in buffer returned by read() */
	     for (p = buf; p < buf + numRead; )
	    {
		 event = (struct inotify_event *) p;
		 if (event->mask & IN_MODIFY)
		 {
			v_secure_system("/etc/utopia/service.d/service_lan/dhcp_lease_sync.sh");
		 } 
		 p += sizeof(struct inotify_event) + event->len;
	     }

    }
}

void main()
{

	  pid_t process_id = 0;
	  // Create child process
	  process_id = fork();
	  // Indication of fork() failure
	 if (process_id < 0)
	 {
		printf("fork failed!\n");
		// Return failure in exit status
		exit(1);
	 }
	// PARENT PROCESS. Need to kill it.
	 if (process_id > 0)
	 {
		printf("process_id of child process %d \n", process_id);
	 	// return success in exit status
		exit(0);
	 }

     pthread_t tid;
	if (pthread_create(&tid, NULL, MonitorDHCPLeaseFile, NULL))
	{
		printf("Failed to create a thread\n");
		return -1;
	}

        while(1)
       {
            sleep(30);
       }
}
