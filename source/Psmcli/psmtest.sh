##########################################################################
# If not stated otherwise in this file or this component's Licenses.txt
# file the following copyright and licenses apply:
#
# Copyright 2015 RDK Management
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
# http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
##########################################################################
#######################################################################
#   Copyright [2014] [Cisco Systems, Inc.]
# 
#   Licensed under the Apache License, Version 2.0 (the \"License\");
#   you may not use this file except in compliance with the License.
#   You may obtain a copy of the License at
# 
#       http://www.apache.org/licenses/LICENSE-2.0
# 
#   Unless required by applicable law or agreed to in writing, software
#   distributed under the License is distributed on an \"AS IS\" BASIS,
#   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
#   See the License for the specific language governing permissions and
#   limitations under the License.
#######################################################################

echo "Starting tests ..."

# Set new values
# format: ./psmcli nosubsys set <obj1 name> <obj1 value> <obj2 name> <obj2 value> ...
SET_TEST1=`./psmcli nosubsys set ccsp.landevice.1.ipaddr 192.168.100.2 ccsp.landevice.2.ipaddr 192.168.200.2 ccsp.landevice.3.ipaddr 192.168.300.2 ccsp.landevice.1.macaddr 10:78:d2:e5:aa:34`

if [ $? -eq 100 ]
then
        echo "1: set success"
fi

# Set detail
# format: ./psmcli nosubsys setdetail <obj 1 datatype> <obj1 name> <obj1 value> <obj2 datetype> <obj2 name> <obj2 value> ...
SETDETAIL_TEST2=`./psmcli nosubsys setdetail string ccsp.landevice.1.ipaddr 192.168.100.2 string ccsp.landevice.2.ipaddr 192.168.200.2 string ccsp.landevice.3.ipaddr 192.168.300.2 string ccsp.landevice.1.macaddr 10:78:d2:e5:aa:34`

if [ $? -eq 100 ]
then
        echo "2: setdetail success"
fi

# Get test
# format: ./psmcli nosubsys get <obj1 name> <obj2 name> <obj3 name> ...
GET_TEST3=`./psmcli nosubsys get ccsp.landevice.1.ipaddr`

if [ "$GET_TEST3" = "192.168.100.2" ]
then
        echo "3: get success"
else
        echo $GET_TEST3
fi

GET_TEST4=`./psmcli nosubsys get ccsp.landevice.2.ipaddr`

if [ "$GET_TEST4" =  "192.168.200.2" ]
then
        echo "4: getdetail success"
else
        echo $GET_TEST4
fi

# getdetail test
# format: ./psmcli nosubsys getdetail <obj1 name> <obj2 name> ... 
GET_TEST5=`./psmcli nosubsys getdetail ccsp.landevice.3.ipaddr`

index=0
for x in $GET_TEST5
do
        if [ $index -eq 0 ]
        then
                if [ "$x" = "string" ]
                then
                        echo "5: getdetail success"
                fi
        elif [ $index -eq 1 ]
        then
                if [ "$x" = "192.168.300.2" ]
                then
                        echo "6: getdetail success"
                fi
        fi
        index=$((index+1))
done

# get -e test
# format: ./psmcli nosubsys get -e <obj1 env var> <obj1 name> <obj2 env var> <obj2 name> ...
eval `./psmcli nosubsys get -e LANDEVICE_1_IP ccsp.landevice.1.ipaddr LANDEVICE_2_IP ccsp.landevice.2.ipaddr`

if [ "$LANDEVICE_1_IP" = "192.168.100.2" ] && [ "$LANDEVICE_2_IP" = "192.168.200.2" ]
then
        echo "7: get -e success"
else
        echo $LANDEVICE_1_IP
        echo $LANDEVICE_2_IP
fi

# getdetail -e test
# format: ./psmcli nosubsys getdetail -e <obj1 env var> <obj1 name> <obj2 env var> <obj2 name> ...
eval `./psmcli nosubsys getdetail -e LANDEVICE_1_IP ccsp.landevice.1.ipaddr LANDEVICE_2_IP ccsp.landevice.2.ipaddr`

if [ "$LANDEVICE_1_IP" = "192.168.100.2" ] && [ "$LANDEVICE_2_IP_TYPE" = "string" ]
then
        echo "8: getdetail -e success"
fi

# getallinst test
# format: ./psmcli nosubsys getallinst <obj name>
GET_ALL_INSTANCE_TEST9=`./psmcli nosubsys getallinst ccsp.landevice.`

index=0
for x in $GET_ALL_INSTANCE_TEST9
do
        if [ $index -eq 0 ]
        then
                if [ "$x" = "1" ]
                then
                        echo "9: getallinst success part 1"
                fi
        elif [ $index -eq 1 ]
        then
                if [ "$x" = "2" ]
                then
                        echo "9: getallinst success part 2"
                fi
        elif [ $index -eq 2 ]
        then
                if [ "$x" = "3" ]
                then
                        echo "9: getallinst success part 3"
                fi
        fi
        index=$((index+1))
done

# getinstcnt test
# format: ./psmcli nosubsys getinstcnt <obj1 name> <obj2 name> ...
GET_INSTANCE_CNT_TEST10=`./psmcli nosubsys getinstcnt ccsp.landevice.`
if [ "$GET_INSTANCE_CNT_TEST10" = "3" ]
then
        echo "10: getinstcnt success"
else
        echo $GET_INSTANCE_CNT_TEST10
fi

# del test
DEL_TEST11=`./psmcli nosubsys del ccsp.landevice.1.ipaddr ccsp.landevice.2.ipaddr ccsp.landevice.3.ipaddr ccsp.landevice.1.macaddr`
if [ $? -eq 100 ]
then
	echo "11: del success"
fi

echo "End of tests"

