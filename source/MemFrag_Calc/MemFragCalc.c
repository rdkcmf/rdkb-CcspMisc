/*
* If not stated otherwise in this file or this component's LICENSE file the
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

#include<stdio.h>
#include <string.h>
#include <stdlib.h>
#include "safec_lib_common.h"

#define PART_COLUMN 8
#define COLUMN 11

/*Calculate Total Free Pages  for each column*/
unsigned long TotalFreePages(int index ,int buf[] ,unsigned long llapowerof2[])
{
	unsigned long sum = 0;
	for(;index<COLUMN ; index++)
	{
		sum += (buf[index] * llapowerof2[index] );
	}
	return sum;
}

int main(int argc ,char **argv)
{

	if(argc != 2)
		return 0;

	int buf[COLUMN] = { 0 };

	/*Separate the data of buddyinfo*/
	char* token = strtok(argv[1], ","); 
	int count = 0;
        /* CID: 151599 Misused comma operator*/
	for(count=0; count<COLUMN && token!=NULL; count++)
	{ 
		buf[count] = atoi(token);
		token = strtok(NULL, ",");
	}

	int i=0;
	int avarageFragmentation = 0;
	int overallFragmentation = 0;
	unsigned long llTotalFreePages=0;

	unsigned long llaFragValuePerPages[COLUMN] = { 0 };
	unsigned long llaTotalFragPerPages[COLUMN] = { 0 };
	unsigned long llapowerof2[COLUMN]  = {1 ,2 ,4 ,8 ,16 ,32 ,64 ,128 ,256 ,512 ,1024};

	llTotalFreePages = TotalFreePages(0 ,buf ,llapowerof2);

	/*Finding Frag Value for each column and kept into one array*/
	for(i=0 ;i<COLUMN ;i++)
		llaFragValuePerPages[i] = TotalFreePages(i ,buf ,llapowerof2);

	/*Finding Frag Value in Percentage and kept into one array for finding a avarage of frag*/
	for(i=0 ;i<COLUMN ;i++)
		llaTotalFragPerPages[i] = (	llTotalFreePages - (llaFragValuePerPages[i]) )*100 /	llTotalFreePages;

	/*Finding a Total Frag in Percentage*/
	int sum=0;	
	for(i=0 ;i<COLUMN ; i++)
		sum += (llaTotalFragPerPages[i] );

	overallFragmentation = sum / COLUMN;

        sum=0;
	for(i=3 ;i<COLUMN ; i++)
	        sum += (llaTotalFragPerPages[i] );
   
        avarageFragmentation = sum / PART_COLUMN;
	printf("%d %d\n",overallFragmentation, avarageFragmentation);

return 0;
}

