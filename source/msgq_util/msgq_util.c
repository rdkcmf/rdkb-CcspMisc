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
#include <errno.h>            // errno, perror()
#include <sys/socket.h>
#include <linux/netlink.h>
#include <unistd.h>
#include <mqueue.h>
#include <string.h>

#define MAX_MSG_SIZE    512
#define MAX_MSGDATA_SIZE 256

typedef enum _MSGQOption
{
    MSG_SEND = 0,
    MSG_RECEIVE
}MSGQOption;


typedef enum _MSGQ
{
    MSG_UNKNOWN = -1,
    GW_SM_MSG = 1
}MSGQ;

typedef struct _EventData
{
    int MsgType;
    int Event;
    char msg[MAX_MSGDATA_SIZE];
}EventData;

typedef enum event_t
{
	    GM_EVENT_STA_MODE_CONNECTED,
    GM_EVENT_AP_MODE_CONNECTED,
    GM_EVENT_AP_MODE_DISCONNECTED,
    GM_EVENT_STA_MODE_DISCONNECTED,
    GM_EVENT_GRE_TUNNEL_ACTIVE,
    GM_EVENT_GRE_TUNNEL_INACTIVE,
    GM_EVENT_WAN_PRIMARY_UP,
    GM_EVENT_WAN_PRIMARY_DOWN,
    GM_EVENT_WAN_SECONDARY_UP,
    GM_EVENT_WAN_SECONDARY_DOWN,
    GM_EVENT_DEV_REMOTE_ACTIVE,
    GM_EVENT_DEV_REMOTE_INACTIVE,
    GM_EVENT_DEV_SUPPORTEDMODE_CHANGED,
    GM_EVENT_ACTIVEGATEWAY_ACK_RECEIVED,
    GM_EVENT_POLICY_TABLE_UPDATED,
    GM_EVENT_FAIL_OVER_STATUS_UPDATED,
    GM_EVENT_STATIMEOUT_UPDATED,
    GM_EVENT_HEALTH_CHECK_TIMEOUT_UPDATED,
    GM_EVENT_DEV_REMOTE_SYNC_COMPLETED,
    GM_EVENT_DEV_REQ_ACTIVEGATEWAY_INACTIVE,
    GM_EVENT_DEV_BACKUPINACTIVE_WAIT_TIMEOUT_UPDATED,
    GM_EVENT_TIMEDOUT,
    GM_EVENT_HEALTH_CHECK_COMPLETED,
    GM_EVENT_DEV_PRIMARY_WAN_CHECK_STATUS_UPDATED,
    GM_EVENT_DEV_PRIMARY_WAN_IP_UPDATED,
    GM_EVENT_DEVICE_NETWORK_MODE_UPDATED,
    GM_EVENT_DUMP_LOG,
    GM_EVENT_REMDEV_CAPABILITIES_UPDATED,
    GM_EVENT_REMDEV_UPDATED,
    GM_EVENT_REMOTE_WAN_ACTIVE,
    GM_EVENT_REMOTE_WAN_INACTIVE,
    GM_EVENT_REMOTE_DEV_CLEARDB,
    GM_EVENT_REQ_MESHBACKHAUL_IFNAME_UPDATE,
    GM_EVENT_MESHBACKHAUL_IFNAME_UPDATED,
    GM_EVENT_REMOTEDEVICEDETECTION_TIMEOUT_UPDATED,
    GM_EVENT_MAX
}event_t;

int event[] ={
    GM_EVENT_STA_MODE_CONNECTED,
    GM_EVENT_AP_MODE_CONNECTED,
    GM_EVENT_AP_MODE_DISCONNECTED,
    GM_EVENT_STA_MODE_DISCONNECTED,
    GM_EVENT_GRE_TUNNEL_ACTIVE,
    GM_EVENT_GRE_TUNNEL_INACTIVE,
    GM_EVENT_WAN_PRIMARY_UP,
    GM_EVENT_WAN_PRIMARY_DOWN,
    GM_EVENT_WAN_SECONDARY_UP,
    GM_EVENT_WAN_SECONDARY_DOWN,
    GM_EVENT_DEV_REMOTE_ACTIVE,
    GM_EVENT_DEV_REMOTE_INACTIVE,
    GM_EVENT_DEV_SUPPORTEDMODE_CHANGED,
    GM_EVENT_ACTIVEGATEWAY_ACK_RECEIVED,
    GM_EVENT_POLICY_TABLE_UPDATED,
    GM_EVENT_FAIL_OVER_STATUS_UPDATED,
    GM_EVENT_STATIMEOUT_UPDATED,
    GM_EVENT_HEALTH_CHECK_TIMEOUT_UPDATED,
    GM_EVENT_DEV_REMOTE_SYNC_COMPLETED,
    GM_EVENT_DEV_REQ_ACTIVEGATEWAY_INACTIVE,
    GM_EVENT_DEV_BACKUPINACTIVE_WAIT_TIMEOUT_UPDATED,
    GM_EVENT_TIMEDOUT,
    GM_EVENT_HEALTH_CHECK_COMPLETED,
    GM_EVENT_DEV_PRIMARY_WAN_CHECK_STATUS_UPDATED,
    GM_EVENT_DEV_PRIMARY_WAN_IP_UPDATED,
    GM_EVENT_DEVICE_NETWORK_MODE_UPDATED,
    GM_EVENT_DUMP_LOG,
    GM_EVENT_REMDEV_CAPABILITIES_UPDATED,
    GM_EVENT_REMDEV_UPDATED,
    GM_EVENT_REMOTE_WAN_ACTIVE,
    GM_EVENT_REMOTE_WAN_INACTIVE,
    GM_EVENT_REMOTE_DEV_CLEARDB,
    GM_EVENT_REQ_MESHBACKHAUL_IFNAME_UPDATE,
    GM_EVENT_MESHBACKHAUL_IFNAME_UPDATED,
    GM_EVENT_REMOTEDEVICEDETECTION_TIMEOUT_UPDATED
};


