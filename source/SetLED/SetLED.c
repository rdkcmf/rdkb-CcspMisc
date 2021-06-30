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
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "platform_hal.h"


int main(int argc, char* argv[]) {
    LEDMGMT_PARAMS ledMgmt = { 0 };

	if(argc < 3) {
		printf("Invalid arguments.\n");
	}
	else {
		if(argv[1]) {
			ledMgmt.LedColor = atoi(argv[1]);

			if(ledMgmt.LedColor < 0 || ledMgmt.LedColor > 4) {
		        printf("Color Not Supported.\n");
				ledMgmt.LedColor = NOT_SUPPORTED;
			}
		}
        if(argv[2]) {
            ledMgmt.State = atoi(argv[2]);

			if(ledMgmt.State < 0 || ledMgmt.State > 1) {
				printf("Invalid State, setting LED State as Solid\n");
				ledMgmt.State = 0;
			}
        }
        if(argv[3]) {
            ledMgmt.Interval = atoi(argv[3]);

        	if(ledMgmt.Interval < 0){
            	printf("Setting Default Blink interval as 0sec\n");
            	ledMgmt.Interval = 0;
        	}   
        }

#if defined(INTEL_PUMA7) || defined(_XB6_PRODUCT_REQ_) || defined(_CBR2_PRODUCT_REQ_)
		if(RETURN_ERR == platform_hal_setLed(&ledMgmt)) {
			printf("LED Set Unsuccessful!\n");
		}
#endif
	}

	return 0;
}
