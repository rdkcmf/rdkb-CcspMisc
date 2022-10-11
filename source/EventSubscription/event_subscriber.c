/*
 * If not stated otherwise in this file or this component's Licenses.txt file
 * the following copyright and licenses apply:
 *
 * Copyright 2019 RDK Management
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
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <sys/time.h>
#include <time.h>
#include <unistd.h>
#include <string.h>
#include <getopt.h>
#include <rbus.h>
#include <signal.h>
#include <stdint.h>

rbusHandle_t handle;

/* CID :258957 Unbounded source buffer (STRING_SIZE) */
#define COMP_LENGTH 64
#define EVENT_LENGTH 256

static char argComponentName[COMP_LENGTH] = {0};
static char argEventName[EVENT_LENGTH] = {0};

static void eventReceiveHandler(
		rbusHandle_t handle,
		rbusEvent_t const* event,
		rbusEventSubscription_t* subscription)
{
	(void)handle;
	(void)subscription;

	const char* eventName = event->name;

	printf("User data: %s\n", (char*)subscription->userData);
	printf("event.type = %d\n", event->type);
	printf("argEventName = %s\n", argEventName);
	printf("eventName = %s\n", eventName);

	rbusValue_t value ;

	if (event->data == NULL)
	{
		printf("FAIL: event->data is NULL\n");
	}

	value = rbusObject_GetValue(event->data, NULL );
	if(!value)
	{
		printf("FAIL: value is NULL\n");
	}
	else
	{
		if(rbusValue_GetType(value) == RBUS_STRING)
		{
			char* sVal;
			char newValue[256] = {0};
			sVal=rbusValue_ToString(value, 0,0);
			strncpy(newValue,sVal,sizeof(newValue)-1);
			printf("result:SUCCESS new_value='%s'\n", newValue);
			free(sVal);
		}
		if(rbusValue_GetType(value) == RBUS_BOOLEAN)
		{
			bool newValue = rbusValue_GetBoolean(value);
			printf("New value of %s is = ", eventName);
			printf(newValue ? "true\n" : "false\n");
		}
		if(rbusValue_GetType(value) == RBUS_INT8)
		{
			int8_t newValue = rbusValue_GetInt8(value);
			printf("New value of '%s' is = %d\n", eventName, newValue);
		}
		if(rbusValue_GetType(value) == RBUS_UINT8)
		{
			uint8_t newValue = rbusValue_GetUInt8(value);
			printf("New value of '%s' is = %d\n", eventName, newValue);
		}
		if(rbusValue_GetType(value) == RBUS_INT16)
		{
			int16_t newValue = rbusValue_GetInt16(value);
			printf("New value of '%s' is = %d\n", eventName, newValue);
		}
		if(rbusValue_GetType(value) == RBUS_UINT16)
		{
			uint16_t newValue = rbusValue_GetUInt16(value);
			printf("New value of '%s' is = %d\n", eventName, newValue);
		}
		if(rbusValue_GetType(value) == RBUS_INT32)
		{
			int32_t newValue = rbusValue_GetInt32(value);
			printf("New value of '%s' is = %d\n", eventName, newValue);
		}
		if(rbusValue_GetType(value) == RBUS_UINT32)
		{
			uint32_t newValue = rbusValue_GetUInt32(value);
			printf("New value of '%s' is = %d\n", eventName, newValue);
		}
	}
	rbusValue_Release(value);
}

// Handler for SIGINT, caused by Ctrl-C at keyboard
void handle_sigint(int sig)
{
	printf("User Interrupt '%d': Unsubscribing the event '%s'\n", sig, argEventName);
	rbusEvent_Unsubscribe(handle,argEventName);
	exit(0);
}

int main(int argc, char *argv[])
{
	if (argc != 3)
	{
		printf("Missing aguments. Pass arguments in the order RBUS_COMPONENT_NAME EVENT_NAME\n");
		return -1;
	}

        snprintf(argComponentName,COMP_LENGTH,argv[1]);
        snprintf(argEventName,EVENT_LENGTH, argv[2]);

	int rc = RBUS_ERROR_SUCCESS;
	
	rc = rbus_open(&handle, argComponentName);
	if(rc != RBUS_ERROR_SUCCESS)
	{
		printf("consumer: rbus_open failed: %d\n", rc);
		return -1;
	}
	printf("Subscribing to %s\n", argEventName);
	/* subscribe to all value change events on property "Device.Provider1.Param1". Pass component name as userdata*/
	rc = rbusEvent_Subscribe(
			handle,
			argEventName,
			eventReceiveHandler,
			argComponentName,
			0);
	if(rc != RBUS_ERROR_SUCCESS)
	{
		printf("consumer: rbusEvent_Subscribe failed: %d\n", rc);
		return -1;
	}

	signal(SIGINT, handle_sigint);

	while(1)
	{
		sleep(20);
	}

	return rc;
}
