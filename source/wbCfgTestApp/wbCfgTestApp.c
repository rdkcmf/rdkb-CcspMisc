#define SUBDOC_COUNT 3

#include "webconfig_framework.h"


#define TXID_LOWER 1000

#define TXID_UPPER 1100

#define VERSION_LOWER 2000

#define VERSION_UPPER 2100


#define ENTRIES_LOWER 1
#define ENTRIES_UPPER 100

#define MIN 0

#if 1
#include "ccsp_memory.h"        // for AnscAllocate/FreeMemory

char* component_id = "ccsp.webconfignotify";
char *pCfg = CCSP_MSG_BUS_CFG;
#endif 


int gEntries = 0;
int sleep_time = 0 ;

typedef struct teststruct {

	int val1;

	int val2 ;

	int val3 ;


}teststruct;

int gVersion = 99;
uint32_t getV(char* subdoc)
{
	if ( strcmp(subdoc,"portforwarding") == 0 )
	{
		return 100;
	}
	else if ( strcmp(subdoc,"blockeddevices") == 0 )
		return 101;
	else if ( strcmp(subdoc,"porttriggering") == 0 )
		return 102;
}

int setV(char* subdoc,uint32_t version)
{
  printf("subdoc is %s, version is %u\n",subdoc,version);
  gVersion = version;
  return 0;
}

pErr scenario_ACK(void *data)
{
	pErr lexecReturn ;
 	lexecReturn = (pErr) malloc (sizeof(Err));
	if (lexecReturn == NULL )
	{
		printf("malloc failed\n");
	}
	memset(lexecReturn,0,sizeof(lexecReturn));
	struct teststruct *data_received = (struct teststruct*) data ;

	printf("***** Processing Bob data received *******\n");
	printf("Data received is %d %d %d \n",data_received->val1,data_received->val2,data_received->val3);
	sleep(1);

	printf("***** Completed executing blob data *******\n");

	lexecReturn->ErrorCode = 200;

	if (data_received) 
	{
		free(data_received);
		data_received = NULL ;
	}

	return lexecReturn;
}

pErr scenario_NACK(void *data)
{
	pErr lexecReturn;
	lexecReturn = (pErr) malloc (sizeof(Err));
	if (lexecReturn == NULL )
	{
		printf("malloc failed\n");
	}
	memset(lexecReturn,0,sizeof(lexecReturn));


		struct teststruct *data_received = (struct teststruct*) data ;

	printf("***** Processing Bob data received ******\n");
	printf("***** Failed to execute blob data *******\n");

	lexecReturn->ErrorCode = 201;
	strncpy(lexecReturn->ErrorMsg,"failed to apply blob,SYSCFG FAILURE",sizeof(lexecReturn->ErrorMsg)-1);

	if (data_received) 
	{
		free(data_received);
		data_received = NULL ;
	}
	return lexecReturn;
}

pErr scenario_TimeOut(void *data)
{
	pErr lexecReturn;
	lexecReturn = (pErr) malloc (sizeof(Err));
	if (lexecReturn == NULL )
	{
		printf("malloc failed\n");
	}
	memset(lexecReturn,0,sizeof(lexecReturn));


	struct teststruct *data_received = (struct teststruct*) data ;

	printf("****** Processing Bob data received ********\n");
	int count = sleep_time + 5 ;
	sleep(count);
	printf("Data received is %d %d %d \n",data_received->val1,data_received->val2,data_received->val3);
	lexecReturn->ErrorCode = 200;
	if (data_received) 
	{
		free(data_received);
		data_received = NULL ;
	}

	return lexecReturn;
}

#if 1
pErr scenario_gen(void *data)
{
	pErr lexecReturn;
	lexecReturn = (pErr) malloc (sizeof(Err));
	if (lexecReturn == NULL )
	{
		printf("malloc failed\n");
	}
	memset(lexecReturn,0,sizeof(lexecReturn));

	struct teststruct *data_received = (struct teststruct*) data ;
	sleep(1);

	printf(" Processing Bob data received\n");
	printf("Data received is %d %d %d \n",data_received->val1,data_received->val2,data_received->val3);
	lexecReturn->ErrorCode = 200;
	printf(" Processing Bob data Completed\n");

	if (data_received) 
	{
		free(data_received);
		data_received = NULL ;
	}

	return lexecReturn;
}

