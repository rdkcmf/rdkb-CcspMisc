/*
 * If not stated otherwise in this file or this component's Licenses.txt file the
 * following copyright and licenses apply:
 *
 * Copyright 2015 RDK Management
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


#ifndef  _BRIDGE_UTIL_H
#define  _BRIDGE_UTIL_H


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdbool.h>

#include <pthread.h> 
#include <sys/mman.h>
#include <time.h>

#include <sys/socket.h>
#include <sys/types.h>
#include<sys/ioctl.h>
#include<errno.h>
#include "linux/if.h"
#include "linux/sockios.h"

#include <ccsp_base_api.h>   // for CCSP_Message_Bus_Init/Exit
#include "ccsp_memory.h"        // for AnscAllocate/FreeMemory
#include "ccsp_psm_helper.h"
#include "sysevent/sysevent.h"
#include "syscfg/syscfg.h"

#include "bridge_util_hal.h"

//#define bridge_util_log printf
#define FAILED  -1
#define INSERT_OPERATION -2
#define MAX_GRE_STATE_WAIT_TIME 180
#define LOCAL_MOCABR_UP_FILE "/tmp/MoCABridge_up"

#define BRIDGE_UTIL_RUNNING "/tmp/.bridgeUtilRunning"
#define WAN_STATUS_EVENT "wan-status"

#define BR_SHM_MUTEX "BridgeUtilMutex"

#ifdef RDKB_EXTENDER_ENABLED
typedef enum DeviceNetworkMode
{
    DEVICE_NETWORKINGMODE_ROUTER = 0,
    DEVICE_NETWORKINGMODE_EXTENDER
}DeviceNetworkMode;
#endif

typedef struct br_shm_mutex {
  pthread_mutex_t *ptr; 
  int br_shm_create;        
  int br_shm_fd;       
  char br_mutex[32];        

} br_shm_mutex;

int getIfList(bridgeDetails *bridgeInfo);
void getCurrentIfList(char *bridge, char *current_if_list);

void removeIfaceFromBridge(bridgeDetails *bridgeInfo,char *current_if_list);
void addIfaceToBridge(bridgeDetails *bridgeInfo,char *current_if_list);
int SyncBrInterfaces();

void enableMoCaIsolationSettings (bridgeDetails *bridgeInfo);
void disableMoCaIsolationSettings (bridgeDetails *bridgeInfo);
int getXfinityEnableStatus();
int wait_for_gre_ready(char* GreIf);
int updateIfaceInfo(bridgeDetails *bridgeInfo, char* ifNameToBeUpdated, int Opr , int type);

int CreateBrInterface();
int DeleteBrInterface();
void AddOrDeletePort(char* bridge_name, char* iface_name,int operation);
void add_to_group(char* bridge_name,char* iface_name);
void remove_from_group(char* bridge_name,char* iface_name);
br_shm_mutex br_shm_mutex_init(char *name);

int br_shm_mutex_close(br_shm_mutex mutex);

int HandlePreConfigGeneric(bridgeDetails *bridgeInfo,int Config) ;
int HandlePostConfigGeneric(bridgeDetails *bridgeInfo,int Config) ;

int Initialize();
int ExitFunc();
void getSettings();

int checkIfExists(char* iface_name);
void removeIfaceFromList(char *str, const char *sub);
int bridgeUtils_main(int argc, char *argv[]);
int getMTU(int InstanceNumber);
void assignIpToBridge(char* bridgeName, char* l3netName);
void removePgdInterfacesFromCurrentIfList(char *current_if_list);
#endif
