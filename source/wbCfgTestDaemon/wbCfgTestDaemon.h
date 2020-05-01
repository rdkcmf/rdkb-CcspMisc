#include "ccsp_memory.h"        // for AnscAllocate/FreeMemory
#include "ccsp_base_api.h"
#include "ccsp_message_bus.h" 
#include<stdio.h>
#include<string.h>
typedef void (*WebConfigEventCallback)(char* Info, void *user_data);

void registerEvent(WebConfigEventCallback cbevent);

