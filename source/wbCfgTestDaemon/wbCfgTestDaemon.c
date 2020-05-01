#include "wbCfgTestDaemon.h"


 void *bus_handle = NULL;
char* component_id = "ccsp.webCfgTestd";

void callbk(char* info, void *user_data)
{

    printf("Received callbk from webconfigSignal , Info Received is %s \n", info);

}

void registerEvent(WebConfigEventCallback eventCB)
{
    printf("Inside %s \n", __FUNCTION__);

	int ret = 0;
    char *pCfg = CCSP_MSG_BUS_CFG;

    ret = CCSP_Message_Bus_Init(component_id, pCfg, &bus_handle,
            (CCSP_MESSAGE_BUS_MALLOC) Ansc_AllocateMemory_Callback, Ansc_FreeMemory_Callback);
    if (ret == -1) {
		printf("Message bus init failed\n");
    }


    CcspBaseIf_SetCallback2(bus_handle, "webconfigSignal",
            eventCB, NULL);

    ret = CcspBaseIf_Register_Event(bus_handle, NULL, "webconfigSignal");
    if (ret != CCSP_Message_Bus_OK) {
        printf("webconfigSignal unsuccessfull\n");
        return -1;
    } else {
        printf("Registration with CCSP Bus successful\n");
    }
    printf("%s Registration complete\n", __FUNCTION__);
    return 0;
}

void deamonize()
{

	registerEvent(callbk);
	while(1)
	{
		sleep(60);
	}
}

int main(int argc, char const *argv[])
{

	pid_t process_id = 0;
    pid_t sid = 0;
    int ret = 0;

    // Create child process
    process_id = fork();
    if (process_id < 0) {
        printf("fork failed!\n");
        return 1;
    } else if (process_id > 0) {
        return 0;
    }

    //unmask the file mode
    umask(0);

    //set new session
    sid = setsid();
    if (sid < 0) {
        printf("setsid failed!\n");
        return 1;
    }

    // Change the current working directory to root.
    chdir("/");

    deamonize();

	return 0;
}
