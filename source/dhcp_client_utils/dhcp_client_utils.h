/*
 * If not stated otherwise in this file or this component's LICENSE file the
 * following copyright and licenses apply:
 *
 * Copyright 2020 Sky
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <signal.h>
#include <stdbool.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include "syscfg/syscfg.h"
#include "platform_hal.h"

#define TRUE_STR               "true"
#define TRUE                   1
#define FALSE                  0
#define SUCCESS                0
#define FAILURE                1
#define BUFLEN_4               4           //!< buffer length 4
#define BUFLEN_8               8           //!< buffer length 8
#define BUFLEN_16              16          //!< buffer length 16
#define BUFLEN_18              18          //!< buffer length 18
#define BUFLEN_24              24          //!< buffer length 24
#define BUFLEN_32              32          //!< buffer length 32
#define BUFLEN_40              40          //!< buffer length 40
#define BUFLEN_48              48          //!< buffer length 48
#define BUFLEN_64              64          //!< buffer length 64
#define BUFLEN_80              80          //!< buffer length 80
#define BUFLEN_128             128         //!< buffer length 128
#define BUFLEN_256             256         //!< buffer length 256
#define BUFLEN_264             264         //!< buffer length 264
#define BUFLEN_512             512         //!< buffer length 512
#define BUFLEN_1024            1024        //!< buffer length 1024
#define CONSOLE_LOG_FILE       "/rdklogs/logs/WANMANAGERLog.txt.0"

#define DBG_PRINT(fmt ...)     {\
    FILE     *fp        = NULL;\
    fp = fopen ( CONSOLE_LOG_FILE, "a+");\
    if (fp)\
    {\
        fprintf(fp,fmt);\
        fclose(fp);\
    }\
}\

#define UNUSED_VARIABLE(x) (void)(x)
typedef struct dhcp_opt {
    char * ifname;
} dhcp_params;

pid_t start_dhcpv4_client (dhcp_params * params);
pid_t start_dhcpv6_client (dhcp_params * params);
pid_t start_exe(char * exe, char * args);