#endif 


int rollbackFunction()
{
		struct teststruct rollback ;

		printf("rollbackFunction\n");

		rollback.val1=10;
		rollback.val2=20;
		rollback.val3=30;

		printf("FUNCTION %s : rolling back of data complete \n",__FUNCTION__);

return 0;

}
pErr scenario_MAXTIMEOUT(void *data)

{
	pErr lexecReturn;
	lexecReturn = (pErr) malloc (sizeof(Err));
	if (lexecReturn == NULL )
	{
		printf("malloc failed\n");
	}
	memset(lexecReturn,0,sizeof(lexecReturn));

	struct teststruct *data_received = (struct teststruct*) data ;
	printf("******* scenario 5 Processing Bob data received *********\n");

	int count = sleep_time * 4 ;
	sleep(count);

	printf("Data received is %d %d %d \n",data_received->val1,data_received->val2,data_received->val3);

	if (data_received) 
	{
		free(data_received);
		data_received = NULL ;
	}

	return lexecReturn;
}


size_t calculateTimeout_portmap(size_t numOfEntries)
{

	return  (DEFAULT_TIMEOUT + (numOfEntries * 1)) ;

}

void callTestFunc()
{
	char *arr[SUBDOC_COUNT+1]= { "blockeddevices","portforwarding","porttriggering",(char*) 0};
	int ver[3] = {1,2,3};
	int i =0;

	blobRegInfo *blobData;

	blobData = (blobRegInfo*) malloc(SUBDOC_COUNT * sizeof(blobRegInfo));


	memset(blobData, 0, SUBDOC_COUNT * sizeof(blobRegInfo));

	blobRegInfo *blobDataPointer = blobData;

   	for (i=0 ; i < SUBDOC_COUNT ; i++ )
	{

		strncpy(blobDataPointer->subdoc_name , arr[i], sizeof(blobDataPointer->subdoc_name)-1);

		printf("Registering subdoc %s \n",blobDataPointer->subdoc_name);

		blobDataPointer++;

	}
	blobDataPointer = blobData ;

	getVersion VersionGet = getV;

	setVersion VersionSet = setV;
	register_sub_docs(blobData,SUBDOC_COUNT,VersionGet,VersionSet);

    int count, num;

	int defCase = 0;
	srand(time(0));

	int error_case=0;
	sleep(1);

	printf("Please enter the scenario which needs to be simulated\n");

	scanf("%d", &error_case);

	int calT = 0;
	execData *execDataTest = NULL;
	teststruct *tStruct = NULL;

	switch(error_case)
	{

		case 1 : 

			printf("*********** WebcConfig Scenario 1 ***********\n");
			printf("*********** Validating ACK ******************\n");



			tStruct = (teststruct*) malloc ( sizeof(teststruct));

		    execDataTest = (execData*) malloc (sizeof(execData));

		    memset(execDataTest, 0, sizeof(execDataTest));

			printf("Please enter 1 to use subdoc/component specific , 2 to use default calculte timeout\n");
			scanf("%d", &calT); 

			tStruct->val1 = (rand() % (200 - 100 + 1)) + 100; 
			tStruct->val2 = (rand() % (300 - 200 + 1)) + 200; 
			tStruct->val3 = (rand() % (400 - 300 + 1)) + 300; 
		

	        printf("Please enter the number of entries in subdocs\n");
	        scanf("%d", &gEntries);

	        execDataTest->numOfEntries = gEntries;

	        execDataTest->txid = (rand() % (TXID_UPPER - TXID_LOWER + 1)) + TXID_LOWER; 
	        execDataTest->version = (rand() % (VERSION_UPPER - VERSION_LOWER + 1)) + VERSION_LOWER; 
	        //execDataTest->numOfEntries = (rand() % (ENTRIES_UPPER - ENTRIES_LOWER + 1)) + ENTRIES_LOWER; 

	        num = (rand() % ((SUBDOC_COUNT-1) - MIN + 1)) + MIN; 

	        strncpy(execDataTest->subdoc_name,arr[num],sizeof(execDataTest->subdoc_name)-1);

	        printf("tStruct->val1 is %d ,tStruct->val2 is  %d , tStruct->val3 %d\n ",tStruct->val1,tStruct->val2,tStruct->val3);

	        printf("execDataTest->txid is %hu ,execDataTest->version %u , execDataTest->numOfEntries %lu , execDataTest->subdoc_name %s \n ",execDataTest->txid,execDataTest->version,execDataTest->numOfEntries,execDataTest->subdoc_name);

			execDataTest->user_data = (void*) tStruct;
			execDataTest->executeBlobRequest = scenario_ACK;

			if ( calT == 1 )
			{
				  execDataTest->calcTimeout = calculateTimeout_portmap ;

			}
			else
			{
				execDataTest->calcTimeout = NULL ;
			}

		    PushBlobRequest(execDataTest);

		    if (execDataTest)
		    {
		    	free(execDataTest);
		    	execDataTest = NULL ;
		    }

	 	    break;


		case 2 : 

			printf("*********** WebcConfig Scenario 2***********\n");
			printf("**** Validating notifying subdoc versions to WebConfig Client ****\n");
			printf("Please enter the component init file\n");
			char init_file[128] = {0} ;
			scanf("%s",init_file);
		    check_component_crash(init_file);


	    break;


	    case 3 : 

			printf("*********** WebcConfig Scenario 3 ***********\n");
			printf("**********Validating timeout expiry *********\n");

		    execDataTest = (execData*) malloc (sizeof(execData));


			tStruct = (teststruct*) malloc ( sizeof(teststruct));
		    memset(execDataTest, 0, sizeof(execDataTest));
	        printf("Please enter the number of entries in subdocs\n");
	        scanf("%d", &gEntries);

			tStruct->val1 = (rand() % (200 - 100 + 1)) + 100; 
			tStruct->val2 = (rand() % (300 - 200 + 1)) + 200; 
			tStruct->val3 = (rand() % (400 - 300 + 1)) + 300; 
		
	        execDataTest->txid = (rand() % (TXID_UPPER - TXID_LOWER + 1)) + TXID_LOWER; 
	        execDataTest->version = (rand() % (VERSION_UPPER - VERSION_LOWER + 1)) + VERSION_LOWER; 
	      //  execDataTest->numOfEntries = (rand() % (ENTRIES_UPPER - ENTRIES_LOWER + 1)) + ENTRIES_LOWER; 


	        execDataTest->numOfEntries = gEntries;
	        num = (rand() % ((SUBDOC_COUNT-1) - MIN + 1)) + MIN; 
	        //printf("int num %d, \n",num);

	        strncpy(execDataTest->subdoc_name,arr[num],sizeof(execDataTest->subdoc_name)-1);

	        printf("tStruct->val1 is %d ,tStruct->val2 is  %d , tStruct->val3 %d\n ",tStruct->val1,tStruct->val2,tStruct->val3);

	        printf("execDataTest->txid is %hu ,execDataTest->version %u , execDataTest->numOfEntries %lu , execDataTest->subdoc_name %s \n ",execDataTest->txid,execDataTest->version,execDataTest->numOfEntries,execDataTest->subdoc_name);
			
			execDataTest->user_data = (void*)tStruct;
			execDataTest->calcTimeout = NULL ;
			execDataTest->executeBlobRequest = scenario_TimeOut;


		    PushBlobRequest(execDataTest);

			sleep_time = (DEFAULT_TIMEOUT + (gEntries * DEFAULT_TIMEOUT_PER_ENTRY)) ;

		    sleep(sleep_time);

		   	execDataTest->txid = (rand() % (TXID_UPPER - TXID_LOWER + 1)) + TXID_LOWER; 
			printf("New transaction id generated is %hu, version %u\n", execDataTest->txid,execDataTest->version);

		    PushBlobRequest(execDataTest);

		    sleep(sleep_time);

		    if (execDataTest)
		    {
		    	free(execDataTest);
		    	execDataTest = NULL ;
		    }


			break;

	  	    case 4 : 

			printf("*********** WebcConfig Scenario 4 ***********\n");
			printf("*********** Validating NACK *****************\n");

		    execDataTest = (execData*) malloc (sizeof(execData));
			tStruct = (teststruct*) malloc ( sizeof(teststruct));

		    memset(execDataTest, 0, sizeof(execDataTest));

			tStruct->val1 = (rand() % (200 - 100 + 1)) + 100; 
			tStruct->val2 = (rand() % (300 - 200 + 1)) + 200; 
			tStruct->val3 = (rand() % (400 - 300 + 1)) + 300; 
		
	        execDataTest->txid = (rand() % (TXID_UPPER - TXID_LOWER + 1)) + TXID_LOWER; 
	        execDataTest->version = (rand() % (VERSION_UPPER - VERSION_LOWER + 1)) + VERSION_LOWER; 
	        printf("Please enter the number of entries in subdocs\n");
	        scanf("%d", &gEntries);

	        execDataTest->numOfEntries = gEntries;

	        num = (rand() % ((SUBDOC_COUNT-1) - MIN + 1)) + MIN; 
	       // printf("int num %d, \n",num);
	        strncpy(execDataTest->subdoc_name,arr[num],sizeof(execDataTest->subdoc_name)-1);

	        printf("tStruct->val1 is %d ,tStruct->val2 is  %d , tStruct->val3 %d\n ",tStruct->val1,tStruct->val2,tStruct->val3);

	        printf("execDataTest->txid is %hu ,execDataTest->version %u , execDataTest->numOfEntries %lu , execDataTest->subdoc_name %s \n ",execDataTest->txid,execDataTest->version,execDataTest->numOfEntries,execDataTest->subdoc_name);

			execDataTest->user_data =(void*)tStruct;
			execDataTest->executeBlobRequest = scenario_NACK;
			execDataTest->calcTimeout = NULL ;

			execDataTest->rollbackFunc = rollbackFunction ;

	   		PushBlobRequest(execDataTest);


		    /*if (execDataTest)
		    {
		    	free(execDataTest);
		    	execDataTest = NULL ;
		    } */

	  	    break;


	  	  case 5 : 
			printf("*********** WebcConfig Scenario 5 ***********\n");

			printf("*********** Validating MAXTIMEOUT ***********\n");

		    execDataTest = (execData*) malloc (sizeof(execData));
			tStruct = (teststruct*) malloc ( sizeof(teststruct));

		    memset(execDataTest, 0, sizeof(execDataTest));

			tStruct->val1 = (rand() % (200 - 100 + 1)) + 100; 
			tStruct->val2 = (rand() % (300 - 200 + 1)) + 200; 
			tStruct->val3 = (rand() % (400 - 300 + 1)) + 300; 
		
	        execDataTest->txid = (rand() % (TXID_UPPER - TXID_LOWER + 1)) + TXID_LOWER; 
	        execDataTest->version = (rand() % (VERSION_UPPER - VERSION_LOWER + 1)) + VERSION_LOWER; 
	      //  execDataTest->numOfEntries = (rand() % (ENTRIES_UPPER - ENTRIES_LOWER + 1)) + ENTRIES_LOWER; 

	        printf("Please enter the number of entries in subdocs\n");
	        scanf("%d", &gEntries);

	        execDataTest->numOfEntries = gEntries;
	        num = (rand() % ((SUBDOC_COUNT-1) - MIN + 1)) + MIN; 
	       // printf("int num %d, \n",num);

	        strncpy(execDataTest->subdoc_name,arr[num],sizeof(execDataTest->subdoc_name)-1);

	        printf("tStruct->val1 is %d ,tStruct->val2 is %d , tStruct->val3 %d\n ",tStruct->val1,tStruct->val2,tStruct->val3);

	        printf("execDataTest->txid is %hu ,execDataTest->version %u , execDataTest->numOfEntries %lu , execDataTest->subdoc_name %s \n ",execDataTest->txid,execDataTest->version,execDataTest->numOfEntries,execDataTest->subdoc_name);

			execDataTest->user_data = (void*)tStruct;
			execDataTest->executeBlobRequest = scenario_MAXTIMEOUT;
			execDataTest->calcTimeout = NULL ;

	   		PushBlobRequest(execDataTest);

			sleep_time = (DEFAULT_TIMEOUT + (gEntries * DEFAULT_TIMEOUT_PER_ENTRY)) ;

		    sleep(sleep_time);

		   	execDataTest->txid = (rand() % (TXID_UPPER - TXID_LOWER + 1)) + TXID_LOWER; 
			printf("New transaction id generated is %hu, version %u\n", execDataTest->txid,execDataTest->version);

		    PushBlobRequest(execDataTest);


		    sleep(sleep_time);


		   	execDataTest->txid = (rand() % (TXID_UPPER - TXID_LOWER + 1)) + TXID_LOWER; 
			printf("New transaction id generated is %hu, version %u\n", execDataTest->txid,execDataTest->version);

		    PushBlobRequest(execDataTest);

		    sleep(sleep_time*2);

		    if (execDataTest)
		    {
		    	free(execDataTest);
		    	execDataTest = NULL ;
		    }

	    break;

		    default : 
		    printf("Executing default test case\n");
		    defCase = 1 ;
		    break;
	}



	if ( defCase == 1 )
	{
		printf("Enter the number of request to be sent\n");
	    scanf("%d", &count);
	    for (i=0; i < count ;i++)
		{

			tStruct = (teststruct*) malloc ( sizeof(teststruct));

		    execDataTest = (execData*) malloc (sizeof(execData));

		    memset(execDataTest, 0, sizeof(execDataTest));


			tStruct->val1 = (rand() % (200 - 100 + 1)) + 100; 
			tStruct->val2 = (rand() % (300 - 200 + 1)) + 200; 
			tStruct->val3 = (rand() % (400 - 300 + 1)) + 300; 
		
	        execDataTest->txid = (rand() % (TXID_UPPER - TXID_LOWER + 1)) + TXID_LOWER; 
	        execDataTest->version = (rand() % (VERSION_UPPER - VERSION_LOWER + 1)) + VERSION_LOWER; 
	        execDataTest->numOfEntries = (rand() % (ENTRIES_UPPER - ENTRIES_LOWER + 1)) + ENTRIES_LOWER; 

	       	num = (rand() % ((SUBDOC_COUNT-1) - MIN + 1)) + MIN; 

	        strncpy(execDataTest->subdoc_name,arr[num],sizeof(execDataTest->subdoc_name)-1);

	        printf("tStruct->val1 is %d ,tStruct->val2 is  %d , tStruct->val3 %d \n ",tStruct->val1,tStruct->val2,tStruct->val3);

	        printf("execDataTest->txid is %hu ,execDataTest->version %u , execDataTest->numOfEntries %lu , execDataTest->subdoc_name %s \n ",execDataTest->txid,execDataTest->version,execDataTest->numOfEntries,execDataTest->subdoc_name);

			execDataTest->user_data = (void*)tStruct; 
			execDataTest->executeBlobRequest = scenario_gen;
			execDataTest->calcTimeout = NULL ;

		    PushBlobRequest(execDataTest);

		    if (execDataTest)
		    {
		    	free(execDataTest);
		    	execDataTest = NULL ;
		    }

		}		
	}


}

int main(int argc, char *argv[])
{
	if ( argc > 1 )
	{
		if ( strcmp(argv[1],"help") == 0 )
		{
			printf("**************************************************************************************\n");
			printf("Enter 1 to simulate ACK\n");
			printf("Enter 2 to simulate notifying version to WebConfig Client\n");
			printf("Enter 3 to simulate timeout case\n");
			printf("Enter 4 to simulate NACK\n");
			printf("Enter 5 to simulate MAXTIMEOUT\n");
			printf("Enter any other number to validate default case\n");
			printf("**************************************************************************************\n");


		}
		else
		{
			printf("Pass help as an argument to get the help Info\n");
		}
		return 0;

	}

	#if 1
	int ret;
	        ret = CCSP_Message_Bus_Init(component_id, pCfg, &bus_handle,(CCSP_MESSAGE_BUS_MALLOC) Ansc_AllocateMemory_Callback, Ansc_FreeMemory_Callback);
	        if (ret == -1)
	        {
	                printf("Message bus init failed\n");
	        }
	#endif 

	callTestFunc();
 
    while(1)
    {
    	sleep(1);
    }
	return 0;

}