int SendmsgToQ(char *queueName, EventData *pEventData)
{
    mqd_t mqTmp;
    char buffer[MAX_MSG_SIZE + 1];

    if (pEventData == NULL || queueName == NULL)
        return -1;

    printf("\n %s Mq Send name %s\n",__FUNCTION__,queueName);
    mqTmp = mq_open(queueName, O_WRONLY | O_NONBLOCK);
    if ((mqd_t) -1 != mqTmp)
    {
    printf("\n %s Mq Send name %s mq %d\n",__FUNCTION__,queueName,mqTmp);
        memset(buffer, 0, sizeof(buffer));
        pEventData->MsgType = GW_SM_MSG;
        memcpy(buffer,pEventData,sizeof(EventData));
        if ( (mqd_t) -1 == mq_send(mqTmp, buffer, MAX_MSG_SIZE, 0) )
        {
            // failed to send
            printf("\n Mq Send failed %s errno %d \n",__FUNCTION__,errno);
        }
        mq_close(mqTmp);
    }
    return 0;
}

int main(int argc, char *argv[])
{
    char *qname = "/gwmgr_smqueue";
    int qoption = 0;
    int eventtype = 0;
    if (argc  < 2)
    {
    printf(" \n Choose event value :  \n");
    printf ("\n GM_EVENT_STA_MODE_CONNECTED = 0 \n");
    printf ("\n GM_EVENT_AP_MODE_CONNECTED = 1 \n");
    printf ("\n GM_EVENT_AP_MODE_DISCONNECTED = 2 \n");
printf ("\n GM_EVENT_STA_MODE_DISCONNECTED = 3 \n");
printf ("\n GM_EVENT_GRE_TUNNEL_ACTIVE = 4 \n");
    printf ("\n GM_EVENT_GRE_TUNNEL_INACTIVE = 5 \n");
    printf ("\n GM_EVENT_WAN_PRIMARY_UP = 6 \n");
    printf ("\n GM_EVENT_WAN_PRIMARY_DOWN = 7 \n");
    printf ("\n GM_EVENT_WAN_SECONDARY_UP = 8 \n");
    printf ("\n GM_EVENT_WAN_SECONDARY_DOWN = 9 \n");
    printf ("\n GM_EVENT_DEV_REMOTE_ACTIVE = 10 \n");
    printf ("\n GM_EVENT_DEV_REMOTE_INACTIVE = 11 \n");
    printf ("\n GM_EVENT_DEV_SUPPORTEDMODE_CHANGED = 12 \n");
    printf ("\n GM_EVENT_ACTIVEGATEWAY_ACK_RECEIVED = 13 \n");
    printf ("\n GM_EVENT_POLICY_TABLE_UPDATED = 14 \n");
    printf ("\n GM_EVENT_FAIL_OVER_STATUS_UPDATED = 15 \n");
    printf ("\n GM_EVENT_STATIMEOUT_UPDATED = 16 \n");
    printf ("\n GM_EVENT_HEALTH_CHECK_TIMEOUT_UPDATED = 17 \n");
    printf ("\n GM_EVENT_DEV_REMOTE_SYNC_COMPLETED = 18 \n");
    printf ("\n GM_EVENT_DEV_REQ_ACTIVEGATEWAY_INACTIVE = 19 \n");
    printf ("\n GM_EVENT_DEV_BACKUPINACTIVE_WAIT_TIMEOUT_UPDATED = 20 \n");
    printf ("\n GM_EVENT_TIMEDOUT = 21 \n");
    printf ("\n GM_EVENT_HEALTH_CHECK_COMPLETED = 22 \n");
    printf ("\n GM_EVENT_DEV_PRIMARY_WAN_CHECK_STATUS_UPDATED = 23 \n");
    printf ("\n GM_EVENT_DEV_PRIMARY_WAN_IP_UPDATED = 24 \n");
    printf ("\n GM_EVENT_DEVICE_NETWORK_MODE_UPDATED = 25 \n");
    printf ("\n GM_EVENT_DUMP_LOG = 26 \n");
    printf ("\n GM_EVENT_REMDEV_CAPABILITIES_UPDATED = 27 \n");
    printf ("\n GM_EVENT_REMDEV_UPDATED = 28 \n");
    printf ("\n GM_EVENT_REMOTE_WAN_ACTIVE = 29 \n");
    printf ("\n GM_EVENT_REMOTE_WAN_INACTIVE = 30 \n");
    printf ("\n GM_EVENT_REMOTE_DEV_CLEARDB = 31 \n");
    printf ("\n GM_EVENT_REQ_MESHBACKHAUL_IFNAME_UPDATE = 32 \n");
    printf ("\n GM_EVENT_MESHBACKHAUL_IFNAME_UPDATED = 33 \n");
    printf ("\n GM_EVENT_REMOTEDEVICEDETECTION_TIMEOUT_UPDATED = 34 \n");


        return -1;
    }
#if 0
    if (argv[1])
    {    
        qname = argv[1];
    }

    if (argv[2])
    {
        qoption = atoi(argv[2]);
    }
#endif
    if (argv[1] && (atoi(argv[1]) < GM_EVENT_MAX))
    {
        eventtype = event[atoi(argv[1])];
    }
    if (qoption == MSG_SEND)
    {
        EventData event_t  = {0};
        event_t.Event = eventtype; 
        SendmsgToQ(qname,&event_t);
    }
    return 0;
}
