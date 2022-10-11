/*
* If not stated otherwise in this file or this component's LICENSE file the
* following copyright and licenses apply:
*
* Copyright 2020 RDK Management
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

#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <experimental/filesystem>
#include "gtest/gtest.h"
#include <gmock/gmock.h>
#include "mocks/mock_syscfg.h"
#include "mocks/mock_file_io.h"
#include "mocks/mock_psm.h"
#include "mocks/mock_socket.h"
#include "mocks/mock_sysevent.h"
#include "mocks/mock_fd.h"
#include "mocks/mock_util.h"
#include "mocks/mock_ovs.h"
#include "mocks/mock_messagebus.h"
#include "mocks/mock_bridge_util_generic.h"

using namespace std;
using std::experimental::filesystem::exists;

extern "C"
{
#include "bridge_util.h"
#include "bridge_util_hal.h"
}

extern int ovsEnable, bridgeUtilEnable, skipWiFi, skipMoCA; //eb_enable;
extern int wan_mode;
extern int InstanceNumber;
extern int MocaIsolation_Enabled;
extern char Cmd_Opr[32];
extern char primaryBridgeName[64];

using ::testing::_;
using ::testing::Return;
using ::testing::StrEq;

SyscfgMock * g_syscfgMock = NULL;
FileIOMock * g_fileIOMock = NULL;
SocketMock * g_socketMock = NULL;
SyseventMock * g_syseventMock = NULL;
PsmMock * g_psmMock = NULL;
FileDescriptorMock * g_fdMock = NULL;
MessageBusMock * g_messagebusMock = NULL;
UtilMock * g_utilMock = NULL;
OvsMock * g_ovsMock = NULL;
BridgeUtilsGenericMock * g_bridgeUtilsGenericMock = NULL;

class BridgeUtilsTestFixture : public ::testing::Test {
    protected:
        SyscfgMock mockedSyscfg;
        FileIOMock mockedFileIO;
        SocketMock mockedSocket;
        SyseventMock mockedSysevent;
        PsmMock mockedPsm;
        FileDescriptorMock mockedFd;
        MessageBusMock mockedMsgbus;
        UtilMock  mockedUtil;
        OvsMock mockedOvs;
	BridgeUtilsGenericMock mockedGeneric;

        BridgeUtilsTestFixture()
        {
            g_syscfgMock = &mockedSyscfg;
            g_fileIOMock = &mockedFileIO;
            g_socketMock = &mockedSocket;
            g_syseventMock = &mockedSysevent;
            g_psmMock = &mockedPsm;
            g_fdMock = &mockedFd;
            g_messagebusMock = &mockedMsgbus;
            g_utilMock = &mockedUtil;
            g_ovsMock = &mockedOvs;
	    g_bridgeUtilsGenericMock = &mockedGeneric;
        }

        virtual ~BridgeUtilsTestFixture()
        {
            g_syscfgMock = NULL;
            g_fileIOMock = NULL;
            g_socketMock = NULL;
            g_syseventMock = NULL;
            g_psmMock = NULL;
            g_fdMock = NULL;
            g_messagebusMock = NULL;
            g_utilMock = NULL;
            g_ovsMock = NULL;
	    g_bridgeUtilsGenericMock = NULL;
        }
        virtual void SetUp()
        {
            bridge_util_log("%s %s %s\n", __func__,
                ::testing::UnitTest::GetInstance()->current_test_info()->test_case_name(),
                ::testing::UnitTest::GetInstance()->current_test_info()->name());
        }

        virtual void TearDown()
        {
            bridge_util_log("%s %s %s\n", __func__,
                ::testing::UnitTest::GetInstance()->current_test_info()->test_case_name(),
                ::testing::UnitTest::GetInstance()->current_test_info()->name());
        }
};

TEST (BridgeUtils, getMTU)
{
    EXPECT_EQ(1600, getMTU(MESH));
    EXPECT_EQ(1600, getMTU(MESH_BACKHAUL));
    EXPECT_EQ(0, getMTU(ETH_BACKHAUL));
    EXPECT_EQ(0, getMTU(PRIVATE_LAN));
    EXPECT_EQ(0, getMTU(HOME_SECURITY));
    EXPECT_EQ(0, getMTU(HOTSPOT_2G));
    EXPECT_EQ(0, getMTU(HOTSPOT_5G));
    EXPECT_EQ(0, getMTU(LOST_N_FOUND));
    EXPECT_EQ(0, getMTU(HOTSPOT_SECURE_2G));
    EXPECT_EQ(0, getMTU(HOTSPOT_SECURE_5G));
    EXPECT_EQ(0, getMTU(MOCA_ISOLATION));
}

TEST_F(BridgeUtilsTestFixture, Initialize)
{
    char expectedCmd[64] = {0};
    memset(expectedCmd,0,sizeof(expectedCmd));
    snprintf(expectedCmd,sizeof(expectedCmd),"touch %s",BRIDGE_UTIL_RUNNING);
    EXPECT_CALL(*g_utilMock, system(StrEq(expectedCmd)))
        .Times(1)
        .WillOnce(Return(1));
    EXPECT_CALL(*g_messagebusMock, CCSP_Message_Bus_Init(_, _, _, _, _))
          .Times(1)
          .WillOnce(Return(0));
    EXPECT_CALL(*g_syscfgMock, syscfg_init())
      	 .Times(1)
         .WillOnce(Return(0));
    EXPECT_CALL(*g_syseventMock, sysevent_open(_, _, _, _, _))
	 .Times(1)
         .WillOnce(Return(0));

    EXPECT_EQ(0, Initialize());
}

TEST_F(BridgeUtilsTestFixture, InitializeMessageBusFail)
{
    char expectedCmd[64] = {0};
    memset(expectedCmd,0,sizeof(expectedCmd));
    snprintf(expectedCmd,sizeof(expectedCmd),"touch %s",BRIDGE_UTIL_RUNNING);
    EXPECT_CALL(*g_utilMock, system(StrEq(expectedCmd)))
        .Times(1)
        .WillOnce(Return(1));
    EXPECT_CALL(*g_messagebusMock, CCSP_Message_Bus_Init(_, _, _, _, _))
	  .Times(1)
	  .WillOnce(Return(-1));

    EXPECT_EQ(FAILED, Initialize());
}

TEST_F(BridgeUtilsTestFixture, InitializeSyscfgFail1)
{
    char expectedCmd[64] = {0};
    memset(expectedCmd,0,sizeof(expectedCmd));
    snprintf(expectedCmd,sizeof(expectedCmd),"touch %s",BRIDGE_UTIL_RUNNING);
    EXPECT_CALL(*g_utilMock, system(StrEq(expectedCmd)))
        .Times(1)
        .WillOnce(Return(1));
    EXPECT_CALL(*g_messagebusMock, CCSP_Message_Bus_Init(_, _, _, _, _))
         .Times(1)
         .WillOnce(Return(0));
    EXPECT_CALL(*g_syscfgMock, syscfg_init())
	  .Times(1)
          .WillOnce(Return(-1));
    EXPECT_EQ(FAILED, Initialize());
}

TEST_F(BridgeUtilsTestFixture, InitializeSyscfgFail2)
{
    char expectedCmd[64] = {0};
    memset(expectedCmd,0,sizeof(expectedCmd));
    snprintf(expectedCmd,sizeof(expectedCmd),"touch %s",BRIDGE_UTIL_RUNNING);
    EXPECT_CALL(*g_utilMock, system(StrEq(expectedCmd)))
        .Times(1)
        .WillOnce(Return(1));
    EXPECT_CALL(*g_messagebusMock, CCSP_Message_Bus_Init(_, _, _, _, _))
         .Times(1)
         .WillOnce(Return(0));
    EXPECT_CALL(*g_syscfgMock, syscfg_init())
          .Times(1)
          .WillOnce(Return(-20));
    EXPECT_EQ(FAILED, Initialize());
}

TEST_F(BridgeUtilsTestFixture, InitializeSyseventopenFail1)
{
    char expectedCmd[64] = {0};
    memset(expectedCmd,0,sizeof(expectedCmd));
    snprintf(expectedCmd,sizeof(expectedCmd),"touch %s",BRIDGE_UTIL_RUNNING);
    EXPECT_CALL(*g_utilMock, system(StrEq(expectedCmd)))
        .Times(1)
        .WillOnce(Return(1));
    EXPECT_CALL(*g_messagebusMock, CCSP_Message_Bus_Init(_, _, _, _, _))
	  .Times(1)
          .WillOnce(Return(0));
    EXPECT_CALL(*g_syscfgMock, syscfg_init())
	  .Times(1)
          .WillOnce(Return(0));
    EXPECT_CALL(*g_syseventMock, sysevent_open(_, _, _, _, _))
         .Times(1)
         .WillOnce(Return(-1));
    EXPECT_EQ(FAILED, Initialize());
}

TEST_F(BridgeUtilsTestFixture, InitializeSyseventopenFail2)
{
    char expectedCmd[64] = {0};
    memset(expectedCmd,0,sizeof(expectedCmd));
    snprintf(expectedCmd,sizeof(expectedCmd),"touch %s",BRIDGE_UTIL_RUNNING);
    EXPECT_CALL(*g_utilMock, system(StrEq(expectedCmd)))
        .Times(1)
        .WillOnce(Return(1));
    EXPECT_CALL(*g_messagebusMock, CCSP_Message_Bus_Init(_, _, _, _, _))
          .Times(1)
          .WillOnce(Return(0));
    EXPECT_CALL(*g_syscfgMock, syscfg_init())
          .Times(1)
          .WillOnce(Return(0));
    EXPECT_CALL(*g_syseventMock, sysevent_open(_, _, _, _, _))
         .Times(1)
         .WillOnce(Return(-1241));
    EXPECT_EQ(FAILED, Initialize());
}

TEST_F(BridgeUtilsTestFixture, InitializeSyseventopenFail3)
{
    char expectedCmd[64] = {0};
    memset(expectedCmd,0,sizeof(expectedCmd));
    snprintf(expectedCmd,sizeof(expectedCmd),"touch %s",BRIDGE_UTIL_RUNNING); 
    EXPECT_CALL(*g_utilMock, system(StrEq(expectedCmd)))
        .Times(1)
        .WillOnce(Return(1));
    EXPECT_CALL(*g_messagebusMock, CCSP_Message_Bus_Init(_, _, _, _, _))
          .Times(1)
          .WillOnce(Return(0));
    EXPECT_CALL(*g_syscfgMock, syscfg_init())
          .Times(1)
          .WillOnce(Return(0));
    EXPECT_CALL(*g_syseventMock, sysevent_open(_, _, _, _, _))
         .Times(1)
         .WillOnce(Return(-1242));
    EXPECT_EQ(FAILED, Initialize());
}

TEST_F(BridgeUtilsTestFixture, InitializeSyseventopenFail4)
{
    char expectedCmd[64] = {0};
    memset(expectedCmd,0,sizeof(expectedCmd));
    snprintf(expectedCmd,sizeof(expectedCmd),"touch %s",BRIDGE_UTIL_RUNNING); 
    EXPECT_CALL(*g_utilMock, system(StrEq(expectedCmd)))
        .Times(1)
        .WillOnce(Return(1));
    EXPECT_CALL(*g_messagebusMock, CCSP_Message_Bus_Init(_, _, _, _, _))
          .Times(1)
          .WillOnce(Return(0));
    EXPECT_CALL(*g_syscfgMock, syscfg_init())
          .Times(1)
          .WillOnce(Return(0));
    EXPECT_CALL(*g_syseventMock, sysevent_open(_, _, _, _, _))
         .Times(1)
         .WillOnce(Return(-1243));
    EXPECT_EQ(FAILED, Initialize());
}

TEST_F(BridgeUtilsTestFixture, InitializeSyseventopenFail5)
{
    char expectedCmd[64] = {0};
    memset(expectedCmd,0,sizeof(expectedCmd));
    snprintf(expectedCmd,sizeof(expectedCmd),"touch %s",BRIDGE_UTIL_RUNNING); 
    EXPECT_CALL(*g_utilMock, system(StrEq(expectedCmd)))
        .Times(1)
        .WillOnce(Return(1));
    EXPECT_CALL(*g_messagebusMock, CCSP_Message_Bus_Init(_, _, _, _, _))
          .Times(1)
          .WillOnce(Return(0));
    EXPECT_CALL(*g_syscfgMock, syscfg_init())
          .Times(1)
          .WillOnce(Return(0));
    EXPECT_CALL(*g_syseventMock, sysevent_open(_, _, _, _, _))
         .Times(1)
         .WillOnce(Return(-1245));
    EXPECT_EQ(FAILED, Initialize());
}

TEST_F(BridgeUtilsTestFixture, InitializeSyseventopenFail6)
{
    char expectedCmd[64] = {0};
    memset(expectedCmd,0,sizeof(expectedCmd));
    snprintf(expectedCmd,sizeof(expectedCmd),"touch %s",BRIDGE_UTIL_RUNNING); 
    EXPECT_CALL(*g_utilMock, system(StrEq(expectedCmd)))
        .Times(1)
        .WillOnce(Return(1));
    EXPECT_CALL(*g_messagebusMock, CCSP_Message_Bus_Init(_, _, _, _, _))
          .Times(1)
          .WillOnce(Return(0));
    EXPECT_CALL(*g_syscfgMock, syscfg_init())
          .Times(1)
          .WillOnce(Return(0));
    EXPECT_CALL(*g_syseventMock, sysevent_open(_, _, _, _, _))
         .Times(1)
         .WillOnce(Return(-1248));
    EXPECT_EQ(FAILED, Initialize());
}

ACTION_TEMPLATE(SetArgNPointeeTo, HAS_1_TEMPLATE_PARAMS(unsigned, uIndex), AND_2_VALUE_PARAMS(pData, uiDataSize))
{
    memcpy(std::get<uIndex>(args), pData, uiDataSize);
}

ACTION_P(SetPsmValueArg4, value)
{
    *static_cast<char**>(arg4) = *value;
}

TEST_F(BridgeUtilsTestFixture, getXfinityEnableStatus)
{
    char paramName[] = "dmsb.hotspot.enable";
    char expectedValue[] = "1";
    EXPECT_CALL(*g_psmMock, PSM_Get_Record_Value2( _, _, StrEq(paramName), _, _))
        .Times(1)
        .WillOnce(::testing::DoAll(
            SetPsmValueArg4(&expectedValue),
            ::testing::Return(100)
        ));
    EXPECT_EQ(1, getXfinityEnableStatus());
}

TEST_F(BridgeUtilsTestFixture, getXfinityEnableStatusPsmFail)
{
    EXPECT_CALL(*g_psmMock, PSM_Get_Record_Value2( _, _, _, _, _))
        .Times(1)
        .WillOnce(Return(192));
    EXPECT_EQ(0, getXfinityEnableStatus());
}

TEST_F(BridgeUtilsTestFixture, checkIfExists)
{
    char input[] = "brlan0";
    EXPECT_CALL(*g_socketMock, socket(_, _, _))
        .Times(1)
        .WillOnce(Return(10));
    EXPECT_CALL(*g_fdMock, ioctl(_, _, _))
        .Times(1)
        .WillOnce(Return(0));
    EXPECT_CALL(*g_socketMock, close(_))
        .Times(1)
        .WillOnce(Return(0));
    EXPECT_EQ(INTERFACE_EXIST, checkIfExists(input));
}

TEST_F(BridgeUtilsTestFixture, checkIfExistsFailure)
{
    char input[] = "blan22";
    EXPECT_CALL(*g_socketMock, socket(_, _, _))
        .Times(1)
        .WillOnce(Return(-1));
    errno = ENODEV;
    EXPECT_CALL(*g_fdMock, ioctl(_, _, _))
        .Times(1)
        .WillOnce(Return(-1));
    EXPECT_CALL(*g_socketMock, close(_))
        .Times(1)
        .WillOnce(Return(0));
    EXPECT_EQ(INTERFACE_NOT_EXIST, checkIfExists(input));
}

TEST_F(BridgeUtilsTestFixture, checkIfExistsInBridgeOvsEnable)
{
    char bridge[] = "brlan0";
    char iface[] = "nmoca0";
    ovsEnable = 1;
    char expectedCmd[128] = {0};
    snprintf(expectedCmd,sizeof(expectedCmd),"ovs-vsctl list-ifaces %s | grep %s | tr \"\n\" \" \" ",bridge, iface);
    char expectedIfList[] = "nmoca0 ";
    FILE * expectedFd = (FILE *)0xffffffff;
    EXPECT_CALL(*g_fileIOMock, popen(StrEq(expectedCmd), StrEq("r")))
       .Times(1)
       .WillOnce(::testing::Return(expectedFd));
    EXPECT_CALL(*g_fileIOMock, fgets(_, _, expectedFd))
        .Times(1)
        .WillOnce(::testing::DoAll(
            SetArgNPointeeTo<0>(std::begin(expectedIfList), sizeof(expectedIfList)),
            ::testing::Return((char*)expectedIfList)
        ));
    EXPECT_CALL(*g_fileIOMock, pclose(expectedFd))
       .Times(1)
       .WillOnce(::testing::Return(0));
    EXPECT_EQ(INTERFACE_EXIST, checkIfExistsInBridge(iface, bridge));
}

TEST_F(BridgeUtilsTestFixture, checkIfExistsInBridgeOvsDisable)
{
    char bridge[] = "brlan0";
    char iface[] = "wl0";
    char expectedCmd[128] = {0};
    snprintf(expectedCmd,sizeof(expectedCmd),"brctl show %s | sed '1d' | awk '{print $NF}' | grep %s | tr \"\n\" \" \" ",bridge, iface);
    ovsEnable = 0;
    char expectedIfList[] = "wl0 wl1";
    FILE * expectedFd = (FILE *)0xffffffff;
    EXPECT_CALL(*g_fileIOMock, popen(StrEq(expectedCmd), StrEq("r")))
       .Times(1)
       .WillOnce(::testing::Return(expectedFd));
    EXPECT_CALL(*g_fileIOMock, fgets(_, _, expectedFd))
        .Times(1)
        .WillOnce(::testing::DoAll(
            SetArgNPointeeTo<0>(std::begin(expectedIfList), sizeof(expectedIfList)),
            ::testing::Return((char*)expectedIfList)
        ));
    EXPECT_CALL(*g_fileIOMock, pclose(expectedFd))
       .Times(1)
       .WillOnce(::testing::Return(0));
    EXPECT_EQ(INTERFACE_EXIST, checkIfExistsInBridge(iface, bridge));
}

TEST_F(BridgeUtilsTestFixture, checkIfExistsInBridgePopenFailure)
{
    char bridge[] = "brlan0";
    char iface[] = "wl0";
    ovsEnable = 0;
    FILE * expectedFd = (FILE *)0x00000000;
    char expectedCmd[128] = {0};
    snprintf(expectedCmd,sizeof(expectedCmd),"brctl show %s | sed '1d' | awk '{print $NF}' | grep %s | tr \"\n\" \" \" ",bridge, iface);
    EXPECT_CALL(*g_fileIOMock, popen(StrEq(expectedCmd), StrEq("r")))
       .Times(1)
       .WillOnce(::testing::Return(expectedFd));
    EXPECT_EQ(INTERFACE_NOT_EXIST, checkIfExistsInBridge(iface, bridge));
}

TEST(BridgeUtils, removeIfaceFromList)
{
    char IfList[] = "wl0 wl11 moca0 ath0 eth3";
    char expected[] = "wl0 wl11 moca0  eth3";
    char expectedCmd[4096] = {0} ;
    removeIfaceFromList(IfList, "ath0");
    EXPECT_STREQ(IfList, expected);
    removeIfaceFromList(IfList, "");
    EXPECT_STREQ(IfList, IfList);
}

TEST_F(BridgeUtilsTestFixture, enableMoCaIsolationSettingsPSMFail)
{
    bridgeDetails bridgeInfo;
    memset(&bridgeInfo, 0, sizeof(bridgeDetails));
    strncpy(bridgeInfo.bridgeName,"brlan0",sizeof(bridgeInfo.bridgeName)-1);

    char paramName1[128] = "dmsb.MultiLAN.MoCAIsoLation_l3net";
    EXPECT_CALL(*g_psmMock, PSM_Get_Record_Value2( _, _, StrEq(paramName1), _, _))
        .Times(1)
        .WillOnce(Return(9005));
    
    char paramName2[128] = {};
    memset(paramName2,0,sizeof(paramName2));
	snprintf(paramName2,sizeof(paramName2), "dmsb.l3net.%d.V4Addr",0);
    char ipaddr[64] = {};
    EXPECT_CALL(*g_psmMock, PSM_Get_Record_Value2( _, _, StrEq(paramName2), _, _))
        .Times(1)
        .WillOnce(Return(9005));
    char paramName3[128] = {};
    char expectedValue3[128] = {0};
    memset(expectedValue3,0,sizeof(expectedValue3));
    memset(paramName3,0,sizeof(paramName3));
	snprintf(paramName3,sizeof(paramName3), "dmsb.l2net.%d.Name",1);
    EXPECT_CALL(*g_psmMock, PSM_Get_Record_Value2( _, _, StrEq(paramName3), _, _))
        .Times(1)
        .WillOnce(Return(9005));

    char expectedValue4[516] = {};
    snprintf(expectedValue4,sizeof(expectedValue4),"ip link set %s allmulticast on ;\
	    	ifconfig %s %s ; \
	    	ip link set %s up ; \
	    	echo 0 > /proc/sys/net/ipv4/icmp_echo_ignore_broadcasts ; \
	    	sysctl -w net.ipv4.conf.all.arp_announce=3 ; \
	    	ip rule add from all iif %s lookup all_lans ; \
	    	echo 0 > /proc/sys/net/ipv4/conf/%s/rp_filter ;\
	    	touch %s ;",
	    	bridgeInfo.bridgeName,
	    	bridgeInfo.bridgeName,
	    	ipaddr,
	    	bridgeInfo.bridgeName,
	    	bridgeInfo.bridgeName,
	    	bridgeInfo.bridgeName,
	    	LOCAL_MOCABR_UP_FILE);

    EXPECT_CALL(*g_utilMock, system(StrEq(expectedValue4)))
        .Times(1)
        .WillOnce(Return(1));
    enableMoCaIsolationSettings(&bridgeInfo);
    EXPECT_STREQ(primaryBridgeName, expectedValue3);
}

TEST_F(BridgeUtilsTestFixture, enableMoCaIsolationSettings)
{
    bridgeDetails bridgeInfo;
    memset(&bridgeInfo, 0, sizeof(bridgeDetails));
    strncpy(bridgeInfo.bridgeName,"brlan0",sizeof(bridgeInfo.bridgeName)-1);

    char paramName1[128] = "dmsb.MultiLAN.MoCAIsoLation_l3net";
    char expectedValue1[128] = "9";
    EXPECT_CALL(*g_psmMock, PSM_Get_Record_Value2( _, _, StrEq(paramName1), _, _))
        .Times(1)
        .WillOnce(::testing::DoAll(
            SetPsmValueArg4(&expectedValue1),
            ::testing::Return(100)
        ));
    
    char paramName2[128] = {};
    memset(paramName2,0,sizeof(paramName2));
	snprintf(paramName2,sizeof(paramName2), "dmsb.l3net.%d.V4Addr",9);
    char ipaddr[] = "192.168.10.12";
    EXPECT_CALL(*g_psmMock, PSM_Get_Record_Value2( _, _, StrEq(paramName2), _, _))
        .Times(1)
        .WillOnce(::testing::DoAll(
            SetPsmValueArg4(&ipaddr),
            ::testing::Return(100)
        ));
    char paramName3[128] = {};
    char expectedValue3[] = "brlan0";
    memset(paramName3,0,sizeof(paramName3));
	snprintf(paramName3,sizeof(paramName3), "dmsb.l2net.%d.Name",1);
    EXPECT_CALL(*g_psmMock, PSM_Get_Record_Value2( _, _, StrEq(paramName3), _, _))
        .Times(1)
        .WillOnce(::testing::DoAll(
            SetPsmValueArg4(&expectedValue3),
            ::testing::Return(100)
        ));

    char expectedValue4[516] = {};
    snprintf(expectedValue4,sizeof(expectedValue4),"ip link set %s allmulticast on ;\
	    	ifconfig %s %s ; \
	    	ip link set %s up ; \
	    	echo 0 > /proc/sys/net/ipv4/icmp_echo_ignore_broadcasts ; \
	    	sysctl -w net.ipv4.conf.all.arp_announce=3 ; \
	    	ip rule add from all iif %s lookup all_lans ; \
	    	echo 0 > /proc/sys/net/ipv4/conf/%s/rp_filter ;\
	    	touch %s ;",
	    	bridgeInfo.bridgeName,
	    	bridgeInfo.bridgeName,
	    	ipaddr,
	    	bridgeInfo.bridgeName,
	    	bridgeInfo.bridgeName,
	    	bridgeInfo.bridgeName,
	    	LOCAL_MOCABR_UP_FILE);

    EXPECT_CALL(*g_utilMock, system(StrEq(expectedValue4)))
        .Times(1)
        .WillOnce(Return(1));
    enableMoCaIsolationSettings(&bridgeInfo);
    EXPECT_STREQ(primaryBridgeName, expectedValue3);
}

TEST_F(BridgeUtilsTestFixture, disableMoCaIsolationSettings)
{
    bridgeDetails bridgeInfo;
    memset(&bridgeInfo, 0, sizeof(bridgeDetails));
    strncpy(bridgeInfo.bridgeName,"brlan0",sizeof(bridgeInfo.bridgeName)-1);
    char expectedCMD[256] = {0};
	snprintf(expectedCMD,sizeof(expectedCMD),"ip link set %s down",bridgeInfo.bridgeName);
    EXPECT_CALL(*g_utilMock, system(StrEq(expectedCMD)))
        .Times(1)
        .WillOnce(Return(1));
    disableMoCaIsolationSettings(&bridgeInfo);
}

TEST_F(BridgeUtilsTestFixture, getIfList)
{
    InstanceNumber = 1;
    char paramName1[128] = {0};
    snprintf(paramName1,sizeof(paramName1), "dmsb.l2net.%d.Name", InstanceNumber);
    char expectedValue1[128] = "brlan0";
    EXPECT_CALL(*g_psmMock, PSM_Get_Record_Value2( _, _, StrEq(paramName1), _, _))
        .Times(1)
        .WillOnce(::testing::DoAll(
            SetPsmValueArg4(&expectedValue1),
            ::testing::Return(100)
        ));
    
    char paramName2[128] = {0};
    snprintf(paramName2,sizeof(paramName2), "dmsb.l2net.%d.Vid", InstanceNumber);
    char expectedValue2[128] = "100";
    EXPECT_CALL(*g_psmMock, PSM_Get_Record_Value2( _, _, StrEq(paramName2), _, _))
        .Times(1)
        .WillOnce(::testing::DoAll(
            SetPsmValueArg4(&expectedValue2),
            ::testing::Return(100)
        ));
    
    char paramName3[128] = {0};
    snprintf(paramName3,sizeof(paramName3), "dmsb.l2net.%d.Members.Link", InstanceNumber);
    char expectedValue3[128] = "link0 link1";
    EXPECT_CALL(*g_psmMock, PSM_Get_Record_Value2( _, _, StrEq(paramName3), _, _))
        .Times(1)
        .WillOnce(::testing::DoAll(
            SetPsmValueArg4(&expectedValue3),
            ::testing::Return(100)
        ));
    
    char paramName4[128] = {0};
    snprintf(paramName4,sizeof(paramName4), "dmsb.l2net.%d.Members.Eth", InstanceNumber);
    char expectedValue4[128] = "eth0 eth1";
    EXPECT_CALL(*g_psmMock, PSM_Get_Record_Value2( _, _, StrEq(paramName4), _, _))
        .Times(1)
        .WillOnce(::testing::DoAll(
            SetPsmValueArg4(&expectedValue4),
            ::testing::Return(100)
        ));

    char paramName5[128] = {0};
    snprintf(paramName5,sizeof(paramName5), "dmsb.l2net.%d.Members.Moca", InstanceNumber);
    char expectedValue5[128] = "moca0 moca1 moca2";
    EXPECT_CALL(*g_psmMock, PSM_Get_Record_Value2( _, _, StrEq(paramName5), _, _))
        .Times(1)
        .WillOnce(::testing::DoAll(
            SetPsmValueArg4(&expectedValue5),
            ::testing::Return(100)
        ));

    char paramName6[128] = {0};
    snprintf(paramName6,sizeof(paramName6), "dmsb.l2net.%d.Members.WiFi", InstanceNumber);
    char expectedValue6[128] = "wifi0";
    EXPECT_CALL(*g_psmMock, PSM_Get_Record_Value2( _, _, StrEq(paramName6), _, _))
        .Times(1)
        .WillOnce(::testing::DoAll(
            SetPsmValueArg4(&expectedValue6),
            ::testing::Return(100)
        ));
    
    char paramName7[128] = {0};
    snprintf(paramName7,sizeof(paramName7), "dmsb.l2net.%d.Members.Gre", InstanceNumber);
    char expectedValue7[128] = "";
    EXPECT_CALL(*g_psmMock, PSM_Get_Record_Value2( _, _, StrEq(paramName7), _, _))
        .Times(1)
        .WillOnce(::testing::DoAll(
            SetPsmValueArg4(&expectedValue7),
            ::testing::Return(100)
        ));

    bridgeDetails bridgeInfo;
    memset(&bridgeInfo, 0, sizeof(bridgeDetails));
    EXPECT_EQ(0, getIfList(&bridgeInfo));
    EXPECT_STREQ(bridgeInfo.bridgeName, expectedValue1);
    EXPECT_EQ(bridgeInfo.vlanID, atoi(expectedValue2));
    EXPECT_STREQ(bridgeInfo.vlan_name, expectedValue3);
    EXPECT_STREQ(bridgeInfo.ethIfList, expectedValue4);
    EXPECT_STREQ(bridgeInfo.MoCAIfList, expectedValue5);
    EXPECT_STREQ(bridgeInfo.WiFiIfList, expectedValue6);
    EXPECT_STREQ(bridgeInfo.GreIfList, expectedValue7);
}

TEST_F(BridgeUtilsTestFixture, getIfListSkipMoca)
{
    InstanceNumber = 2;
    skipMoCA = 1;
    char paramName1[128] = {0};
    snprintf(paramName1,sizeof(paramName1), "dmsb.l2net.%d.Name", InstanceNumber);
    char expectedValue1[128] = "brlan1";
    EXPECT_CALL(*g_psmMock, PSM_Get_Record_Value2( _, _, StrEq(paramName1), _, _))
        .Times(1)
        .WillOnce(::testing::DoAll(
            SetPsmValueArg4(&expectedValue1),
            ::testing::Return(100)
        ));
    
    char paramName2[128] = {0};
    snprintf(paramName2,sizeof(paramName2), "dmsb.l2net.%d.Vid", InstanceNumber);
    char expectedValue2[128] = "101";
    EXPECT_CALL(*g_psmMock, PSM_Get_Record_Value2( _, _, StrEq(paramName2), _, _))
        .Times(1)
        .WillOnce(::testing::DoAll(
            SetPsmValueArg4(&expectedValue2),
            ::testing::Return(100)
        ));
    
    char paramName3[128] = {0};
    snprintf(paramName3,sizeof(paramName3), "dmsb.l2net.%d.Members.Link", InstanceNumber);
    char expectedValue3[128] = "link0 link1";
    EXPECT_CALL(*g_psmMock, PSM_Get_Record_Value2( _, _, StrEq(paramName3), _, _))
        .Times(1)
        .WillOnce(::testing::DoAll(
            SetPsmValueArg4(&expectedValue3),
            ::testing::Return(100)
        ));
    
    char paramName4[128] = {0};
    snprintf(paramName4,sizeof(paramName4), "dmsb.l2net.%d.Members.Eth", InstanceNumber);
    char expectedValue4[128] = "eth0 eth1";
    EXPECT_CALL(*g_psmMock, PSM_Get_Record_Value2( _, _, StrEq(paramName4), _, _))
        .Times(1)
        .WillOnce(::testing::DoAll(
            SetPsmValueArg4(&expectedValue4),
            ::testing::Return(100)
        ));

    char paramName6[128] = {0};
    snprintf(paramName6,sizeof(paramName6), "dmsb.l2net.%d.Members.WiFi", InstanceNumber);
    char expectedValue6[128] = "wifi0";
    EXPECT_CALL(*g_psmMock, PSM_Get_Record_Value2( _, _, StrEq(paramName6), _, _))
        .Times(1)
        .WillOnce(::testing::DoAll(
            SetPsmValueArg4(&expectedValue6),
            ::testing::Return(100)
        ));
    
    char paramName7[128] = {0};
    snprintf(paramName7,sizeof(paramName7), "dmsb.l2net.%d.Members.Gre", InstanceNumber);
    char expectedValue7[128] = "";
    EXPECT_CALL(*g_psmMock, PSM_Get_Record_Value2( _, _, StrEq(paramName7), _, _))
        .Times(1)
        .WillOnce(::testing::DoAll(
            SetPsmValueArg4(&expectedValue7),
            ::testing::Return(100)
        ));

    bridgeDetails bridgeInfo;
    memset(&bridgeInfo, 0, sizeof(bridgeDetails));
    EXPECT_EQ(0, getIfList(&bridgeInfo));
    EXPECT_STREQ(bridgeInfo.bridgeName, expectedValue1);
    EXPECT_EQ(bridgeInfo.vlanID, atoi(expectedValue2));
    EXPECT_STREQ(bridgeInfo.vlan_name, expectedValue3);
    EXPECT_STREQ(bridgeInfo.ethIfList, expectedValue4);
    EXPECT_STREQ(bridgeInfo.MoCAIfList, "");
    EXPECT_STREQ(bridgeInfo.WiFiIfList, expectedValue6);
    EXPECT_STREQ(bridgeInfo.GreIfList, expectedValue7);
}

TEST_F(BridgeUtilsTestFixture, getIfListSkipWifi)
{
    InstanceNumber = 3;
    skipWiFi = 1;
    skipMoCA = 0;

    char paramName1[128] = {0};
    snprintf(paramName1,sizeof(paramName1), "dmsb.l2net.%d.Name", InstanceNumber);
    char expectedValue1[128] = "brlan2";
    EXPECT_CALL(*g_psmMock, PSM_Get_Record_Value2( _, _, StrEq(paramName1), _, _))
        .Times(1)
        .WillOnce(::testing::DoAll(
            SetPsmValueArg4(&expectedValue1),
            ::testing::Return(100)
        ));
    
    char paramName2[128] = {0};
    snprintf(paramName2,sizeof(paramName2), "dmsb.l2net.%d.Vid", InstanceNumber);
    char expectedValue2[128] = "100";
    EXPECT_CALL(*g_psmMock, PSM_Get_Record_Value2( _, _, StrEq(paramName2), _, _))
        .Times(1)
        .WillOnce(::testing::DoAll(
            SetPsmValueArg4(&expectedValue2),
            ::testing::Return(100)
        ));
    
    char paramName3[128] = {0};
    snprintf(paramName3,sizeof(paramName3), "dmsb.l2net.%d.Members.Link", InstanceNumber);
    char expectedValue3[128] = "link0 link1";
    EXPECT_CALL(*g_psmMock, PSM_Get_Record_Value2( _, _, StrEq(paramName3), _, _))
        .Times(1)
        .WillOnce(::testing::DoAll(
            SetPsmValueArg4(&expectedValue3),
            ::testing::Return(100)
        ));
    
    char paramName4[128] = {0};
    snprintf(paramName4,sizeof(paramName4), "dmsb.l2net.%d.Members.Eth", InstanceNumber);
    char expectedValue4[128] = "eth0 eth1";
    EXPECT_CALL(*g_psmMock, PSM_Get_Record_Value2( _, _, StrEq(paramName4), _, _))
        .Times(1)
        .WillOnce(::testing::DoAll(
            SetPsmValueArg4(&expectedValue4),
            ::testing::Return(100)
        ));

    char paramName5[128] = {0};
    snprintf(paramName5,sizeof(paramName5), "dmsb.l2net.%d.Members.Moca", InstanceNumber);
    char expectedValue5[128] = "moca0 moca1 moca2";
    EXPECT_CALL(*g_psmMock, PSM_Get_Record_Value2( _, _, StrEq(paramName5), _, _))
        .Times(1)
        .WillOnce(::testing::DoAll(
            SetPsmValueArg4(&expectedValue5),
            ::testing::Return(100)
        ));
    
    char paramName7[128] = {0};
    snprintf(paramName7,sizeof(paramName7), "dmsb.l2net.%d.Members.Gre", InstanceNumber);
    char expectedValue7[128] = "";
    EXPECT_CALL(*g_psmMock, PSM_Get_Record_Value2( _, _, StrEq(paramName7), _, _))
        .Times(1)
        .WillOnce(::testing::DoAll(
            SetPsmValueArg4(&expectedValue7),
            ::testing::Return(100)
        ));

    bridgeDetails bridgeInfo;
    memset(&bridgeInfo, 0, sizeof(bridgeDetails));
    EXPECT_EQ(0, getIfList(&bridgeInfo));
    EXPECT_STREQ(bridgeInfo.bridgeName, expectedValue1);
    EXPECT_EQ(bridgeInfo.vlanID, atoi(expectedValue2));
    EXPECT_STREQ(bridgeInfo.vlan_name, expectedValue3);
    EXPECT_STREQ(bridgeInfo.ethIfList, expectedValue4);
    EXPECT_STREQ(bridgeInfo.MoCAIfList, expectedValue5);
    EXPECT_STREQ(bridgeInfo.WiFiIfList, "");
    EXPECT_STREQ(bridgeInfo.GreIfList, expectedValue7);
}

TEST_F(BridgeUtilsTestFixture, getIfListPsmFail)
{
    InstanceNumber = 1;
    skipMoCA = 0;
    skipWiFi = 0;
    char paramName1[128] = {0};
    snprintf(paramName1,sizeof(paramName1), "dmsb.l2net.%d.Name", InstanceNumber);
    char expectedValue[128] = {0};
    EXPECT_CALL(*g_psmMock, PSM_Get_Record_Value2( _, _, StrEq(paramName1), _, _))
        .Times(1)
        .WillOnce(Return(9005));
    
    char paramName2[128] = {0};
    snprintf(paramName2,sizeof(paramName2), "dmsb.l2net.%d.Vid", InstanceNumber);
    EXPECT_CALL(*g_psmMock, PSM_Get_Record_Value2( _, _, StrEq(paramName2), _, _))
        .Times(1)
        .WillOnce(Return(9005));
    
    char paramName3[128] = {0};
    snprintf(paramName3,sizeof(paramName3), "dmsb.l2net.%d.Members.Link", InstanceNumber);
    EXPECT_CALL(*g_psmMock, PSM_Get_Record_Value2( _, _, StrEq(paramName3), _, _))
        .Times(1)
        .WillOnce(Return(9005));
    
    char paramName4[128] = {0};
    snprintf(paramName4,sizeof(paramName4), "dmsb.l2net.%d.Members.Eth", InstanceNumber);
    EXPECT_CALL(*g_psmMock, PSM_Get_Record_Value2( _, _, StrEq(paramName4), _, _))
        .Times(1)
        .WillOnce(Return(9005));

    char paramName5[128] = {0};
    snprintf(paramName5,sizeof(paramName5), "dmsb.l2net.%d.Members.Moca", InstanceNumber);
    EXPECT_CALL(*g_psmMock, PSM_Get_Record_Value2( _, _, StrEq(paramName5), _, _))
        .Times(1)
        .WillOnce(Return(9005));

    char paramName6[128] = {0};
    snprintf(paramName6,sizeof(paramName6), "dmsb.l2net.%d.Members.WiFi", InstanceNumber);
    EXPECT_CALL(*g_psmMock, PSM_Get_Record_Value2( _, _, StrEq(paramName6), _, _))
        .Times(1)
        .WillOnce(Return(9005));
    
    char paramName7[128] = {0};
    snprintf(paramName7,sizeof(paramName7), "dmsb.l2net.%d.Members.Gre", InstanceNumber);
    EXPECT_CALL(*g_psmMock, PSM_Get_Record_Value2( _, _, StrEq(paramName7), _, _))
        .Times(1)
        .WillOnce(Return(9005));

    bridgeDetails bridgeInfo;
    memset(&bridgeInfo, 0, sizeof(bridgeDetails));
    EXPECT_EQ(0, getIfList(&bridgeInfo));
    EXPECT_STREQ(bridgeInfo.bridgeName, expectedValue);
    EXPECT_EQ(bridgeInfo.vlanID, 0);
    EXPECT_STREQ(bridgeInfo.vlan_name, expectedValue);
    EXPECT_STREQ(bridgeInfo.ethIfList, expectedValue);
    EXPECT_STREQ(bridgeInfo.MoCAIfList, expectedValue);
    EXPECT_STREQ(bridgeInfo.WiFiIfList, expectedValue);
    EXPECT_STREQ(bridgeInfo.GreIfList, expectedValue);
}

TEST_F(BridgeUtilsTestFixture, wait_for_gre_ready)
{
    char greIf[] = "gre0 gre1";
    char GreState[64] = {0} , greSysName[128] = {0};
    snprintf(greSysName,sizeof(greSysName),"if_%s-status","gre0");
    strcpy(GreState, "ready");
    EXPECT_CALL(*g_syseventMock, sysevent_get(_, _, StrEq(greSysName), _, _))
	 .Times(1)
        .WillOnce(::testing::DoAll(
            SetArgNPointeeTo<3>(std::begin(GreState), sizeof(GreState)),
            ::testing::Return(0)
        ));
    
    EXPECT_EQ(0, wait_for_gre_ready(greIf));
}

TEST_F(BridgeUtilsTestFixture, wait_for_gre_ready_syseventFailed)
{
    char greIf[] = "gre0 gre1";
    char GreState[64] = {0} , greSysName[128] = {0}, GreState1[64] = {0};
    snprintf(greSysName,sizeof(greSysName),"if_%s-status","gre0");
    strcpy(GreState, "waiting");
    strcpy(GreState1, "ready");
    EXPECT_CALL(*g_syseventMock, sysevent_get(_, _, StrEq(greSysName), _, _))
	 .Times(4)
        .WillOnce(Return(-1))
        .WillOnce(Return(-1))
        .WillOnce(::testing::DoAll(
            SetArgNPointeeTo<3>(std::begin(GreState), sizeof(GreState)),
            ::testing::Return(0)
        ))
        .WillOnce(::testing::DoAll(
            SetArgNPointeeTo<3>(std::begin(GreState1), sizeof(GreState1)),
            ::testing::Return(0)
        ));
    
    EXPECT_EQ(0, wait_for_gre_ready(greIf));
}

TEST_F(BridgeUtilsTestFixture, assignIpToBridge)
{
    char bridgeName[] = "brlan10";
    char l3netName[] = "dmsb.MultiLAN.MeshBhaul_l3net";

    char paramName1[128] = {0};
    snprintf(paramName1,sizeof(paramName1), "dmsb.MultiLAN.MeshBhaul_l3net");
    char expectedValue1[128] = "9";
    EXPECT_CALL(*g_psmMock, PSM_Get_Record_Value2( _, _, StrEq(paramName1), _, _))
        .Times(1)
        .WillOnce(::testing::DoAll(
            SetPsmValueArg4(&expectedValue1),
            ::testing::Return(100)
        ));

    char paramName2[128] = {0};
    snprintf(paramName2,sizeof(paramName2), "dmsb.l3net.%d.V4Addr", 9);
    char expectedValue2[128] = "192.168.10.11";
    EXPECT_CALL(*g_psmMock, PSM_Get_Record_Value2( _, _, StrEq(paramName2), _, _))
        .Times(1)
        .WillOnce(::testing::DoAll(
            SetPsmValueArg4(&expectedValue2),
            ::testing::Return(100)
        ));
    
    char paramName3[128] = {0};
    snprintf(paramName3,sizeof(paramName3), "dmsb.l3net.%d.V4SubnetMask", 9);
    char expectedValue3[128] = "192.168.255.255";
    EXPECT_CALL(*g_psmMock, PSM_Get_Record_Value2( _, _, StrEq(paramName3), _, _))
        .Times(1)
        .WillOnce(::testing::DoAll(
            SetPsmValueArg4(&expectedValue3),
            ::testing::Return(100)
        ));
    char expectedCmd[216] = {0};
    snprintf(expectedCmd,sizeof(expectedCmd),"ifconfig %s %s netmask %s up",bridgeName,expectedValue2, expectedValue3);
    EXPECT_CALL(*g_utilMock, system(StrEq(expectedCmd)))
        .Times(1)
        .WillOnce(Return(1));
    assignIpToBridge(bridgeName, l3netName);
}

TEST_F(BridgeUtilsTestFixture, assignIpToBridgeNoSubnet)
{
    char bridgeName[] = "br403";
    char l3netName[] = "dmsb.MultiLAN.Lnf_l3net";

    char paramName1[128] = {0};
    snprintf(paramName1,sizeof(paramName1), "dmsb.MultiLAN.Lnf_l3net");
    char expectedValue1[128] = "6";
    EXPECT_CALL(*g_psmMock, PSM_Get_Record_Value2( _, _, StrEq(paramName1), _, _))
        .Times(1)
        .WillOnce(::testing::DoAll(
            SetPsmValueArg4(&expectedValue1),
            ::testing::Return(100)
        ));

    char paramName2[128] = {0};
    snprintf(paramName2,sizeof(paramName2), "dmsb.l3net.%d.V4Addr", 6);
    char expectedValue2[128] = "192.168.10.9";
    EXPECT_CALL(*g_psmMock, PSM_Get_Record_Value2( _, _, StrEq(paramName2), _, _))
        .Times(1)
        .WillOnce(::testing::DoAll(
            SetPsmValueArg4(&expectedValue2),
            ::testing::Return(100)
        ));
    
    char paramName3[128] = {0};
    snprintf(paramName3,sizeof(paramName3), "dmsb.l3net.%d.V4SubnetMask", 6);
    char expectedValue3[128] = "";
    memset(expectedValue3, 0, sizeof(expectedValue3));
    EXPECT_CALL(*g_psmMock, PSM_Get_Record_Value2( _, _, StrEq(paramName3), _, _))
        .Times(1)
        .WillOnce(::testing::DoAll(
            SetPsmValueArg4(&expectedValue3),
            ::testing::Return(100)
        ));
    char expectedCmd[216] = {0};
    snprintf(expectedCmd,sizeof(expectedCmd),"ifconfig %s %s",bridgeName,expectedValue2);
    EXPECT_CALL(*g_utilMock, system(StrEq(expectedCmd)))
        .Times(1)
        .WillOnce(Return(1));
    assignIpToBridge(bridgeName, l3netName);
}

TEST_F(BridgeUtilsTestFixture, assignIpToBridgePsmFail)
{
    char bridgeName[] = "br403";
    char l3netName[] = "dmsb.MultiLAN.Lnf_l3net";

    char paramName1[128] = {0};
    snprintf(paramName1,sizeof(paramName1), "dmsb.MultiLAN.Lnf_l3net");
    EXPECT_CALL(*g_psmMock, PSM_Get_Record_Value2( _, _, StrEq(paramName1), _, _))
        .Times(1)
        .WillOnce(Return(9005));

    assignIpToBridge(bridgeName, l3netName);
}

TEST_F(BridgeUtilsTestFixture, getCurrentIfListOvsEnable)
{
    ovsEnable = 1;
    char bridge[] = "brlan0";
    /* CID :249149 Out-of-bounds access (OVERRUN)*/
    char ifList[TOTAL_IFLIST_SIZE] = {0};
    char expectedCmd[128] = {0} ;
    snprintf(expectedCmd,sizeof(expectedCmd),"ovs-vsctl list-ifaces %s | tr \"\n\" \" \" ",bridge);
    char expectedIfList[] = "ath0 ath1 lan0 lbr0 nmoca0 ";
    FILE * expectedFd = (FILE *)0xffffffff;
    EXPECT_CALL(*g_fileIOMock, popen(StrEq(expectedCmd), StrEq("r")))
       .Times(1)
       .WillOnce(::testing::Return(expectedFd));
    EXPECT_CALL(*g_fileIOMock, fgets(_, _, expectedFd))
        .Times(1)
        .WillOnce(::testing::DoAll(
            SetArgNPointeeTo<0>(std::begin(expectedIfList), sizeof(expectedIfList)),
            ::testing::Return((char*)expectedIfList)
        ));
    EXPECT_CALL(*g_fileIOMock, pclose(expectedFd))
       .Times(1)
       .WillOnce(::testing::Return(0));
    getCurrentIfList(bridge, ifList);
    EXPECT_STREQ(expectedIfList, ifList);
}

TEST_F(BridgeUtilsTestFixture, getCurrentIfListBridgeUtilsEnable)
{
    ovsEnable = 0;
    bridgeUtilEnable = 1;
    char bridge[] = "brlan1";
    /* CID :249138 Out-of-bounds access (OVERRUN) */
    char ifList[TOTAL_IFLIST_SIZE] = {0};
    char expectedCmd[128] = {0} ;
    snprintf(expectedCmd,sizeof(expectedCmd),"brctl show %s | sed '1d' | awk '{print $NF}' | tr \"\n\" \" \" ",bridge);
    char expectedIfList[] = "wl01 wl11 gre0 ";
    FILE * expectedFd = (FILE *)0xffffffff;
    EXPECT_CALL(*g_fileIOMock, popen(StrEq(expectedCmd), StrEq("r")))
       .Times(1)
       .WillOnce(::testing::Return(expectedFd));
    EXPECT_CALL(*g_fileIOMock, fgets(_, _, expectedFd))
        .Times(1)
        .WillOnce(::testing::DoAll(
            SetArgNPointeeTo<0>(std::begin(expectedIfList), sizeof(expectedIfList)),
            ::testing::Return((char*)expectedIfList)
        ));
    EXPECT_CALL(*g_fileIOMock, pclose(expectedFd))
       .Times(1)
       .WillOnce(::testing::Return(0));
    getCurrentIfList(bridge, ifList);
    EXPECT_STREQ(expectedIfList, ifList);
}

TEST_F(BridgeUtilsTestFixture, getCurrentIfListPopenFail)
{
    ovsEnable = 0;
    bridgeUtilEnable = 1;
    char bridge[] = "brlan1";
    /* CID :249149 Out-of-bounds access (OVERRUN) */
    char ifList[TOTAL_IFLIST_SIZE] = {0};

    char expectedCmd[128] = {0} ;
    snprintf(expectedCmd,sizeof(expectedCmd),"brctl show %s | sed '1d' | awk '{print $NF}' | tr \"\n\" \" \" ",bridge);
    char expectedIfList[216] = {0};
    memset(expectedIfList, 0, sizeof(expectedIfList));
    FILE * expectedFd = (FILE *)0x00000000;
    EXPECT_CALL(*g_fileIOMock, popen(StrEq(expectedCmd), StrEq("r")))
       .Times(1)
       .WillOnce(::testing::Return(expectedFd));

    getCurrentIfList(bridge, ifList);
    EXPECT_STREQ(expectedIfList, ifList);
}

TEST_F(BridgeUtilsTestFixture, getCurrentIfListFail)
{
    ovsEnable = 0;
    bridgeUtilEnable = 0;
    char bridge[] = "brlan1";
    /* CID :249145 Out-of-bounds access (OVERRUN) */
    char ifList[TOTAL_IFLIST_SIZE] = {0};
    char expectedIfList[216] = {0};
    memset(expectedIfList, 0, sizeof(expectedIfList));
    getCurrentIfList(bridge, ifList);
    EXPECT_STREQ(expectedIfList, ifList);
}

TEST_F(BridgeUtilsTestFixture, getSettingsPsmWan)
{
    InstanceNumber = PRIVATE_LAN;
    char paramName[] = "selected_wan_mode";
    char expectedValue[] = "2";
    wan_mode = 0;
    skipMoCA = 0;
    skipWiFi = 0;
    EXPECT_CALL(*g_syscfgMock, syscfg_get( _, StrEq(paramName), _, _))
            .Times(1)
            .WillOnce(::testing::DoAll(
                SetArgNPointeeTo<2>(std::begin(expectedValue), sizeof(expectedValue)),
                ::testing::Return(0)
            ));

    char paramName1[] = "bridge_mode";
    char expectedValue1[] = "0";
    EXPECT_CALL(*g_syscfgMock, syscfg_get( _, StrEq(paramName1), _, _))
            .Times(1)
            .WillOnce(::testing::DoAll(
                SetArgNPointeeTo<2>(std::begin(expectedValue1), sizeof(expectedValue1)),
                ::testing::Return(0)
            ));
    
    char paramName2[] = "HomeSecurityEthernet4Flag";
    char expectedValue2[] = "0";
    EXPECT_CALL(*g_syscfgMock, syscfg_get( _, StrEq(paramName2), _, _))
            .Times(1)
            .WillOnce(::testing::DoAll(
                SetArgNPointeeTo<2>(std::begin(expectedValue2), sizeof(expectedValue2)),
                ::testing::Return(0)
            ));
    
    char paramName3[] = "mesh_ovs_enable";
    char expectedValue3[] = "false";
    EXPECT_CALL(*g_syscfgMock, syscfg_get( _, StrEq(paramName3), _, _))
            .Times(1)
            .WillOnce(::testing::DoAll(
                SetArgNPointeeTo<2>(std::begin(expectedValue3), sizeof(expectedValue3)),
                ::testing::Return(0)
            ));

    char paramName4[] = "bridge_util_enable";
    char expectedValue4[] = "false";
    EXPECT_CALL(*g_syscfgMock, syscfg_get( _, StrEq(paramName4), _, _))
            .Times(1)
            .WillOnce(::testing::DoAll(
                SetArgNPointeeTo<2>(std::begin(expectedValue4), sizeof(expectedValue4)),
                ::testing::Return(0)
            ));
/*    
    char paramName5[] = "eth_wan_iface_name";
    char expectedValue5[] = "";
    memset(expectedValue5, 0, sizeof(expectedValue5));
    EXPECT_CALL(*g_syscfgMock, syscfg_get( _, StrEq(paramName5), _, _))
            .Times(1)
            .WillOnce(::testing::DoAll(
                SetArgNPointeeTo<2>(std::begin(expectedValue5), sizeof(expectedValue5)),
                ::testing::Return(0)
            ));
*/    
    char paramName6[] = "eth_wan_enabled";
    char expectedValue6[] = "false";
    EXPECT_CALL(*g_syscfgMock, syscfg_get( _, StrEq(paramName6), _, _))
            .Times(1)
            .WillOnce(::testing::DoAll(
                SetArgNPointeeTo<2>(std::begin(expectedValue6), sizeof(expectedValue6)),
                ::testing::Return(0)
            ));

    char paramName7[] = "NonRootSupport";
    char expectedValue7[] = "false";
    EXPECT_CALL(*g_syscfgMock, syscfg_get( _, StrEq(paramName7), _, _))
            .Times(1)
            .WillOnce(::testing::DoAll(
                SetArgNPointeeTo<2>(std::begin(expectedValue7), sizeof(expectedValue7)),
                ::testing::Return(0)
            ));
/*
    char paramName8[] = "eb_enable";
    char expectedValue8[] = "false";
    EXPECT_CALL(*g_syscfgMock, syscfg_get( _, StrEq(paramName8), _, _))
            .Times(1)
            .WillOnce(::testing::DoAll(
                SetArgNPointeeTo<2>(std::begin(expectedValue8), sizeof(expectedValue8)),
                ::testing::Return(0)
            ));
*/
    char paramName9[128] = {0};
    snprintf(paramName9,sizeof(paramName9), "dmsb.l2net.EthWanInterface");
    char expectedValue9[128] = "eth3";
    EXPECT_CALL(*g_psmMock, PSM_Get_Record_Value2( _, _, StrEq(paramName9), _, _))
        .Times(1)
        .WillOnce(::testing::DoAll(
            SetPsmValueArg4(&expectedValue9),
            ::testing::Return(100)
        ));

    char paramName10[128] = {0};
    snprintf(paramName10,sizeof(paramName10), "dmsb.l2net.HomeNetworkIsolation");
    char expectedValue10[128] = "0";
    EXPECT_CALL(*g_psmMock, PSM_Get_Record_Value2( _, _, StrEq(paramName10), _, _))
        .Times(1)
        .WillOnce(::testing::DoAll(
            SetPsmValueArg4(&expectedValue10),
            ::testing::Return(100)
        ));
    getSettings();

    EXPECT_EQ(wan_mode, atoi(expectedValue));
    EXPECT_EQ(DeviceMode, atoi(expectedValue1));
    EXPECT_EQ(PORT2ENABLE, atoi(expectedValue2));
    EXPECT_EQ(ovsEnable, (strcmp(expectedValue3, "true") == 0)?1:0);
    EXPECT_EQ(bridgeUtilEnable, (strcmp(expectedValue4, "true") == 0)?1:0);
    EXPECT_STREQ(ethWanIfaceName, expectedValue9);
    EXPECT_EQ(ethWanEnabled, (strcmp(expectedValue6, "true") == 0)?1:0);
//    EXPECT_EQ(eb_enable, (strcmp(expectedValue8, "true") == 0)?1:0);
    EXPECT_EQ(MocaIsolation_Enabled, atoi(expectedValue10));
    EXPECT_EQ(skipWiFi, 0);
    EXPECT_EQ(skipMoCA, 0);
}

TEST_F(BridgeUtilsTestFixture, getSettingsFail)
{
    InstanceNumber = PRIVATE_LAN;
    char paramName[] = "selected_wan_mode";
    char expectedValue[] = "";
    wan_mode = 0;
    skipMoCA = 0;
    skipWiFi = 0;
    EXPECT_CALL(*g_syscfgMock, syscfg_get( _, StrEq(paramName), _, _))
            .Times(1)
            .WillOnce(Return(-1));

    char paramName1[] = "bridge_mode";
    char expectedValue1[] = "";
    EXPECT_CALL(*g_syscfgMock, syscfg_get( _, StrEq(paramName1), _, _))
            .Times(1)
            .WillOnce(Return(-1));
    
    char paramName2[] = "HomeSecurityEthernet4Flag";
    char expectedValue2[] = "";
    EXPECT_CALL(*g_syscfgMock, syscfg_get( _, StrEq(paramName2), _, _))
            .Times(1)
            .WillOnce(Return(-1));
    
    char paramName3[] = "mesh_ovs_enable";
    char expectedValue3[] = "";
    EXPECT_CALL(*g_syscfgMock, syscfg_get( _, StrEq(paramName3), _, _))
            .Times(1)
            .WillOnce(Return(-1));

    char paramName4[] = "bridge_util_enable";
    char expectedValue4[] = "";
    EXPECT_CALL(*g_syscfgMock, syscfg_get( _, StrEq(paramName4), _, _))
            .Times(1)
            .WillOnce(Return(-1));
/*    
    char paramName5[] = "eth_wan_iface_name";
    char expectedValue5[] = "";
    EXPECT_CALL(*g_syscfgMock, syscfg_get( _, StrEq(paramName5), _, _))
            .Times(1)
            .WillOnce(Return(-1));
*/    
    char paramName6[] = "eth_wan_enabled";
    char expectedValue6[] = "";
    EXPECT_CALL(*g_syscfgMock, syscfg_get( _, StrEq(paramName6), _, _))
            .Times(1)
            .WillOnce(Return(-1));

    char paramName7[] = "NonRootSupport";
    char expectedValue7[] = "";
    EXPECT_CALL(*g_syscfgMock, syscfg_get( _, StrEq(paramName7), _, _))
            .Times(1)
            .WillOnce(Return(-1));
/*
    char paramName8[] = "eb_enable";
    char expectedValue8[] = "";
    EXPECT_CALL(*g_syscfgMock, syscfg_get( _, StrEq(paramName8), _, _))
            .Times(1)
            .WillOnce(Return(-1));
*/
    char paramName9[128] = {0};
    snprintf(paramName9,sizeof(paramName9), "dmsb.l2net.EthWanInterface");
    char expectedValue9[128] = "";
    EXPECT_CALL(*g_psmMock, PSM_Get_Record_Value2( _, _, StrEq(paramName9), _, _))
        .Times(1)
        .WillOnce(Return(-1));

    char paramName10[128] = {0};
    snprintf(paramName10,sizeof(paramName10), "dmsb.l2net.HomeNetworkIsolation");
    char expectedValue10[128] = "";
    EXPECT_CALL(*g_psmMock, PSM_Get_Record_Value2( _, _, StrEq(paramName10), _, _))
        .Times(1)
        .WillOnce(Return(-1));
    getSettings();

    EXPECT_EQ(wan_mode, 0);
    EXPECT_EQ(DeviceMode, 0);
    EXPECT_EQ(PORT2ENABLE, 0);
    EXPECT_EQ(ovsEnable, 0);
    EXPECT_EQ(bridgeUtilEnable, 0);
//    EXPECT_STREQ(ethWanIfaceName, expectedValue5);
    EXPECT_EQ(ethWanEnabled, 0);
//    EXPECT_EQ(eb_enable, 0);
    EXPECT_EQ(MocaIsolation_Enabled, 0);
    EXPECT_EQ(skipWiFi, 0);
    EXPECT_EQ(skipMoCA, 0);
}

TEST_F(BridgeUtilsTestFixture, getSettings)
{
    InstanceNumber = PRIVATE_LAN;
    char paramName[] = "selected_wan_mode";
    char expectedValue[] = "1";
    EXPECT_CALL(*g_syscfgMock, syscfg_get( _, StrEq(paramName), _, _))
            .Times(1)
            .WillOnce(::testing::DoAll(
                SetArgNPointeeTo<2>(std::begin(expectedValue), sizeof(expectedValue)),
                ::testing::Return(0)
            ));

    char paramName1[] = "bridge_mode";
    char expectedValue1[] = "1";
    EXPECT_CALL(*g_syscfgMock, syscfg_get( _, StrEq(paramName1), _, _))
            .Times(1)
            .WillOnce(::testing::DoAll(
                SetArgNPointeeTo<2>(std::begin(expectedValue1), sizeof(expectedValue1)),
                ::testing::Return(0)
            ));
    
    char paramName2[] = "HomeSecurityEthernet4Flag";
    char expectedValue2[] = "1";
    EXPECT_CALL(*g_syscfgMock, syscfg_get( _, StrEq(paramName2), _, _))
            .Times(1)
            .WillOnce(::testing::DoAll(
                SetArgNPointeeTo<2>(std::begin(expectedValue2), sizeof(expectedValue2)),
                ::testing::Return(0)
            ));
    
    char paramName3[] = "mesh_ovs_enable";
    char expectedValue3[] = "true";
    EXPECT_CALL(*g_syscfgMock, syscfg_get( _, StrEq(paramName3), _, _))
            .Times(1)
            .WillOnce(::testing::DoAll(
                SetArgNPointeeTo<2>(std::begin(expectedValue3), sizeof(expectedValue3)),
                ::testing::Return(0)
            ));

    char paramName4[] = "bridge_util_enable";
    char expectedValue4[] = "true";
    EXPECT_CALL(*g_syscfgMock, syscfg_get( _, StrEq(paramName4), _, _))
            .Times(1)
            .WillOnce(::testing::DoAll(
                SetArgNPointeeTo<2>(std::begin(expectedValue4), sizeof(expectedValue4)),
                ::testing::Return(0)
            ));
    
    char paramName5[] = "eth_wan_iface_name";
    char expectedValue5[] = "eth0";
    EXPECT_CALL(*g_syscfgMock, syscfg_get( _, StrEq(paramName5), _, _))
            .Times(1)
            .WillOnce(::testing::DoAll(
                SetArgNPointeeTo<2>(std::begin(expectedValue5), sizeof(expectedValue5)),
                ::testing::Return(0)
            ));
    
    char paramName6[] = "eth_wan_enabled";
    char expectedValue6[] = "true";
    EXPECT_CALL(*g_syscfgMock, syscfg_get( _, StrEq(paramName6), _, _))
            .Times(1)
            .WillOnce(::testing::DoAll(
                SetArgNPointeeTo<2>(std::begin(expectedValue6), sizeof(expectedValue6)),
                ::testing::Return(0)
            ));

    char paramName7[] = "NonRootSupport";
    char expectedValue7[] = "false";
    EXPECT_CALL(*g_syscfgMock, syscfg_get( _, StrEq(paramName7), _, _))
            .Times(1)
            .WillOnce(::testing::DoAll(
                SetArgNPointeeTo<2>(std::begin(expectedValue7), sizeof(expectedValue7)),
                ::testing::Return(0)
            ));
/*
    char paramName8[] = "eb_enable";
    char expectedValue8[] = "true";
    EXPECT_CALL(*g_syscfgMock, syscfg_get( _, StrEq(paramName8), _, _))
            .Times(1)
            .WillOnce(::testing::DoAll(
                SetArgNPointeeTo<2>(std::begin(expectedValue8), sizeof(expectedValue8)),
                ::testing::Return(0)
            ));
*/
    char paramName10[128] = {0};
    snprintf(paramName10,sizeof(paramName10), "dmsb.l2net.HomeNetworkIsolation");
    char expectedValue10[128] = "1";
    EXPECT_CALL(*g_psmMock, PSM_Get_Record_Value2( _, _, StrEq(paramName10), _, _))
        .Times(1)
        .WillOnce(::testing::DoAll(
            SetPsmValueArg4(&expectedValue10),
            ::testing::Return(100)
        ));
    getSettings();

    EXPECT_EQ(wan_mode, atoi(expectedValue));
    EXPECT_EQ(DeviceMode, atoi(expectedValue1));
    EXPECT_EQ(PORT2ENABLE, atoi(expectedValue2));
    EXPECT_EQ(ovsEnable, (strcmp(expectedValue3, "true") == 0)?1:0);
    EXPECT_EQ(bridgeUtilEnable, (strcmp(expectedValue4, "true") == 0)?1:0);
    EXPECT_STREQ(ethWanIfaceName, expectedValue5);
    EXPECT_EQ(ethWanEnabled, (strcmp(expectedValue6, "true") == 0)?1:0);
//    EXPECT_EQ(eb_enable, (strcmp(expectedValue8, "true") == 0)?1:0);
    EXPECT_EQ(MocaIsolation_Enabled, atoi(expectedValue10));
    EXPECT_EQ(skipWiFi, 1);
    EXPECT_EQ(skipMoCA, 1);
}

ACTION_P(SetGwConfigArg1, value)
{
    *static_cast<void**>(arg1) = *value;
}

TEST_F(BridgeUtilsTestFixture, AddOrDeletePortOvsUp)
{
    char bridgeName[] = "brlan0";
    char iface[] = "ath0";
    ovsEnable = 1;
    Gateway_Config *pGwConfig = NULL;
    pGwConfig = (Gateway_Config*) malloc(sizeof(Gateway_Config));
    memset(pGwConfig, 0, sizeof(Gateway_Config));
    pGwConfig->if_type = OVS_OTHER_IF_TYPE;
    pGwConfig->mtu = DEFAULT_MTU;
    pGwConfig->vlan_id = DEFAULT_VLAN_ID;
    pGwConfig->if_cmd = OVS_IF_UP_CMD;
    EXPECT_CALL(*g_ovsMock, ovs_agent_api_get_config( _, _))
        .Times(1)
        .WillOnce(::testing::DoAll(
            SetGwConfigArg1((void **)&pGwConfig),
            ::testing::Return(true)
        ));
    EXPECT_CALL(*g_ovsMock, ovs_agent_api_interact( _, _))
        .Times(1)
        .WillOnce(Return(true));
    AddOrDeletePort(bridgeName, iface, OVS_IF_UP_CMD);
}


TEST_F(BridgeUtilsTestFixture, AddOrDeletePortOvsRemove)
{
    char bridgeName[] = "brlan0";
    char iface[] = "ath0";
    ovsEnable = 1;
    Gateway_Config *pGwConfig = NULL;
    pGwConfig = (Gateway_Config*) malloc(sizeof(Gateway_Config));
    memset(pGwConfig, 0, sizeof(Gateway_Config));
    pGwConfig->if_type = OVS_OTHER_IF_TYPE;
    pGwConfig->mtu = DEFAULT_MTU;
    pGwConfig->vlan_id = DEFAULT_VLAN_ID;
    pGwConfig->if_cmd = OVS_IF_UP_CMD;
    EXPECT_CALL(*g_ovsMock, ovs_agent_api_get_config( _, _))
        .Times(1)
        .WillOnce(::testing::DoAll(
            SetGwConfigArg1((void **)&pGwConfig),
            ::testing::Return(true)
        ));
    EXPECT_CALL(*g_ovsMock, ovs_agent_api_interact( _, _))
        .Times(1)
        .WillOnce(Return(true));
    AddOrDeletePort(bridgeName, iface, OVS_BR_REMOVE_CMD);
}


TEST_F(BridgeUtilsTestFixture, AddOrDeletePortOvsFailGetConfig)
{
    char bridgeName[] = "brlan0";
    char iface[] = "ath0";
    ovsEnable = 1;
    EXPECT_CALL(*g_ovsMock, ovs_agent_api_get_config( _, _))
        .Times(1)
        .WillOnce(Return(false));
    AddOrDeletePort(bridgeName, iface, OVS_BR_REMOVE_CMD);
}



TEST_F(BridgeUtilsTestFixture, AddOrDeletePortBridgeUtilsUp)
{
    char bridgeName[] = "brlan0";
    char iface[] = "ath0";
    ovsEnable = 0;
    bridgeUtilEnable = 1;
    char expectedCmd[1024] = {0};
    memset(expectedCmd,0,sizeof(expectedCmd));
    snprintf(expectedCmd,sizeof(expectedCmd),"check_bridge=`brctl show %s` ;\
					if [ \"$check_bridge\" = \"\" ];\
					then \
						brctl addbr %s ;\
						ifconfig %s up ; \
					fi ;",
					bridgeName,	bridgeName,	bridgeName);
    EXPECT_CALL(*g_utilMock, system(StrEq(expectedCmd)))
        .Times(1)
        .WillOnce(Return(1));
    
    char expectedCmd1[1024] = {0};
    memset(expectedCmd1,0,sizeof(expectedCmd1));
    snprintf(expectedCmd1,sizeof(expectedCmd1),"for bridge in `brctl show | cut -f1 | awk 'NF > 0' | sed '1d' | grep -v %s `;\
					do \
					check_if_attached=`brctl show $bridge | grep \"%s\" | grep -v \"%s.\"` ; \
					if [ \"$check_if_attached\" != \"\" ] ;\
						then\
					        echo \"deleting %s from $bridge\" ;\
					        brctl delif $bridge %s ; \
					 fi ;\
					 done ;\
					 check_if_exist=`brctl show %s | grep \"%s\" | grep -v \"%s.\"` ; \
					 if [ \"$check_if_exist\" = \"\" ]; \
					 then \
					 	ifconfig %s up ;\
					    	brctl addif %s %s ;\
					 fi ;",	 bridgeName, iface,	 iface,	 iface,	iface,	bridgeName, iface, iface,			 bridgeName,
					 bridgeName, iface);
    EXPECT_CALL(*g_utilMock, system(StrEq(expectedCmd1)))
        .Times(1)
        .WillOnce(Return(1));
    AddOrDeletePort(bridgeName, iface, OVS_IF_UP_CMD);
}


TEST_F(BridgeUtilsTestFixture, AddOrDeletePortBridgeUtilsDelete)
{
    char bridgeName[] = "brlan0";
    char iface[] = "ath0";
    ovsEnable = 0;
    bridgeUtilEnable = 1;
    char expectedCmd[216] = {0};
    memset(expectedCmd,0,sizeof(expectedCmd));
    snprintf(expectedCmd,sizeof(expectedCmd),"brctl delif %s %s",bridgeName,iface);
    EXPECT_CALL(*g_utilMock, system(StrEq(expectedCmd)))
        .Times(1)
        .WillOnce(Return(1));
    
    AddOrDeletePort(bridgeName, iface, OVS_BR_REMOVE_CMD);
}

TEST_F(BridgeUtilsTestFixture, AddOrDeletePortFail)
{
    /* CID :249134 Destination buffer too small (STRING_OVERFLOW) */
    char bridgeName[9] = "";
    char iface[] = "ath0";
    AddOrDeletePort(bridgeName, iface, OVS_BR_REMOVE_CMD);
    strcpy(bridgeName, "brlan123");
    memset(iface, 0, sizeof(iface));
    AddOrDeletePort(bridgeName, iface, OVS_IF_UP_CMD);
}

TEST_F(BridgeUtilsTestFixture, AddOrDeletePortOvsFail)
{
    char bridgeName[] = "brlan0";
    char iface[] = "ath0";
    ovsEnable = 1;
    Gateway_Config *pGwConfig = NULL;
    pGwConfig = (Gateway_Config*) malloc(sizeof(Gateway_Config));
    memset(pGwConfig, 0, sizeof(Gateway_Config));
    pGwConfig->if_type = OVS_OTHER_IF_TYPE;
    pGwConfig->mtu = DEFAULT_MTU;
    pGwConfig->vlan_id = DEFAULT_VLAN_ID;
    pGwConfig->if_cmd = OVS_IF_UP_CMD;
    EXPECT_CALL(*g_ovsMock, ovs_agent_api_get_config( _, _))
        .Times(2)
        .WillOnce(::testing::DoAll(
            SetGwConfigArg1((void **)&pGwConfig),
            ::testing::Return(true)))
        .WillOnce(::testing::DoAll(
            SetGwConfigArg1((void **)&pGwConfig),
            ::testing::Return(true)
        ));
    EXPECT_CALL(*g_ovsMock, ovs_agent_api_interact( _, _))
        .Times(2)
        .WillOnce(Return(false))
        .WillOnce(Return(true));
    AddOrDeletePort(bridgeName, iface, OVS_IF_UP_CMD);
}

TEST_F(BridgeUtilsTestFixture, removeIfaceFromBridge)
{
    ovsEnable = 1;
    DeviceMode = 0;
    bridgeDetails bridgeInfo;
    memset(&bridgeInfo, 0, sizeof(bridgeDetails));
    strncpy(bridgeInfo.bridgeName,"brlan1",sizeof(bridgeInfo.bridgeName)-1);
    strncpy(bridgeInfo.vlan_name,"link0",sizeof(bridgeInfo.vlan_name)-1);
    bridgeInfo.vlanID = 101;
    strncpy(bridgeInfo.ethIfList,"eth0",sizeof(bridgeInfo.ethIfList)-1);
    strncpy(bridgeInfo.MoCAIfList,"moca0",sizeof(bridgeInfo.MoCAIfList)-1);
    strncpy(bridgeInfo.GreIfList,"gre0",sizeof(bridgeInfo.GreIfList)-1);
    strncpy(bridgeInfo.WiFiIfList,"wl0",sizeof(bridgeInfo.WiFiIfList)-1);
    char ifacesList[218] = "eth0 wl0 ath0 gre0 link1";

    Gateway_Config *pGwConfig = NULL;
    pGwConfig = (Gateway_Config*) malloc(sizeof(Gateway_Config));
    memset(pGwConfig, 0, sizeof(Gateway_Config));
    pGwConfig->if_type = OVS_OTHER_IF_TYPE;
    pGwConfig->mtu = DEFAULT_MTU;
    pGwConfig->vlan_id = DEFAULT_VLAN_ID;
    pGwConfig->if_cmd = OVS_IF_UP_CMD;
    EXPECT_CALL(*g_ovsMock, ovs_agent_api_get_config( _, _))
        .Times(3)
        .WillOnce(::testing::DoAll(
            SetGwConfigArg1((void **)&pGwConfig),
            ::testing::Return(true)
        ))
	.WillOnce(::testing::DoAll(
            SetGwConfigArg1((void **)&pGwConfig),
            ::testing::Return(true)
        ))
        .WillOnce(::testing::DoAll(
            SetGwConfigArg1((void **)&pGwConfig),
            ::testing::Return(true)
        ));
    EXPECT_CALL(*g_ovsMock, ovs_agent_api_interact( _, _))
        .Times(3)
        .WillOnce(Return(true))
	.WillOnce(Return(true))
        .WillOnce(Return(true));

    removeIfaceFromBridge(&bridgeInfo, ifacesList);
    EXPECT_EQ(need_wifi_gw_refresh, 1);
    EXPECT_EQ(need_switch_gw_refresh, 1);
}

TEST_F(BridgeUtilsTestFixture, CreateBrInterface)
{
    char event[64] = {0} , value[64] = {0};
    InstanceNumber = 1;
    ovsEnable = 1;
    DeviceMode = 0;
    ethWanEnabled = 0;
    wan_mode = 1;
    skipMoCA = 0;
    skipWiFi = 0;
    snprintf(event,sizeof(event),"multinet_%d-status",InstanceNumber);
    strcpy(value, "partial");
    EXPECT_CALL(*g_syseventMock, sysevent_set(_, _, StrEq(event), StrEq(value), _))
	 .Times(1)
        .WillOnce(Return(0));
    char paramName1[128] = {0};
    snprintf(paramName1,sizeof(paramName1), "dmsb.l2net.%d.Name", InstanceNumber);
    char expectedValue1[128] = "brlan0";
    EXPECT_CALL(*g_psmMock, PSM_Get_Record_Value2( _, _, StrEq(paramName1), _, _))
        .Times(1)
        .WillOnce(::testing::DoAll(
            SetPsmValueArg4(&expectedValue1),
            ::testing::Return(100)
        ));
    
    char paramName2[128] = {0};
    snprintf(paramName2,sizeof(paramName2), "dmsb.l2net.%d.Vid", InstanceNumber);
    char expectedValue2[128] = "100";
    EXPECT_CALL(*g_psmMock, PSM_Get_Record_Value2( _, _, StrEq(paramName2), _, _))
        .Times(1)
        .WillOnce(::testing::DoAll(
            SetPsmValueArg4(&expectedValue2),
            ::testing::Return(100)
        ));
    
    char paramName3[128] = {0};
    snprintf(paramName3,sizeof(paramName3), "dmsb.l2net.%d.Members.Link", InstanceNumber);
    char expectedValue3[128] = "link0";
    EXPECT_CALL(*g_psmMock, PSM_Get_Record_Value2( _, _, StrEq(paramName3), _, _))
        .Times(1)
        .WillOnce(::testing::DoAll(
            SetPsmValueArg4(&expectedValue3),
            ::testing::Return(100)
        ));
    
    char paramName4[128] = {0};
    snprintf(paramName4,sizeof(paramName4), "dmsb.l2net.%d.Members.Eth", InstanceNumber);
    char expectedValue4[128] = "eth0 eth1";
    EXPECT_CALL(*g_psmMock, PSM_Get_Record_Value2( _, _, StrEq(paramName4), _, _))
        .Times(1)
        .WillOnce(::testing::DoAll(
            SetPsmValueArg4(&expectedValue4),
            ::testing::Return(100)
        ));

    char paramName5[128] = {0};
    snprintf(paramName5,sizeof(paramName5), "dmsb.l2net.%d.Members.Moca", InstanceNumber);
    char expectedValue5[128] = "moca0";
    EXPECT_CALL(*g_psmMock, PSM_Get_Record_Value2( _, _, StrEq(paramName5), _, _))
        .Times(1)
        .WillOnce(::testing::DoAll(
            SetPsmValueArg4(&expectedValue5),
            ::testing::Return(100)
        ));

    char paramName6[128] = {0};
    snprintf(paramName6,sizeof(paramName6), "dmsb.l2net.%d.Members.WiFi", InstanceNumber);
    char expectedValue6[128] = "wifi0";
    EXPECT_CALL(*g_psmMock, PSM_Get_Record_Value2( _, _, StrEq(paramName6), _, _))
        .Times(1)
        .WillOnce(::testing::DoAll(
            SetPsmValueArg4(&expectedValue6),
            ::testing::Return(100)
        ));
    
    char paramName7[128] = {0};
    snprintf(paramName7,sizeof(paramName7), "dmsb.l2net.%d.Members.Gre", InstanceNumber);
    char expectedValue7[128] = "gre0";
    EXPECT_CALL(*g_psmMock, PSM_Get_Record_Value2( _, _, StrEq(paramName7), _, _))
        .Times(1)
        .WillOnce(::testing::DoAll(
            SetPsmValueArg4(&expectedValue7),
            ::testing::Return(100)
        ));

    EXPECT_CALL(*g_bridgeUtilsGenericMock, HandlePreConfigVendorGeneric(_, _))
	 .Times(1)
        .WillOnce(Return(0));
    char event1[64] = {0} , value1[64] = {0};
    snprintf(event1,sizeof(event1),"multinet_%d-name",InstanceNumber);
    strcpy(value1, "brlan0");
    EXPECT_CALL(*g_syseventMock, sysevent_set(_, _, StrEq(event1), StrEq(value1), _))
	 .Times(1)
        .WillOnce(Return(0));
    char event2[64] = {0} , value2[64] = {0};
    snprintf(event2,sizeof(event2),"multinet_%d-vid",InstanceNumber);
    snprintf(value2,sizeof(value2),"%d",100);
    EXPECT_CALL(*g_syseventMock, sysevent_set(_, _, StrEq(event2), StrEq(value2), _))
	 .Times(1)
        .WillOnce(Return(0));
    Gateway_Config *pGwConfig = NULL;
    pGwConfig = (Gateway_Config*) malloc(sizeof(Gateway_Config));
    memset(pGwConfig, 0, sizeof(Gateway_Config));
    pGwConfig->if_type = OVS_OTHER_IF_TYPE;
    pGwConfig->mtu = DEFAULT_MTU;
    pGwConfig->vlan_id = DEFAULT_VLAN_ID;
    pGwConfig->if_cmd = OVS_IF_UP_CMD;
    char GreState[64] = {0} , greSysName[128] = {0};
    snprintf(greSysName,sizeof(greSysName),"if_%s-status","gre0");
    strcpy(GreState, "ready");
    EXPECT_CALL(*g_syseventMock, sysevent_get(_, _, StrEq(greSysName), _, _))
	 .Times(1)
        .WillOnce(::testing::DoAll(
            SetArgNPointeeTo<3>(std::begin(GreState), sizeof(GreState)),
            ::testing::Return(0)
        ));
    EXPECT_CALL(*g_ovsMock, ovs_agent_api_get_config( _, _))
        .Times(6)
	.WillOnce(::testing::DoAll(
            SetGwConfigArg1((void **)&pGwConfig),
            ::testing::Return(true)
        ))
        .WillOnce(::testing::DoAll(
            SetGwConfigArg1((void **)&pGwConfig),
            ::testing::Return(true)
        ))
        .WillOnce(::testing::DoAll(
            SetGwConfigArg1((void **)&pGwConfig),
            ::testing::Return(true)
        ))
        .WillOnce(::testing::DoAll(
            SetGwConfigArg1((void **)&pGwConfig),
            ::testing::Return(true)
        ))
        .WillOnce(::testing::DoAll(
            SetGwConfigArg1((void **)&pGwConfig),
            ::testing::Return(true)
        ))
        .WillOnce(::testing::DoAll(
            SetGwConfigArg1((void **)&pGwConfig),
            ::testing::Return(true)
        ));
    EXPECT_CALL(*g_ovsMock, ovs_agent_api_interact( _, _))
        .Times(6)
        .WillOnce(Return(true))
	.WillOnce(Return(true))
        .WillOnce(Return(true))
        .WillOnce(Return(true))
        .WillOnce(Return(true))
        .WillOnce(Return(true));
    EXPECT_CALL(*g_bridgeUtilsGenericMock, HandlePostConfigVendorGeneric(_, _))
	 .Times(1)
        .WillOnce(Return(0));
    char event21[64] = {0} , value21[64] = {0};
    snprintf(event21,sizeof(event21),"multinet_%d-localready",InstanceNumber);
    strcpy(value21, "1");
    EXPECT_CALL(*g_syseventMock, sysevent_set(_, _, StrEq(event21), StrEq(value21), _))
	 .Times(1)
        .WillOnce(Return(0));
    char event3[64] = {0} , value3[64] = {0};
    snprintf(event3,sizeof(event3),"multinet_%d-status",InstanceNumber);
    strcpy(value3, "ready");
    EXPECT_CALL(*g_syseventMock, sysevent_set(_, _, StrEq(event3), StrEq(value3), _))
	 .Times(1)
        .WillOnce(Return(0));
    char event4[64] = {0} ;
    snprintf(event4,sizeof(event4),"firewall-restart");
    EXPECT_CALL(*g_syseventMock, sysevent_set(_, _, StrEq(event4), _, _))
	 .Times(1)
        .WillOnce(Return(0));
    CreateBrInterface();
}

TEST_F(BridgeUtilsTestFixture, CreateBrInterfaceInst1ETHWAN)
{
    char event[64] = {0} , value[64] = {0};
    InstanceNumber = 1;
    ovsEnable = 1;
    DeviceMode = 1;
    ethWanEnabled = 1;
    wan_mode = 0;
    skipMoCA = 1;
    skipWiFi = 1;
    strncpy(ethWanIfaceName,"eth3",sizeof(ethWanIfaceName)-1);
    snprintf(event,sizeof(event),"multinet_%d-status",InstanceNumber);
    strcpy(value, "partial");
    EXPECT_CALL(*g_syseventMock, sysevent_set(_, _, StrEq(event), StrEq(value), _))
	 .Times(1)
        .WillOnce(Return(0));
    char paramName1[128] = {0};
    snprintf(paramName1,sizeof(paramName1), "dmsb.l2net.%d.Name", InstanceNumber);
    char expectedValue1[128] = "brlan0";
    EXPECT_CALL(*g_psmMock, PSM_Get_Record_Value2( _, _, StrEq(paramName1), _, _))
        .Times(1)
        .WillOnce(::testing::DoAll(
            SetPsmValueArg4(&expectedValue1),
            ::testing::Return(100)
        ));

    char paramName2[128] = {0};
    snprintf(paramName2,sizeof(paramName2), "dmsb.l2net.%d.Vid", InstanceNumber);
    char expectedValue2[128] = "100";
    EXPECT_CALL(*g_psmMock, PSM_Get_Record_Value2( _, _, StrEq(paramName2), _, _))
        .Times(1)
        .WillOnce(::testing::DoAll(
            SetPsmValueArg4(&expectedValue2),
            ::testing::Return(100)
        ));

    char paramName3[128] = {0};
    snprintf(paramName3,sizeof(paramName3), "dmsb.l2net.%d.Members.Link", InstanceNumber);
    char expectedValue3[128] = "";
    EXPECT_CALL(*g_psmMock, PSM_Get_Record_Value2( _, _, StrEq(paramName3), _, _))
        .Times(1)
        .WillOnce(::testing::DoAll(
            SetPsmValueArg4(&expectedValue3),
            ::testing::Return(100)
        ));

    char paramName4[128] = {0};
    snprintf(paramName4,sizeof(paramName4), "dmsb.l2net.%d.Members.Eth", InstanceNumber);
    char expectedValue4[128] = "eth0 eth1 eth3";
    EXPECT_CALL(*g_psmMock, PSM_Get_Record_Value2( _, _, StrEq(paramName4), _, _))
        .Times(1)
        .WillOnce(::testing::DoAll(
            SetPsmValueArg4(&expectedValue4),
            ::testing::Return(100)
        ));

    char paramName7[128] = {0};
    snprintf(paramName7,sizeof(paramName7), "dmsb.l2net.%d.Members.Gre", InstanceNumber);
    char expectedValue7[128] = "gre0";
    EXPECT_CALL(*g_psmMock, PSM_Get_Record_Value2( _, _, StrEq(paramName7), _, _))
        .Times(1)
        .WillOnce(::testing::DoAll(
            SetPsmValueArg4(&expectedValue7),
            ::testing::Return(100)
        ));
    EXPECT_CALL(*g_fileIOMock, access(StrEq("/tmp/autowan_iface_finalized"), _))
	 .Times(1)
        .WillOnce(Return(1));
    EXPECT_CALL(*g_bridgeUtilsGenericMock, HandlePreConfigVendorGeneric(_, _))
	 .Times(1)
        .WillOnce(Return(0));
    char event1[64] = {0} , value1[64] = {0};
    snprintf(event1,sizeof(event1),"multinet_%d-name",InstanceNumber);
    strcpy(value1, "brlan0");
    EXPECT_CALL(*g_syseventMock, sysevent_set(_, _, StrEq(event1), StrEq(value1), _))
	 .Times(1)
        .WillOnce(Return(0));
    char event2[64] = {0} , value2[64] = {0};
    snprintf(event2,sizeof(event2),"multinet_%d-vid",InstanceNumber);
    snprintf(value2,sizeof(value2),"%d",100);
    EXPECT_CALL(*g_syseventMock, sysevent_set(_, _, StrEq(event2), StrEq(value2), _))
	 .Times(1)
        .WillOnce(Return(0));
    Gateway_Config *pGwConfig = NULL;
    pGwConfig = (Gateway_Config*) malloc(sizeof(Gateway_Config));
    memset(pGwConfig, 0, sizeof(Gateway_Config));
    pGwConfig->if_type = OVS_OTHER_IF_TYPE;
    pGwConfig->mtu = DEFAULT_MTU;
    pGwConfig->vlan_id = DEFAULT_VLAN_ID;
    pGwConfig->if_cmd = OVS_IF_UP_CMD;

    char GreState[64] = {0} , greSysName[128] = {0};
    snprintf(greSysName,sizeof(greSysName),"if_%s-status","gre0");
    strcpy(GreState, "ready");
    EXPECT_CALL(*g_syseventMock, sysevent_get(_, _, StrEq(greSysName), _, _))
	 .Times(1)
        .WillOnce(::testing::DoAll(
            SetArgNPointeeTo<3>(std::begin(GreState), sizeof(GreState)),
            ::testing::Return(0)
        ));
    EXPECT_CALL(*g_ovsMock, ovs_agent_api_get_config( _, _))
        .Times(3)
        .WillOnce(::testing::DoAll(
            SetGwConfigArg1((void **)&pGwConfig),
            ::testing::Return(true)
        ))
	.WillOnce(::testing::DoAll(
            SetGwConfigArg1((void **)&pGwConfig),
            ::testing::Return(true)
        ))
	.WillOnce(::testing::DoAll(
            SetGwConfigArg1((void **)&pGwConfig),
            ::testing::Return(true)
        ));
    EXPECT_CALL(*g_ovsMock, ovs_agent_api_interact( _, _))
        .Times(3)
        .WillOnce(Return(true))
	.WillOnce(Return(true))
	.WillOnce(Return(true));

    EXPECT_CALL(*g_bridgeUtilsGenericMock, HandlePostConfigVendorGeneric(_, _))
	 .Times(1)
        .WillOnce(Return(0));
    char event21[64] = {0} , value21[64] = {0};
    snprintf(event21,sizeof(event21),"multinet_%d-localready",InstanceNumber);
    strcpy(value21, "1");
    EXPECT_CALL(*g_syseventMock, sysevent_set(_, _, StrEq(event21), StrEq(value21), _))
	 .Times(1)
        .WillOnce(Return(0));
    char event3[64] = {0} , value3[64] = {0};
    snprintf(event3,sizeof(event3),"multinet_%d-status",InstanceNumber);
    strcpy(value3, "ready");
    EXPECT_CALL(*g_syseventMock, sysevent_set(_, _, StrEq(event3), StrEq(value3), _))
	 .Times(1)
        .WillOnce(Return(0));
    char event4[64] = {0} ;
    snprintf(event4,sizeof(event4),"firewall-restart");
    EXPECT_CALL(*g_syseventMock, sysevent_set(_, _, StrEq(event4), _, _))
	 .Times(1)
        .WillOnce(Return(0));
    CreateBrInterface();
}

TEST_F(BridgeUtilsTestFixture, CreateBrInterfaceLnF)
{
    char event[64] = {0} , value[64] = {0};
    InstanceNumber = 6;
    ovsEnable = 1;
    DeviceMode = 1;
    ethWanEnabled = 1;
    wan_mode = 0;
    skipMoCA = 1;
    skipWiFi = 0;
    BridgeOprInPropgress = 1;
    memset(event, 0, sizeof(event));
    snprintf(event,sizeof(event),"multinet_%d-status",InstanceNumber);
    strcpy(value, "partial");
    EXPECT_CALL(*g_syseventMock, sysevent_set(_, _, StrEq(event), StrEq(value), _))
	 .Times(1)
        .WillOnce(Return(0));
    char paramName1[128] = {0};
    snprintf(paramName1,sizeof(paramName1), "dmsb.l2net.%d.Name", InstanceNumber);
    char expectedValue1[128] = "br106";
    EXPECT_CALL(*g_psmMock, PSM_Get_Record_Value2( _, _, StrEq(paramName1), _, _))
        .Times(1)
        .WillOnce(::testing::DoAll(
            SetPsmValueArg4(&expectedValue1),
            ::testing::Return(100)
        ));

    char paramName2[128] = {0};
    snprintf(paramName2,sizeof(paramName2), "dmsb.l2net.%d.Vid", InstanceNumber);
    char expectedValue2[128] = "106";
    EXPECT_CALL(*g_psmMock, PSM_Get_Record_Value2( _, _, StrEq(paramName2), _, _))
        .Times(1)
        .WillOnce(::testing::DoAll(
            SetPsmValueArg4(&expectedValue2),
            ::testing::Return(100)
        ));

    char paramName3[128] = {0};
    snprintf(paramName3,sizeof(paramName3), "dmsb.l2net.%d.Members.Link", InstanceNumber);
    char expectedValue3[128] = "";
    EXPECT_CALL(*g_psmMock, PSM_Get_Record_Value2( _, _, StrEq(paramName3), _, _))
        .Times(1)
        .WillOnce(::testing::DoAll(
            SetPsmValueArg4(&expectedValue3),
            ::testing::Return(100)
        ));

    char paramName4[128] = {0};
    snprintf(paramName4,sizeof(paramName4), "dmsb.l2net.%d.Members.Eth", InstanceNumber);
    char expectedValue4[128] = "";
    EXPECT_CALL(*g_psmMock, PSM_Get_Record_Value2( _, _, StrEq(paramName4), _, _))
        .Times(1)
        .WillOnce(::testing::DoAll(
            SetPsmValueArg4(&expectedValue4),
            ::testing::Return(100)
        ));

    char paramName6[128] = {0};
    snprintf(paramName6,sizeof(paramName6), "dmsb.l2net.%d.Members.WiFi", InstanceNumber);
    char expectedValue6[128] = "wifi0 wifi1 wifi2 wifi3";
    EXPECT_CALL(*g_psmMock, PSM_Get_Record_Value2( _, _, StrEq(paramName6), _, _))
        .Times(1)
        .WillOnce(::testing::DoAll(
            SetPsmValueArg4(&expectedValue6),
            ::testing::Return(100)
        ));

    char paramName7[128] = {0};
    snprintf(paramName7,sizeof(paramName7), "dmsb.l2net.%d.Members.Gre", InstanceNumber);
    char expectedValue7[128] = "";
    EXPECT_CALL(*g_psmMock, PSM_Get_Record_Value2( _, _, StrEq(paramName7), _, _))
        .Times(1)
        .WillOnce(::testing::DoAll(
            SetPsmValueArg4(&expectedValue7),
            ::testing::Return(100)
        ));
    EXPECT_CALL(*g_bridgeUtilsGenericMock, HandlePreConfigVendorGeneric(_, _))
	 .Times(1)
        .WillOnce(Return(0));
    char event1[64] = {0} , value1[64] = {0};
    snprintf(event1,sizeof(event1),"multinet_%d-name",InstanceNumber);
    strcpy(value1, "br106");
    EXPECT_CALL(*g_syseventMock, sysevent_set(_, _, StrEq(event1), StrEq(value1), _))
	 .Times(1)
        .WillOnce(Return(0));
    char event2[64] = {0} , value2[64] = {0};
    snprintf(event2,sizeof(event2),"multinet_%d-vid",InstanceNumber);
    snprintf(value2,sizeof(value2),"%d",106);
    EXPECT_CALL(*g_syseventMock, sysevent_set(_, _, StrEq(event2), StrEq(value2), _))
	 .Times(1)
        .WillOnce(Return(0));
    Gateway_Config *pGwConfig = NULL;
    pGwConfig = (Gateway_Config*) malloc(sizeof(Gateway_Config));
    memset(pGwConfig, 0, sizeof(Gateway_Config));
    pGwConfig->if_type = OVS_OTHER_IF_TYPE;
    pGwConfig->mtu = DEFAULT_MTU;
    pGwConfig->vlan_id = DEFAULT_VLAN_ID;
    pGwConfig->if_cmd = OVS_IF_UP_CMD;

    EXPECT_CALL(*g_ovsMock, ovs_agent_api_get_config( _, _))
        .Times(4)
        .WillOnce(::testing::DoAll(
            SetGwConfigArg1((void **)&pGwConfig),
            ::testing::Return(true)
        ))
	.WillOnce(::testing::DoAll(
            SetGwConfigArg1((void **)&pGwConfig),
            ::testing::Return(true)
        ))
	.WillOnce(::testing::DoAll(
            SetGwConfigArg1((void **)&pGwConfig),
            ::testing::Return(true)
        ))
    .WillOnce(::testing::DoAll(
            SetGwConfigArg1((void **)&pGwConfig),
            ::testing::Return(true)
        ));
    EXPECT_CALL(*g_ovsMock, ovs_agent_api_interact( _, _))
        .Times(4)
        .WillOnce(Return(true))
        .WillOnce(Return(true))
        .WillOnce(Return(true))
        .WillOnce(Return(true));

    char expectedValue111[128] = "8";
    EXPECT_CALL(*g_psmMock, PSM_Get_Record_Value2( _, _, StrEq("dmsb.MultiLAN.LnF_l3net"), _, _))
        .Times(1)
        .WillOnce(::testing::DoAll(
            SetPsmValueArg4(&expectedValue111),
            ::testing::Return(100)
        ));

    char paramName21[128] = {0};
    snprintf(paramName21,sizeof(paramName2), "dmsb.l3net.%d.V4Addr", 8);
    char expectedValue21[128] = "192.168.10.11";
    EXPECT_CALL(*g_psmMock, PSM_Get_Record_Value2( _, _, StrEq(paramName21), _, _))
        .Times(1)
        .WillOnce(::testing::DoAll(
            SetPsmValueArg4(&expectedValue21),
            ::testing::Return(100)
        ));

    char paramName31[128] = {0};
    snprintf(paramName31,sizeof(paramName31), "dmsb.l3net.%d.V4SubnetMask", 8);
    char expectedValue31[128] = "";
    EXPECT_CALL(*g_psmMock, PSM_Get_Record_Value2( _, _, StrEq(paramName31), _, _))
        .Times(1)
        .WillOnce(::testing::DoAll(
            SetPsmValueArg4(&expectedValue31),
            ::testing::Return(100)
        ));

    char expectedCmd123[216] = {0};
    snprintf(expectedCmd123,sizeof(expectedCmd123),"ifconfig %s %s",expectedValue1,expectedValue21);
    EXPECT_CALL(*g_utilMock, system(StrEq(expectedCmd123)))
        .Times(1)
        .WillOnce(Return(1));

    EXPECT_CALL(*g_bridgeUtilsGenericMock, HandlePostConfigVendorGeneric(_, _))
	 .Times(1)
        .WillOnce(Return(0));
    char event21[64] = {0} , value21[64] = {0};
    snprintf(event21,sizeof(event21),"multinet_%d-localready",InstanceNumber);
    strcpy(value21, "1");
    EXPECT_CALL(*g_syseventMock, sysevent_set(_, _, StrEq(event21), StrEq(value21), _))
	 .Times(1)
        .WillOnce(Return(0));
    char event3[64] = {0} , value3[64] = {0};
    snprintf(event3,sizeof(event3),"multinet_%d-status",InstanceNumber);
    strcpy(value3, "ready");
    EXPECT_CALL(*g_syseventMock, sysevent_set(_, _, StrEq(event3), StrEq(value3), _))
	 .Times(1)
        .WillOnce(Return(0));
    char event4[64] = {0} ;
    snprintf(event4,sizeof(event4),"firewall-restart");
    EXPECT_CALL(*g_syseventMock, sysevent_set(_, _, StrEq(event4), _, _))
	 .Times(1)
        .WillOnce(Return(0));
    CreateBrInterface();
}

TEST_F(BridgeUtilsTestFixture, CreateBrInterfaceMocaIsolation)
{
    char event[64] = {0} , value[64] = {0};
    InstanceNumber = MOCA_ISOLATION;
    ovsEnable = 1;
    DeviceMode = 0;
    ethWanEnabled = 0;
    wan_mode = 0;
    skipMoCA = 0;
    skipWiFi = 0;
    BridgeOprInPropgress = 1;
    MocaIsolation_Enabled = 1;
    snprintf(event,sizeof(event),"multinet_%d-status",InstanceNumber);
    strcpy(value, "partial");
    EXPECT_CALL(*g_syseventMock, sysevent_set(_, _, StrEq(event), StrEq(value), _))
	 .Times(1)
        .WillOnce(Return(0));
    char paramName1[128] = {0};
    snprintf(paramName1,sizeof(paramName1), "dmsb.l2net.%d.Name", InstanceNumber);
    char expectedValue1[128] = "brlan10";
    EXPECT_CALL(*g_psmMock, PSM_Get_Record_Value2( _, _, StrEq(paramName1), _, _))
        .Times(1)
        .WillOnce(::testing::DoAll(
            SetPsmValueArg4(&expectedValue1),
            ::testing::Return(100)
        ));

    char paramName2[128] = {0};
    snprintf(paramName2,sizeof(paramName2), "dmsb.l2net.%d.Vid", InstanceNumber);
    char expectedValue2[128] = "111";
    EXPECT_CALL(*g_psmMock, PSM_Get_Record_Value2( _, _, StrEq(paramName2), _, _))
        .Times(1)
        .WillOnce(::testing::DoAll(
            SetPsmValueArg4(&expectedValue2),
            ::testing::Return(100)
        ));

    char paramName3[128] = {0};
    snprintf(paramName3,sizeof(paramName3), "dmsb.l2net.%d.Members.Link", InstanceNumber);
    char expectedValue3[128] = "";
    EXPECT_CALL(*g_psmMock, PSM_Get_Record_Value2( _, _, StrEq(paramName3), _, _))
        .Times(1)
        .WillOnce(::testing::DoAll(
            SetPsmValueArg4(&expectedValue3),
            ::testing::Return(100)
        ));

    char paramName4[128] = {0};
    snprintf(paramName4,sizeof(paramName4), "dmsb.l2net.%d.Members.Eth", InstanceNumber);
    char expectedValue4[128] = "";
    EXPECT_CALL(*g_psmMock, PSM_Get_Record_Value2( _, _, StrEq(paramName4), _, _))
        .Times(1)
        .WillOnce(::testing::DoAll(
            SetPsmValueArg4(&expectedValue4),
            ::testing::Return(100)
        ));

    char paramName5[128] = {0};
    snprintf(paramName5,sizeof(paramName5), "dmsb.l2net.%d.Members.Moca", InstanceNumber);
    char expectedValue5[128] = "moca0";
    EXPECT_CALL(*g_psmMock, PSM_Get_Record_Value2( _, _, StrEq(paramName5), _, _))
        .Times(1)
        .WillOnce(::testing::DoAll(
            SetPsmValueArg4(&expectedValue5),
            ::testing::Return(100)
        ));

    char paramName6[128] = {0};
    snprintf(paramName6,sizeof(paramName6), "dmsb.l2net.%d.Members.WiFi", InstanceNumber);
    char expectedValue6[128] = "";
    EXPECT_CALL(*g_psmMock, PSM_Get_Record_Value2( _, _, StrEq(paramName6), _, _))
        .Times(1)
        .WillOnce(::testing::DoAll(
            SetPsmValueArg4(&expectedValue6),
            ::testing::Return(100)
        ));

    char paramName7[128] = {0};
    snprintf(paramName7,sizeof(paramName7), "dmsb.l2net.%d.Members.Gre", InstanceNumber);
    char expectedValue7[128] = "";
    EXPECT_CALL(*g_psmMock, PSM_Get_Record_Value2( _, _, StrEq(paramName7), _, _))
        .Times(1)
        .WillOnce(::testing::DoAll(
            SetPsmValueArg4(&expectedValue7),
            ::testing::Return(100)
        ));
    EXPECT_CALL(*g_bridgeUtilsGenericMock, HandlePreConfigVendorGeneric(_, _))
	 .Times(1)
        .WillOnce(Return(0));
    char event1[64] = {0} , value1[64] = {0};
    snprintf(event1,sizeof(event1),"multinet_%d-name",InstanceNumber);
    strcpy(value1, "brlan10");
    EXPECT_CALL(*g_syseventMock, sysevent_set(_, _, StrEq(event1), StrEq(value1), _))
	 .Times(1)
        .WillOnce(Return(0));
    char event2[64] = {0} , value2[64] = {0};
    snprintf(event2,sizeof(event2),"multinet_%d-vid",InstanceNumber);
    snprintf(value2,sizeof(value2),"%d",111);
    EXPECT_CALL(*g_syseventMock, sysevent_set(_, _, StrEq(event2), StrEq(value2), _))
	 .Times(1)
        .WillOnce(Return(0));
    Gateway_Config *pGwConfig = NULL;
    pGwConfig = (Gateway_Config*) malloc(sizeof(Gateway_Config));
    memset(pGwConfig, 0, sizeof(Gateway_Config));
    pGwConfig->if_type = OVS_OTHER_IF_TYPE;
    pGwConfig->mtu = DEFAULT_MTU;
    pGwConfig->vlan_id = DEFAULT_VLAN_ID;
    pGwConfig->if_cmd = OVS_IF_UP_CMD;

    EXPECT_CALL(*g_ovsMock, ovs_agent_api_get_config( _, _))
        .Times(1)
        .WillOnce(::testing::DoAll(
            SetGwConfigArg1((void **)&pGwConfig),
            ::testing::Return(true)
        ));
    EXPECT_CALL(*g_ovsMock, ovs_agent_api_interact( _, _))
        .Times(1)
        .WillOnce(Return(true));

    char expectedValue111[128] = "7";
    EXPECT_CALL(*g_psmMock, PSM_Get_Record_Value2( _, _, StrEq("dmsb.MultiLAN.MoCAIsoLation_l3net"), _, _))
        .Times(1)
        .WillOnce(::testing::DoAll(
            SetPsmValueArg4(&expectedValue111),
            ::testing::Return(100)
        ));

    char paramName21[128] = {0};
    snprintf(paramName21,sizeof(paramName2), "dmsb.l3net.%d.V4Addr", 7);
    char expectedValue21[128] = "169.254.30.1";
    EXPECT_CALL(*g_psmMock, PSM_Get_Record_Value2( _, _, StrEq(paramName21), _, _))
        .Times(1)
        .WillOnce(::testing::DoAll(
            SetPsmValueArg4(&expectedValue21),
            ::testing::Return(100)
        ));

    char paramName31[128] = {0};
    snprintf(paramName31,sizeof(paramName31), "dmsb.l2net.%d.Name", PRIVATE_LAN);
    char expectedValue31[128] = "brlan0";
    EXPECT_CALL(*g_psmMock, PSM_Get_Record_Value2( _, _, StrEq(paramName31), _, _))
        .Times(1)
        .WillOnce(::testing::DoAll(
            SetPsmValueArg4(&expectedValue31),
            ::testing::Return(100)
        ));

    char expectedCmd123[512] = {0};
    snprintf(expectedCmd123,sizeof(expectedCmd123),"ip link set %s allmulticast on ;\
	    	ifconfig %s %s ; \
	    	ip link set %s up ; \
	    	echo 0 > /proc/sys/net/ipv4/icmp_echo_ignore_broadcasts ; \
	    	sysctl -w net.ipv4.conf.all.arp_announce=3 ; \
	    	ip rule add from all iif %s lookup all_lans ; \
	    	echo 0 > /proc/sys/net/ipv4/conf/%s/rp_filter ;\
	    	touch %s ;",
	    	expectedValue1,
	    	expectedValue1,
	    	expectedValue21,
	    	expectedValue1,
	    	expectedValue1,
	    	expectedValue1,
	    	LOCAL_MOCABR_UP_FILE);
    EXPECT_CALL(*g_utilMock, system(StrEq(expectedCmd123)))
        .Times(1)
        .WillOnce(Return(1));

    EXPECT_CALL(*g_bridgeUtilsGenericMock, HandlePostConfigVendorGeneric(_, _))
	 .Times(1)
        .WillOnce(Return(0));
    char event21[64] = {0} , value21[64] = {0};
    snprintf(event21,sizeof(event21),"multinet_%d-localready",InstanceNumber);
    strcpy(value21, "1");
    EXPECT_CALL(*g_syseventMock, sysevent_set(_, _, StrEq(event21), StrEq(value21), _))
	 .Times(1)
        .WillOnce(Return(0));
    char event3[64] = {0} , value3[64] = {0};
    snprintf(event3,sizeof(event3),"multinet_%d-status",InstanceNumber);
    strcpy(value3, "ready");
    EXPECT_CALL(*g_syseventMock, sysevent_set(_, _, StrEq(event3), StrEq(value3), _))
	 .Times(1)
        .WillOnce(Return(0));
    char event4[64] = {0} ;
    snprintf(event4,sizeof(event4),"firewall-restart");
    EXPECT_CALL(*g_syseventMock, sysevent_set(_, _, StrEq(event4), _, _))
	 .Times(1)
        .WillOnce(Return(0));
    CreateBrInterface();
}

TEST_F(BridgeUtilsTestFixture, CreateBrInterfaceMesh)
{
    char event[64] = {0} , value[64] = {0};
    InstanceNumber = MESH_BACKHAUL;
    ovsEnable = 1;
    DeviceMode = 0;
    ethWanEnabled = 0;
    wan_mode = 0;
    skipMoCA = 0;
    skipWiFi = 0;
    BridgeOprInPropgress = 1;
    snprintf(event,sizeof(event),"multinet_%d-status",InstanceNumber);
    strcpy(value, "partial");
    EXPECT_CALL(*g_syseventMock, sysevent_set(_, _, StrEq(event), StrEq(value), _))
	 .Times(1)
        .WillOnce(Return(0));
    char paramName1[128] = {0};
    snprintf(paramName1,sizeof(paramName1), "dmsb.l2net.%d.Name", InstanceNumber);
    char expectedValue1[128] = "br403";
    EXPECT_CALL(*g_psmMock, PSM_Get_Record_Value2( _, _, StrEq(paramName1), _, _))
        .Times(1)
        .WillOnce(::testing::DoAll(
            SetPsmValueArg4(&expectedValue1),
            ::testing::Return(100)
        ));

    char paramName2[128] = {0};
    snprintf(paramName2,sizeof(paramName2), "dmsb.l2net.%d.Vid", InstanceNumber);
    char expectedValue2[128] = "1060";
    EXPECT_CALL(*g_psmMock, PSM_Get_Record_Value2( _, _, StrEq(paramName2), _, _))
        .Times(1)
        .WillOnce(::testing::DoAll(
            SetPsmValueArg4(&expectedValue2),
            ::testing::Return(100)
        ));

    char paramName3[128] = {0};
    snprintf(paramName3,sizeof(paramName3), "dmsb.l2net.%d.Members.Link", InstanceNumber);
    char expectedValue3[128] = "";
    EXPECT_CALL(*g_psmMock, PSM_Get_Record_Value2( _, _, StrEq(paramName3), _, _))
        .Times(1)
        .WillOnce(::testing::DoAll(
            SetPsmValueArg4(&expectedValue3),
            ::testing::Return(100)
        ));

    char paramName4[128] = {0};
    snprintf(paramName4,sizeof(paramName4), "dmsb.l2net.%d.Members.Eth", InstanceNumber);
    char expectedValue4[128] = "";
    EXPECT_CALL(*g_psmMock, PSM_Get_Record_Value2( _, _, StrEq(paramName4), _, _))
        .Times(1)
        .WillOnce(::testing::DoAll(
            SetPsmValueArg4(&expectedValue4),
            ::testing::Return(100)
        ));

    char paramName5[128] = {0};
    snprintf(paramName5,sizeof(paramName5), "dmsb.l2net.%d.Members.Moca", InstanceNumber);
    char expectedValue5[128] = "";
    EXPECT_CALL(*g_psmMock, PSM_Get_Record_Value2( _, _, StrEq(paramName5), _, _))
        .Times(1)
        .WillOnce(::testing::DoAll(
            SetPsmValueArg4(&expectedValue5),
            ::testing::Return(100)
        ));

    char paramName6[128] = {0};
    snprintf(paramName6,sizeof(paramName6), "dmsb.l2net.%d.Members.WiFi", InstanceNumber);
    char expectedValue6[128] = "";
    EXPECT_CALL(*g_psmMock, PSM_Get_Record_Value2( _, _, StrEq(paramName6), _, _))
        .Times(1)
        .WillOnce(::testing::DoAll(
            SetPsmValueArg4(&expectedValue6),
            ::testing::Return(100)
        ));

    char paramName7[128] = {0};
    snprintf(paramName7,sizeof(paramName7), "dmsb.l2net.%d.Members.Gre", InstanceNumber);
    char expectedValue7[128] = "";
    EXPECT_CALL(*g_psmMock, PSM_Get_Record_Value2( _, _, StrEq(paramName7), _, _))
        .Times(1)
        .WillOnce(::testing::DoAll(
            SetPsmValueArg4(&expectedValue7),
            ::testing::Return(100)
        ));
    EXPECT_CALL(*g_bridgeUtilsGenericMock, HandlePreConfigVendorGeneric(_, _))
	 .Times(1)
        .WillOnce(Return(0));
    char event1[64] = {0} , value1[64] = {0};
    snprintf(event1,sizeof(event1),"multinet_%d-name",InstanceNumber);
    strcpy(value1, "br403");
    EXPECT_CALL(*g_syseventMock, sysevent_set(_, _, StrEq(event1), StrEq(value1), _))
	 .Times(1)
        .WillOnce(Return(0));
    char event2[64] = {0} , value2[64] = {0};
    snprintf(event2,sizeof(event2),"multinet_%d-vid",InstanceNumber);
    snprintf(value2,sizeof(value2),"%d",1060);
    EXPECT_CALL(*g_syseventMock, sysevent_set(_, _, StrEq(event2), StrEq(value2), _))
	 .Times(1)
        .WillOnce(Return(0));
    Gateway_Config *pGwConfig = NULL;
    pGwConfig = (Gateway_Config*) malloc(sizeof(Gateway_Config));
    memset(pGwConfig, 0, sizeof(Gateway_Config));
    pGwConfig->if_type = OVS_OTHER_IF_TYPE;
    pGwConfig->mtu = DEFAULT_MTU;
    pGwConfig->vlan_id = DEFAULT_VLAN_ID;
    pGwConfig->if_cmd = OVS_IF_UP_CMD;

    EXPECT_CALL(*g_ovsMock, ovs_agent_api_get_config( _, _))
        .Times(1)
        .WillOnce(::testing::DoAll(
            SetGwConfigArg1((void **)&pGwConfig),
            ::testing::Return(true)
        ));
    EXPECT_CALL(*g_ovsMock, ovs_agent_api_interact( _, _))
        .Times(1)
        .WillOnce(Return(true));

    char expectedValue111[128] = "9";
    EXPECT_CALL(*g_psmMock, PSM_Get_Record_Value2( _, _, StrEq("dmsb.MultiLAN.MeshBhaul_l3net"), _, _))
        .Times(1)
        .WillOnce(::testing::DoAll(
            SetPsmValueArg4(&expectedValue111),
            ::testing::Return(100)
        ));

    char paramName21[128] = {0};
    snprintf(paramName21,sizeof(paramName2), "dmsb.l3net.%d.V4Addr", 9);
    char expectedValue21[128] = "192.168.245.254";
    EXPECT_CALL(*g_psmMock, PSM_Get_Record_Value2( _, _, StrEq(paramName21), _, _))
        .Times(1)
        .WillOnce(::testing::DoAll(
            SetPsmValueArg4(&expectedValue21),
            ::testing::Return(100)
        ));

    char paramName31[128] = {0};
    snprintf(paramName31,sizeof(paramName31), "dmsb.l3net.%d.V4SubnetMask", 9);
    char expectedValue31[128] = "";
    EXPECT_CALL(*g_psmMock, PSM_Get_Record_Value2( _, _, StrEq(paramName31), _, _))
        .Times(1)
        .WillOnce(::testing::DoAll(
            SetPsmValueArg4(&expectedValue31),
            ::testing::Return(100)
        ));

    char expectedCmd123[216] = {0};
    snprintf(expectedCmd123,sizeof(expectedCmd123),"ifconfig %s %s",expectedValue1,expectedValue21);
    EXPECT_CALL(*g_utilMock, system(StrEq(expectedCmd123)))
        .Times(1)
        .WillOnce(Return(1));

    EXPECT_CALL(*g_bridgeUtilsGenericMock, HandlePostConfigVendorGeneric(_, _))
	 .Times(1)
        .WillOnce(Return(0));
    char event21[64] = {0} , value21[64] = {0};
    snprintf(event21,sizeof(event21),"multinet_%d-localready",InstanceNumber);
    strcpy(value21, "1");
    EXPECT_CALL(*g_syseventMock, sysevent_set(_, _, StrEq(event21), StrEq(value21), _))
	 .Times(1)
        .WillOnce(Return(0));
    char event3[64] = {0} , value3[64] = {0};
    snprintf(event3,sizeof(event3),"multinet_%d-status",InstanceNumber);
    strcpy(value3, "ready");
    EXPECT_CALL(*g_syseventMock, sysevent_set(_, _, StrEq(event3), StrEq(value3), _))
	 .Times(1)
        .WillOnce(Return(0));
    char event4[64] = {0} ;
    snprintf(event4,sizeof(event4),"firewall-restart");
    EXPECT_CALL(*g_syseventMock, sysevent_set(_, _, StrEq(event4), _, _))
	 .Times(1)
        .WillOnce(Return(0));
    CreateBrInterface();
}

TEST_F(BridgeUtilsTestFixture, CreateBrInterfaceMeshWiFi2G)
{
    char event[64] = {0} , value[64] = {0};
    InstanceNumber = MESH_WIFI_BACKHAUL_2G;
    ovsEnable = 1;
    DeviceMode = 0;
    ethWanEnabled = 0;
    wan_mode = 0;
    skipMoCA = 0;
    skipWiFi = 0;
    snprintf(event,sizeof(event),"multinet_%d-status",InstanceNumber);
    strcpy(value, "partial");
    EXPECT_CALL(*g_syseventMock, sysevent_set(_, _, StrEq(event), StrEq(value), _))
	 .Times(1)
        .WillOnce(Return(0));
    char paramName1[128] = {0};
    snprintf(paramName1,sizeof(paramName1), "dmsb.l2net.%d.Name", InstanceNumber);
    char expectedValue1[128] = "brlan112";
    EXPECT_CALL(*g_psmMock, PSM_Get_Record_Value2( _, _, StrEq(paramName1), _, _))
        .Times(1)
        .WillOnce(::testing::DoAll(
            SetPsmValueArg4(&expectedValue1),
            ::testing::Return(100)
        ));

    char paramName2[128] = {0};
    snprintf(paramName2,sizeof(paramName2), "dmsb.l2net.%d.Vid", InstanceNumber);
    char expectedValue2[128] = "112";
    EXPECT_CALL(*g_psmMock, PSM_Get_Record_Value2( _, _, StrEq(paramName2), _, _))
        .Times(1)
        .WillOnce(::testing::DoAll(
            SetPsmValueArg4(&expectedValue2),
            ::testing::Return(100)
        ));

    char paramName3[128] = {0};
    snprintf(paramName3,sizeof(paramName3), "dmsb.l2net.%d.Members.Link", InstanceNumber);
    char expectedValue3[128] = "";
    EXPECT_CALL(*g_psmMock, PSM_Get_Record_Value2( _, _, StrEq(paramName3), _, _))
        .Times(1)
        .WillOnce(::testing::DoAll(
            SetPsmValueArg4(&expectedValue3),
            ::testing::Return(100)
        ));

    char paramName4[128] = {0};
    snprintf(paramName4,sizeof(paramName4), "dmsb.l2net.%d.Members.Eth", InstanceNumber);
    char expectedValue4[128] = "";
    EXPECT_CALL(*g_psmMock, PSM_Get_Record_Value2( _, _, StrEq(paramName4), _, _))
        .Times(1)
        .WillOnce(::testing::DoAll(
            SetPsmValueArg4(&expectedValue4),
            ::testing::Return(100)
        ));

    char paramName5[128] = {0};
    snprintf(paramName5,sizeof(paramName5), "dmsb.l2net.%d.Members.Moca", InstanceNumber);
    char expectedValue5[128] = "";
    EXPECT_CALL(*g_psmMock, PSM_Get_Record_Value2( _, _, StrEq(paramName5), _, _))
        .Times(1)
        .WillOnce(::testing::DoAll(
            SetPsmValueArg4(&expectedValue5),
            ::testing::Return(100)
        ));

    char paramName6[128] = {0};
    snprintf(paramName6,sizeof(paramName6), "dmsb.l2net.%d.Members.WiFi", InstanceNumber);
    char expectedValue6[128] = "wifi0";
    EXPECT_CALL(*g_psmMock, PSM_Get_Record_Value2( _, _, StrEq(paramName6), _, _))
        .Times(1)
        .WillOnce(::testing::DoAll(
            SetPsmValueArg4(&expectedValue6),
            ::testing::Return(100)
        ));

    char paramName7[128] = {0};
    snprintf(paramName7,sizeof(paramName7), "dmsb.l2net.%d.Members.Gre", InstanceNumber);
    char expectedValue7[128] = "";
    EXPECT_CALL(*g_psmMock, PSM_Get_Record_Value2( _, _, StrEq(paramName7), _, _))
        .Times(1)
        .WillOnce(::testing::DoAll(
            SetPsmValueArg4(&expectedValue7),
            ::testing::Return(100)
        ));
    EXPECT_CALL(*g_bridgeUtilsGenericMock, HandlePreConfigVendorGeneric(_, _))
	 .Times(1)
        .WillOnce(Return(0));
    char event1[64] = {0} , value1[64] = {0};
    snprintf(event1,sizeof(event1),"multinet_%d-name",InstanceNumber);
    strcpy(value1, "brlan112");
    EXPECT_CALL(*g_syseventMock, sysevent_set(_, _, StrEq(event1), StrEq(value1), _))
	 .Times(1)
        .WillOnce(Return(0));
    char event2[64] = {0} , value2[64] = {0};
    snprintf(event2,sizeof(event2),"multinet_%d-vid",InstanceNumber);
    snprintf(value2,sizeof(value2),"%d",112);
    EXPECT_CALL(*g_syseventMock, sysevent_set(_, _, StrEq(event2), StrEq(value2), _))
	 .Times(1)
        .WillOnce(Return(0));
    Gateway_Config *pGwConfig = NULL;
    pGwConfig = (Gateway_Config*) malloc(sizeof(Gateway_Config));
    memset(pGwConfig, 0, sizeof(Gateway_Config));
    pGwConfig->if_type = OVS_OTHER_IF_TYPE;
    pGwConfig->mtu = DEFAULT_MTU;
    pGwConfig->vlan_id = DEFAULT_VLAN_ID;
    pGwConfig->if_cmd = OVS_IF_UP_CMD;

    EXPECT_CALL(*g_ovsMock, ovs_agent_api_get_config( _, _))
        .Times(1)
        .WillOnce(::testing::DoAll(
            SetGwConfigArg1((void **)&pGwConfig),
            ::testing::Return(true)
        ));
    EXPECT_CALL(*g_ovsMock, ovs_agent_api_interact( _, _))
        .Times(1)
        .WillOnce(Return(true));

    char expectedValue111[128] = "10";
    EXPECT_CALL(*g_psmMock, PSM_Get_Record_Value2( _, _, StrEq("dmsb.MultiLAN.MeshWiFiBhaul_2G_l3net"), _, _))
        .Times(1)
        .WillOnce(::testing::DoAll(
            SetPsmValueArg4(&expectedValue111),
            ::testing::Return(100)
        ));

    char paramName21[128] = {0};
    snprintf(paramName21,sizeof(paramName2), "dmsb.l3net.%d.V4Addr", 10);
    char expectedValue21[128] = "169.254.0.1";
    EXPECT_CALL(*g_psmMock, PSM_Get_Record_Value2( _, _, StrEq(paramName21), _, _))
        .Times(1)
        .WillOnce(::testing::DoAll(
            SetPsmValueArg4(&expectedValue21),
            ::testing::Return(100)
        ));

    char paramName31[128] = {0};
    snprintf(paramName31,sizeof(paramName31), "dmsb.l3net.%d.V4SubnetMask", 10);
    char expectedValue31[128] = "255.255.255.0";
    EXPECT_CALL(*g_psmMock, PSM_Get_Record_Value2( _, _, StrEq(paramName31), _, _))
        .Times(1)
        .WillOnce(::testing::DoAll(
            SetPsmValueArg4(&expectedValue31),
            ::testing::Return(100)
        ));

    char expectedCmd123[216] = {0};
    snprintf(expectedCmd123,sizeof(expectedCmd123),"ifconfig %s %s netmask %s up",expectedValue1,expectedValue21, expectedValue31);
    EXPECT_CALL(*g_utilMock, system(StrEq(expectedCmd123)))
        .Times(1)
        .WillOnce(Return(1));

    EXPECT_CALL(*g_bridgeUtilsGenericMock, HandlePostConfigVendorGeneric(_, _))
	 .Times(1)
        .WillOnce(Return(0));
    char event21[64] = {0} , value21[64] = {0};
    snprintf(event21,sizeof(event21),"multinet_%d-localready",InstanceNumber);
    strcpy(value21, "1");
    EXPECT_CALL(*g_syseventMock, sysevent_set(_, _, StrEq(event21), StrEq(value21), _))
	 .Times(1)
        .WillOnce(Return(0));
    char event3[64] = {0} , value3[64] = {0};
    snprintf(event3,sizeof(event3),"multinet_%d-status",InstanceNumber);
    strcpy(value3, "ready");
    EXPECT_CALL(*g_syseventMock, sysevent_set(_, _, StrEq(event3), StrEq(value3), _))
	 .Times(1)
        .WillOnce(Return(0));
    char event4[64] = {0} ;
    snprintf(event4,sizeof(event4),"firewall-restart");
    EXPECT_CALL(*g_syseventMock, sysevent_set(_, _, StrEq(event4), _, _))
	 .Times(1)
        .WillOnce(Return(0));
    CreateBrInterface();
}

TEST_F(BridgeUtilsTestFixture, CreateBrInterfaceMeshWiFi5G)
{
    char event[64] = {0} , value[64] = {0};
    InstanceNumber = MESH_WIFI_BACKHAUL_5G;
    ovsEnable = 1;
    DeviceMode = 0;
    ethWanEnabled = 0;
    wan_mode = 0;
    skipMoCA = 0;
    skipWiFi = 0;
    snprintf(event,sizeof(event),"multinet_%d-status",InstanceNumber);
    strcpy(value, "partial");
    EXPECT_CALL(*g_syseventMock, sysevent_set(_, _, StrEq(event), StrEq(value), _))
	 .Times(1)
        .WillOnce(Return(0));
    char paramName1[128] = {0};
    snprintf(paramName1,sizeof(paramName1), "dmsb.l2net.%d.Name", InstanceNumber);
    char expectedValue1[128] = "brlan113";
    EXPECT_CALL(*g_psmMock, PSM_Get_Record_Value2( _, _, StrEq(paramName1), _, _))
        .Times(1)
        .WillOnce(::testing::DoAll(
            SetPsmValueArg4(&expectedValue1),
            ::testing::Return(100)
        ));

    char paramName2[128] = {0};
    snprintf(paramName2,sizeof(paramName2), "dmsb.l2net.%d.Vid", InstanceNumber);
    char expectedValue2[128] = "113";
    EXPECT_CALL(*g_psmMock, PSM_Get_Record_Value2( _, _, StrEq(paramName2), _, _))
        .Times(1)
        .WillOnce(::testing::DoAll(
            SetPsmValueArg4(&expectedValue2),
            ::testing::Return(100)
        ));

    char paramName3[128] = {0};
    snprintf(paramName3,sizeof(paramName3), "dmsb.l2net.%d.Members.Link", InstanceNumber);
    char expectedValue3[128] = "";
    EXPECT_CALL(*g_psmMock, PSM_Get_Record_Value2( _, _, StrEq(paramName3), _, _))
        .Times(1)
        .WillOnce(::testing::DoAll(
            SetPsmValueArg4(&expectedValue3),
            ::testing::Return(100)
        ));

    char paramName4[128] = {0};
    snprintf(paramName4,sizeof(paramName4), "dmsb.l2net.%d.Members.Eth", InstanceNumber);
    char expectedValue4[128] = "";
    EXPECT_CALL(*g_psmMock, PSM_Get_Record_Value2( _, _, StrEq(paramName4), _, _))
        .Times(1)
        .WillOnce(::testing::DoAll(
            SetPsmValueArg4(&expectedValue4),
            ::testing::Return(100)
        ));

    char paramName5[128] = {0};
    snprintf(paramName5,sizeof(paramName5), "dmsb.l2net.%d.Members.Moca", InstanceNumber);
    char expectedValue5[128] = "";
    EXPECT_CALL(*g_psmMock, PSM_Get_Record_Value2( _, _, StrEq(paramName5), _, _))
        .Times(1)
        .WillOnce(::testing::DoAll(
            SetPsmValueArg4(&expectedValue5),
            ::testing::Return(100)
        ));

    char paramName6[128] = {0};
    snprintf(paramName6,sizeof(paramName6), "dmsb.l2net.%d.Members.WiFi", InstanceNumber);
    char expectedValue6[128] = "wifi0";
    EXPECT_CALL(*g_psmMock, PSM_Get_Record_Value2( _, _, StrEq(paramName6), _, _))
        .Times(1)
        .WillOnce(::testing::DoAll(
            SetPsmValueArg4(&expectedValue6),
            ::testing::Return(100)
        ));

    char paramName7[128] = {0};
    snprintf(paramName7,sizeof(paramName7), "dmsb.l2net.%d.Members.Gre", InstanceNumber);
    char expectedValue7[128] = "";
    EXPECT_CALL(*g_psmMock, PSM_Get_Record_Value2( _, _, StrEq(paramName7), _, _))
        .Times(1)
        .WillOnce(::testing::DoAll(
            SetPsmValueArg4(&expectedValue7),
            ::testing::Return(100)
        ));
    EXPECT_CALL(*g_bridgeUtilsGenericMock, HandlePreConfigVendorGeneric(_, _))
	 .Times(1)
        .WillOnce(Return(0));
    char event1[64] = {0} , value1[64] = {0};
    snprintf(event1,sizeof(event1),"multinet_%d-name",InstanceNumber);
    strcpy(value1, "brlan113");
    EXPECT_CALL(*g_syseventMock, sysevent_set(_, _, StrEq(event1), StrEq(value1), _))
	 .Times(1)
        .WillOnce(Return(0));
    char event2[64] = {0} , value2[64] = {0};
    snprintf(event2,sizeof(event2),"multinet_%d-vid",InstanceNumber);
    snprintf(value2,sizeof(value2),"%d",113);
    EXPECT_CALL(*g_syseventMock, sysevent_set(_, _, StrEq(event2), StrEq(value2), _))
	 .Times(1)
        .WillOnce(Return(0));
    Gateway_Config *pGwConfig = NULL;
    pGwConfig = (Gateway_Config*) malloc(sizeof(Gateway_Config));
    memset(pGwConfig, 0, sizeof(Gateway_Config));
    pGwConfig->if_type = OVS_OTHER_IF_TYPE;
    pGwConfig->mtu = DEFAULT_MTU;
    pGwConfig->vlan_id = DEFAULT_VLAN_ID;
    pGwConfig->if_cmd = OVS_IF_UP_CMD;

    EXPECT_CALL(*g_ovsMock, ovs_agent_api_get_config( _, _))
        .Times(1)
        .WillOnce(::testing::DoAll(
            SetGwConfigArg1((void **)&pGwConfig),
            ::testing::Return(true)
        ));
    EXPECT_CALL(*g_ovsMock, ovs_agent_api_interact( _, _))
        .Times(1)
        .WillOnce(Return(true));

    char expectedValue111[128] = "11";
    EXPECT_CALL(*g_psmMock, PSM_Get_Record_Value2( _, _, StrEq("dmsb.MultiLAN.MeshWiFiBhaul_5G_l3net"), _, _))
        .Times(1)
        .WillOnce(::testing::DoAll(
            SetPsmValueArg4(&expectedValue111),
            ::testing::Return(100)
        ));

    char paramName21[128] = {0};
    snprintf(paramName21,sizeof(paramName2), "dmsb.l3net.%d.V4Addr", 11);
    char expectedValue21[128] = "169.254.1.1";
    EXPECT_CALL(*g_psmMock, PSM_Get_Record_Value2( _, _, StrEq(paramName21), _, _))
        .Times(1)
        .WillOnce(::testing::DoAll(
            SetPsmValueArg4(&expectedValue21),
            ::testing::Return(100)
        ));

    char paramName31[128] = {0};
    snprintf(paramName31,sizeof(paramName31), "dmsb.l3net.%d.V4SubnetMask", 11);
    char expectedValue31[128] = "255.255.255.0";
    EXPECT_CALL(*g_psmMock, PSM_Get_Record_Value2( _, _, StrEq(paramName31), _, _))
        .Times(1)
        .WillOnce(::testing::DoAll(
            SetPsmValueArg4(&expectedValue31),
            ::testing::Return(100)
        ));

    char expectedCmd123[216] = {0};
    snprintf(expectedCmd123,sizeof(expectedCmd123),"ifconfig %s %s netmask %s up",expectedValue1,expectedValue21, expectedValue31);
    EXPECT_CALL(*g_utilMock, system(StrEq(expectedCmd123)))
        .Times(1)
        .WillOnce(Return(1));

    EXPECT_CALL(*g_bridgeUtilsGenericMock, HandlePostConfigVendorGeneric(_, _))
	 .Times(1)
        .WillOnce(Return(0));
    char event21[64] = {0} , value21[64] = {0};
    snprintf(event21,sizeof(event21),"multinet_%d-localready",InstanceNumber);
    strcpy(value21, "1");
    EXPECT_CALL(*g_syseventMock, sysevent_set(_, _, StrEq(event21), StrEq(value21), _))
	 .Times(1)
        .WillOnce(Return(0));
    char event3[64] = {0} , value3[64] = {0};
    snprintf(event3,sizeof(event3),"multinet_%d-status",InstanceNumber);
    strcpy(value3, "ready");
    EXPECT_CALL(*g_syseventMock, sysevent_set(_, _, StrEq(event3), StrEq(value3), _))
	 .Times(1)
        .WillOnce(Return(0));
    char event4[64] = {0} ;
    snprintf(event4,sizeof(event4),"firewall-restart");
    EXPECT_CALL(*g_syseventMock, sysevent_set(_, _, StrEq(event4), _, _))
	 .Times(1)
        .WillOnce(Return(0));
    CreateBrInterface();
}

TEST_F(BridgeUtilsTestFixture, CreateBrInterfaceHotspot2G)
{
    char event[64] = {0} , value[64] = {0};
    InstanceNumber = HOTSPOT_2G;
    ovsEnable = 1;
    DeviceMode = 0;
    ethWanEnabled = 0;
    wan_mode = 0;
    skipMoCA = 0;
    skipWiFi = 0;
    snprintf(event,sizeof(event),"multinet_%d-status",InstanceNumber);
    strcpy(value, "partial");
    EXPECT_CALL(*g_syseventMock, sysevent_set(_, _, StrEq(event), StrEq(value), _))
	 .Times(1)
        .WillOnce(Return(0));
    char paramName1[128] = {0};
    snprintf(paramName1,sizeof(paramName1), "dmsb.l2net.%d.Name", InstanceNumber);
    char expectedValue1[128] = "brlan2";
    EXPECT_CALL(*g_psmMock, PSM_Get_Record_Value2( _, _, StrEq(paramName1), _, _))
        .Times(1)
        .WillOnce(::testing::DoAll(
            SetPsmValueArg4(&expectedValue1),
            ::testing::Return(100)
        ));

    char paramName2[128] = {0};
    snprintf(paramName2,sizeof(paramName2), "dmsb.l2net.%d.Vid", InstanceNumber);
    char expectedValue2[128] = "102";
    EXPECT_CALL(*g_psmMock, PSM_Get_Record_Value2( _, _, StrEq(paramName2), _, _))
        .Times(1)
        .WillOnce(::testing::DoAll(
            SetPsmValueArg4(&expectedValue2),
            ::testing::Return(100)
        ));

    char paramName3[128] = {0};
    snprintf(paramName3,sizeof(paramName3), "dmsb.l2net.%d.Members.Link", InstanceNumber);
    char expectedValue3[128] = "";
    EXPECT_CALL(*g_psmMock, PSM_Get_Record_Value2( _, _, StrEq(paramName3), _, _))
        .Times(1)
        .WillOnce(::testing::DoAll(
            SetPsmValueArg4(&expectedValue3),
            ::testing::Return(100)
        ));

    char paramName4[128] = {0};
    snprintf(paramName4,sizeof(paramName4), "dmsb.l2net.%d.Members.Eth", InstanceNumber);
    char expectedValue4[128] = "";
    EXPECT_CALL(*g_psmMock, PSM_Get_Record_Value2( _, _, StrEq(paramName4), _, _))
        .Times(1)
        .WillOnce(::testing::DoAll(
            SetPsmValueArg4(&expectedValue4),
            ::testing::Return(100)
        ));

    char paramName5[128] = {0};
    snprintf(paramName5,sizeof(paramName5), "dmsb.l2net.%d.Members.Moca", InstanceNumber);
    char expectedValue5[128] = "";
    EXPECT_CALL(*g_psmMock, PSM_Get_Record_Value2( _, _, StrEq(paramName5), _, _))
        .Times(1)
        .WillOnce(::testing::DoAll(
            SetPsmValueArg4(&expectedValue5),
            ::testing::Return(100)
        ));

    char paramName6[128] = {0};
    snprintf(paramName6,sizeof(paramName6), "dmsb.l2net.%d.Members.WiFi", InstanceNumber);
    char expectedValue6[128] = "wifi0";
    EXPECT_CALL(*g_psmMock, PSM_Get_Record_Value2( _, _, StrEq(paramName6), _, _))
        .Times(1)
        .WillOnce(::testing::DoAll(
            SetPsmValueArg4(&expectedValue6),
            ::testing::Return(100)
        ));

    char paramName7[128] = {0};
    snprintf(paramName7,sizeof(paramName7), "dmsb.l2net.%d.Members.Gre", InstanceNumber);
    char expectedValue7[128] = "gre0";
    EXPECT_CALL(*g_psmMock, PSM_Get_Record_Value2( _, _, StrEq(paramName7), _, _))
        .Times(1)
        .WillOnce(::testing::DoAll(
            SetPsmValueArg4(&expectedValue7),
            ::testing::Return(100)
        ));

    char expectedValue[] = "1";
    EXPECT_CALL(*g_psmMock, PSM_Get_Record_Value2( _, _, StrEq("dmsb.hotspot.enable"), _, _))
        .Times(1)
        .WillOnce(::testing::DoAll(
            SetPsmValueArg4(&expectedValue),
            ::testing::Return(100)
        ));

    char expectedCmd123[216] = {0};
    snprintf(expectedCmd123,sizeof(expectedCmd123),"sh %s create %d %s",GRE_HANDLER_SCRIPT,InstanceNumber, expectedValue7);
    EXPECT_CALL(*g_utilMock, system(StrEq(expectedCmd123)))
        .Times(1)
        .WillOnce(Return(1));

    EXPECT_CALL(*g_bridgeUtilsGenericMock, HandlePreConfigVendorGeneric(_, _))
	 .Times(1)
        .WillOnce(Return(0));
    char event1[64] = {0} , value1[64] = {0};
    snprintf(event1,sizeof(event1),"multinet_%d-name",InstanceNumber);
    strcpy(value1, "brlan2");
    EXPECT_CALL(*g_syseventMock, sysevent_set(_, _, StrEq(event1), StrEq(value1), _))
	 .Times(1)
        .WillOnce(Return(0));
    char event2[64] = {0} , value2[64] = {0};
    snprintf(event2,sizeof(event2),"multinet_%d-vid",InstanceNumber);
    snprintf(value2,sizeof(value2),"%d",102);
    EXPECT_CALL(*g_syseventMock, sysevent_set(_, _, StrEq(event2), StrEq(value2), _))
	 .Times(1)
        .WillOnce(Return(0));
    Gateway_Config *pGwConfig = NULL;
    pGwConfig = (Gateway_Config*) malloc(sizeof(Gateway_Config));
    memset(pGwConfig, 0, sizeof(Gateway_Config));
    pGwConfig->if_type = OVS_OTHER_IF_TYPE;
    pGwConfig->mtu = DEFAULT_MTU;
    pGwConfig->vlan_id = DEFAULT_VLAN_ID;
    pGwConfig->if_cmd = OVS_IF_UP_CMD;
    char GreState[64] = {0} , greSysName[128] = {0};
    snprintf(greSysName,sizeof(greSysName),"if_%s-status","gre0");
    strcpy(GreState, "ready");
    EXPECT_CALL(*g_syseventMock, sysevent_get(_, _, StrEq(greSysName), _, _))
	 .Times(1)
        .WillOnce(::testing::DoAll(
            SetArgNPointeeTo<3>(std::begin(GreState), sizeof(GreState)),
            ::testing::Return(0)
        ));
    EXPECT_CALL(*g_ovsMock, ovs_agent_api_get_config( _, _))
        .Times(2)
        .WillOnce(::testing::DoAll(
            SetGwConfigArg1((void **)&pGwConfig),
            ::testing::Return(true)
        ))
        .WillOnce(::testing::DoAll(
            SetGwConfigArg1((void **)&pGwConfig),
            ::testing::Return(true)
        ));
    EXPECT_CALL(*g_ovsMock, ovs_agent_api_interact( _, _))
        .Times(2)
        .WillOnce(Return(true))
        .WillOnce(Return(true));

    EXPECT_CALL(*g_bridgeUtilsGenericMock, HandlePostConfigVendorGeneric(_, _))
	 .Times(1)
        .WillOnce(Return(0));
    char event21[64] = {0} , value21[64] = {0};
    snprintf(event21,sizeof(event21),"multinet_%d-localready",InstanceNumber);
    strcpy(value21, "1");
    EXPECT_CALL(*g_syseventMock, sysevent_set(_, _, StrEq(event21), StrEq(value21), _))
	 .Times(1)
        .WillOnce(Return(0));
    char event3[64] = {0} , value3[64] = {0};
    snprintf(event3,sizeof(event3),"multinet_%d-status",InstanceNumber);
    strcpy(value3, "ready");
    EXPECT_CALL(*g_syseventMock, sysevent_set(_, _, StrEq(event3), StrEq(value3), _))
	 .Times(1)
        .WillOnce(Return(0));
    char event4[64] = {0} ;
    snprintf(event4,sizeof(event4),"firewall-restart");
    EXPECT_CALL(*g_syseventMock, sysevent_set(_, _, StrEq(event4), _, _))
	 .Times(1)
        .WillOnce(Return(0));
    CreateBrInterface();
}


TEST_F(BridgeUtilsTestFixture, CreateBrInterfaceHotspotSecure5G)
{
    char event[64] = {0} , value[64] = {0};
    InstanceNumber = HOTSPOT_SECURE_5G;
    ovsEnable = 1;
    DeviceMode = 0;
    ethWanEnabled = 0;
    wan_mode = 0;
    skipMoCA = 0;
    skipWiFi = 0;
    snprintf(event,sizeof(event),"multinet_%d-status",InstanceNumber);
    strcpy(value, "partial");
    EXPECT_CALL(*g_syseventMock, sysevent_set(_, _, StrEq(event), StrEq(value), _))
	 .Times(1)
        .WillOnce(Return(0));
    char paramName1[128] = {0};
    snprintf(paramName1,sizeof(paramName1), "dmsb.l2net.%d.Name", InstanceNumber);
    char expectedValue1[128] = "brlan5";
    EXPECT_CALL(*g_psmMock, PSM_Get_Record_Value2( _, _, StrEq(paramName1), _, _))
        .Times(1)
        .WillOnce(::testing::DoAll(
            SetPsmValueArg4(&expectedValue1),
            ::testing::Return(100)
        ));

    char paramName2[128] = {0};
    snprintf(paramName2,sizeof(paramName2), "dmsb.l2net.%d.Vid", InstanceNumber);
    char expectedValue2[128] = "105";
    EXPECT_CALL(*g_psmMock, PSM_Get_Record_Value2( _, _, StrEq(paramName2), _, _))
        .Times(1)
        .WillOnce(::testing::DoAll(
            SetPsmValueArg4(&expectedValue2),
            ::testing::Return(100)
        ));

    char paramName3[128] = {0};
    snprintf(paramName3,sizeof(paramName3), "dmsb.l2net.%d.Members.Link", InstanceNumber);
    char expectedValue3[128] = "";
    EXPECT_CALL(*g_psmMock, PSM_Get_Record_Value2( _, _, StrEq(paramName3), _, _))
        .Times(1)
        .WillOnce(::testing::DoAll(
            SetPsmValueArg4(&expectedValue3),
            ::testing::Return(100)
        ));

    char paramName4[128] = {0};
    snprintf(paramName4,sizeof(paramName4), "dmsb.l2net.%d.Members.Eth", InstanceNumber);
    char expectedValue4[128] = "";
    EXPECT_CALL(*g_psmMock, PSM_Get_Record_Value2( _, _, StrEq(paramName4), _, _))
        .Times(1)
        .WillOnce(::testing::DoAll(
            SetPsmValueArg4(&expectedValue4),
            ::testing::Return(100)
        ));

    char paramName5[128] = {0};
    snprintf(paramName5,sizeof(paramName5), "dmsb.l2net.%d.Members.Moca", InstanceNumber);
    char expectedValue5[128] = "";
    EXPECT_CALL(*g_psmMock, PSM_Get_Record_Value2( _, _, StrEq(paramName5), _, _))
        .Times(1)
        .WillOnce(::testing::DoAll(
            SetPsmValueArg4(&expectedValue5),
            ::testing::Return(100)
        ));

    char paramName6[128] = {0};
    snprintf(paramName6,sizeof(paramName6), "dmsb.l2net.%d.Members.WiFi", InstanceNumber);
    char expectedValue6[128] = "wifi0";
    EXPECT_CALL(*g_psmMock, PSM_Get_Record_Value2( _, _, StrEq(paramName6), _, _))
        .Times(1)
        .WillOnce(::testing::DoAll(
            SetPsmValueArg4(&expectedValue6),
            ::testing::Return(100)
        ));

    char paramName7[128] = {0};
    snprintf(paramName7,sizeof(paramName7), "dmsb.l2net.%d.Members.Gre", InstanceNumber);
    char expectedValue7[128] = "gre0";
    EXPECT_CALL(*g_psmMock, PSM_Get_Record_Value2( _, _, StrEq(paramName7), _, _))
        .Times(1)
        .WillOnce(::testing::DoAll(
            SetPsmValueArg4(&expectedValue7),
            ::testing::Return(100)
        ));

    char expectedValue[] = "0";
    EXPECT_CALL(*g_psmMock, PSM_Get_Record_Value2( _, _, StrEq("dmsb.hotspot.enable"), _, _))
        .Times(1)
        .WillOnce(::testing::DoAll(
            SetPsmValueArg4(&expectedValue),
            ::testing::Return(100)
        ));

    EXPECT_CALL(*g_bridgeUtilsGenericMock, HandlePreConfigVendorGeneric(_, _))
	 .Times(1)
        .WillOnce(Return(0));
    char event1[64] = {0} , value1[64] = {0};
    snprintf(event1,sizeof(event1),"multinet_%d-name",InstanceNumber);
    strcpy(value1, "brlan5");
    EXPECT_CALL(*g_syseventMock, sysevent_set(_, _, StrEq(event1), StrEq(value1), _))
	 .Times(1)
        .WillOnce(Return(0));
    char event2[64] = {0} , value2[64] = {0};
    snprintf(event2,sizeof(event2),"multinet_%d-vid",InstanceNumber);
    snprintf(value2,sizeof(value2),"%d",105);
    EXPECT_CALL(*g_syseventMock, sysevent_set(_, _, StrEq(event2), StrEq(value2), _))
	 .Times(1)
        .WillOnce(Return(0));
    Gateway_Config *pGwConfig = NULL;
    pGwConfig = (Gateway_Config*) malloc(sizeof(Gateway_Config));
    memset(pGwConfig, 0, sizeof(Gateway_Config));
    pGwConfig->if_type = OVS_OTHER_IF_TYPE;
    pGwConfig->mtu = DEFAULT_MTU;
    pGwConfig->vlan_id = DEFAULT_VLAN_ID;
    pGwConfig->if_cmd = OVS_IF_UP_CMD;
    char GreState[64] = {0} , greSysName[128] = {0};
    snprintf(greSysName,sizeof(greSysName),"if_%s-status","gre0");
    strcpy(GreState, "ready");
    EXPECT_CALL(*g_syseventMock, sysevent_get(_, _, StrEq(greSysName), _, _))
	 .Times(1)
        .WillOnce(::testing::DoAll(
            SetArgNPointeeTo<3>(std::begin(GreState), sizeof(GreState)),
            ::testing::Return(0)
        ));
    EXPECT_CALL(*g_ovsMock, ovs_agent_api_get_config( _, _))
        .Times(2)
        .WillOnce(::testing::DoAll(
            SetGwConfigArg1((void **)&pGwConfig),
            ::testing::Return(true)
        ))
        .WillOnce(::testing::DoAll(
            SetGwConfigArg1((void **)&pGwConfig),
            ::testing::Return(true)
        ));
    EXPECT_CALL(*g_ovsMock, ovs_agent_api_interact( _, _))
        .Times(2)
        .WillOnce(Return(true))
        .WillOnce(Return(true));

    EXPECT_CALL(*g_bridgeUtilsGenericMock, HandlePostConfigVendorGeneric(_, _))
	 .Times(1)
        .WillOnce(Return(0));
    char event21[64] = {0} , value21[64] = {0};
    snprintf(event21,sizeof(event21),"multinet_%d-localready",InstanceNumber);
    strcpy(value21, "1");
    EXPECT_CALL(*g_syseventMock, sysevent_set(_, _, StrEq(event21), StrEq(value21), _))
	 .Times(1)
        .WillOnce(Return(0));
    char event3[64] = {0} , value3[64] = {0};
    snprintf(event3,sizeof(event3),"multinet_%d-status",InstanceNumber);
    strcpy(value3, "ready");
    EXPECT_CALL(*g_syseventMock, sysevent_set(_, _, StrEq(event3), StrEq(value3), _))
	 .Times(1)
        .WillOnce(Return(0));
    char event4[64] = {0} ;
    snprintf(event4,sizeof(event4),"firewall-restart");
    EXPECT_CALL(*g_syseventMock, sysevent_set(_, _, StrEq(event4), _, _))
	 .Times(1)
        .WillOnce(Return(0));
    CreateBrInterface();
}

TEST_F(BridgeUtilsTestFixture, DeleteBrInterface)
{
    InstanceNumber = LOST_N_FOUND;
    BridgeOprInPropgress = DELETE_BRIDGE;
    ovsEnable = 1;
    DeviceMode = 0;
    ethWanEnabled = 0;
    wan_mode = 0;
    skipMoCA = 0;
    skipWiFi = 0;
    char event[64] = {0} , value[64] = {0};
    snprintf(event,sizeof(event),"multinet_%d-status",InstanceNumber);
    strcpy(value, "stopping");
    EXPECT_CALL(*g_syseventMock, sysevent_set(_, _, StrEq(event), StrEq(value), _))
	 .Times(1)
        .WillOnce(Return(0));
    char paramName1[128] = {0};
    snprintf(paramName1,sizeof(paramName1), "dmsb.l2net.%d.Name", InstanceNumber);
    char expectedValue1[128] = "br106";
    EXPECT_CALL(*g_psmMock, PSM_Get_Record_Value2( _, _, StrEq(paramName1), _, _))
        .Times(1)
        .WillOnce(::testing::DoAll(
            SetPsmValueArg4(&expectedValue1),
            ::testing::Return(100)
        ));

    char paramName2[128] = {0};
    snprintf(paramName2,sizeof(paramName2), "dmsb.l2net.%d.Vid", InstanceNumber);
    char expectedValue2[128] = "106";
    EXPECT_CALL(*g_psmMock, PSM_Get_Record_Value2( _, _, StrEq(paramName2), _, _))
        .Times(1)
        .WillOnce(::testing::DoAll(
            SetPsmValueArg4(&expectedValue2),
            ::testing::Return(100)
        ));

    char paramName3[128] = {0};
    snprintf(paramName3,sizeof(paramName3), "dmsb.l2net.%d.Members.Link", InstanceNumber);
    char expectedValue3[128] = "";
    EXPECT_CALL(*g_psmMock, PSM_Get_Record_Value2( _, _, StrEq(paramName3), _, _))
        .Times(1)
        .WillOnce(::testing::DoAll(
            SetPsmValueArg4(&expectedValue3),
            ::testing::Return(100)
        ));

    char paramName4[128] = {0};
    snprintf(paramName4,sizeof(paramName4), "dmsb.l2net.%d.Members.Eth", InstanceNumber);
    char expectedValue4[128] = "";
    EXPECT_CALL(*g_psmMock, PSM_Get_Record_Value2( _, _, StrEq(paramName4), _, _))
        .Times(1)
        .WillOnce(::testing::DoAll(
            SetPsmValueArg4(&expectedValue4),
            ::testing::Return(100)
        ));

    char paramName5[128] = {0};
    snprintf(paramName5,sizeof(paramName5), "dmsb.l2net.%d.Members.Moca", InstanceNumber);
    char expectedValue5[128] = "";
    EXPECT_CALL(*g_psmMock, PSM_Get_Record_Value2( _, _, StrEq(paramName5), _, _))
        .Times(1)
        .WillOnce(::testing::DoAll(
            SetPsmValueArg4(&expectedValue5),
            ::testing::Return(100)
        ));

    char paramName6[128] = {0};
    snprintf(paramName6,sizeof(paramName6), "dmsb.l2net.%d.Members.WiFi", InstanceNumber);
    char expectedValue6[128] = "wifi0 wifi1 wifi2 wifi3";
    EXPECT_CALL(*g_psmMock, PSM_Get_Record_Value2( _, _, StrEq(paramName6), _, _))
        .Times(1)
        .WillOnce(::testing::DoAll(
            SetPsmValueArg4(&expectedValue6),
            ::testing::Return(100)
        ));

    char paramName7[128] = {0};
    snprintf(paramName7,sizeof(paramName7), "dmsb.l2net.%d.Members.Gre", InstanceNumber);
    char expectedValue7[128] = "";
    EXPECT_CALL(*g_psmMock, PSM_Get_Record_Value2( _, _, StrEq(paramName7), _, _))
        .Times(1)
        .WillOnce(::testing::DoAll(
            SetPsmValueArg4(&expectedValue7),
            ::testing::Return(100)
        ));
    char expectedCmd[256] = {0};
    memset(expectedCmd,0,sizeof(expectedCmd));
    snprintf(expectedCmd,sizeof(expectedCmd),"ip link set %s down", expectedValue1);
    EXPECT_CALL(*g_utilMock, system(StrEq(expectedCmd)))
        .Times(1)
        .WillOnce(Return(1));
    EXPECT_CALL(*g_bridgeUtilsGenericMock, HandlePreConfigVendorGeneric(_, _))
	 .Times(1)
        .WillOnce(Return(0));
    Gateway_Config *pGwConfig = NULL;
    pGwConfig = (Gateway_Config*) malloc(sizeof(Gateway_Config));
    memset(pGwConfig, 0, sizeof(Gateway_Config));
    pGwConfig->if_type = OVS_OTHER_IF_TYPE;
    pGwConfig->mtu = DEFAULT_MTU;
    pGwConfig->vlan_id = DEFAULT_VLAN_ID;
    pGwConfig->if_cmd = OVS_IF_UP_CMD;

    EXPECT_CALL(*g_ovsMock, ovs_agent_api_get_config( _, _))
        .Times(1)
    	.WillOnce(::testing::DoAll(
            SetGwConfigArg1((void **)&pGwConfig),
            ::testing::Return(true)
        ));
    EXPECT_CALL(*g_ovsMock, ovs_agent_api_interact( _, _))
        .Times(1)
        .WillOnce(Return(true));

    EXPECT_CALL(*g_bridgeUtilsGenericMock, HandlePostConfigVendorGeneric(_, _))
	 .Times(1)
        .WillOnce(Return(0));

    char event21[64] = {0} , value21[64] = {0};
    snprintf(event21,sizeof(event21),"multinet_%d-localready",InstanceNumber);
    strcpy(value21, "0");
    EXPECT_CALL(*g_syseventMock, sysevent_set(_, _, StrEq(event21), StrEq(value21), _))
	 .Times(1)
        .WillOnce(Return(0));
    char event3[64] = {0} , value3[64] = {0};
    snprintf(event3,sizeof(event3),"multinet_%d-status",InstanceNumber);
    strcpy(value3, "stopped");
    EXPECT_CALL(*g_syseventMock, sysevent_set(_, _, StrEq(event3), StrEq(value3), _))
	 .Times(1)
        .WillOnce(Return(0));
    char event4[64] = {0} ;
    snprintf(event4,sizeof(event4),"firewall-restart");
    EXPECT_CALL(*g_syseventMock, sysevent_set(_, _, StrEq(event4), _, _))
	 .Times(1)
        .WillOnce(Return(0));
    DeleteBrInterface();
}

TEST_F(BridgeUtilsTestFixture, SyncBrInterfaces)
{
    InstanceNumber = HOME_SECURITY;
    BridgeOprInPropgress = CREATE_BRIDGE;
    ovsEnable = 1;
    DeviceMode = 0;
    ethWanEnabled = 0;
    wan_mode = 0;
    skipMoCA = 0;
    skipWiFi = 0;
    char event[64] = {0} , value[64] = {0};
    snprintf(event,sizeof(event),"multinet_%d-status",InstanceNumber);
    strcpy(value, "partial");
    EXPECT_CALL(*g_syseventMock, sysevent_set(_, _, StrEq(event), StrEq(value), _))
	 .Times(1)
        .WillOnce(Return(0));
    char paramName1[128] = {0};
    snprintf(paramName1,sizeof(paramName1), "dmsb.l2net.%d.Name", InstanceNumber);
    char expectedValue1[128] = "brlan1";
    EXPECT_CALL(*g_psmMock, PSM_Get_Record_Value2( _, _, StrEq(paramName1), _, _))
        .Times(1)
        .WillOnce(::testing::DoAll(
            SetPsmValueArg4(&expectedValue1),
            ::testing::Return(100)
        ));

    char paramName2[128] = {0};
    snprintf(paramName2,sizeof(paramName2), "dmsb.l2net.%d.Vid", InstanceNumber);
    char expectedValue2[128] = "101";
    EXPECT_CALL(*g_psmMock, PSM_Get_Record_Value2( _, _, StrEq(paramName2), _, _))
        .Times(1)
        .WillOnce(::testing::DoAll(
            SetPsmValueArg4(&expectedValue2),
            ::testing::Return(100)
        ));

    char paramName3[128] = {0};
    snprintf(paramName3,sizeof(paramName3), "dmsb.l2net.%d.Members.Link", InstanceNumber);
    char expectedValue3[128] = "";
    EXPECT_CALL(*g_psmMock, PSM_Get_Record_Value2( _, _, StrEq(paramName3), _, _))
        .Times(1)
        .WillOnce(::testing::DoAll(
            SetPsmValueArg4(&expectedValue3),
            ::testing::Return(100)
        ));

    char paramName4[128] = {0};
    snprintf(paramName4,sizeof(paramName4), "dmsb.l2net.%d.Members.Eth", InstanceNumber);
    char expectedValue4[128] = "";
    EXPECT_CALL(*g_psmMock, PSM_Get_Record_Value2( _, _, StrEq(paramName4), _, _))
        .Times(1)
        .WillOnce(::testing::DoAll(
            SetPsmValueArg4(&expectedValue4),
            ::testing::Return(100)
        ));

    char paramName5[128] = {0};
    snprintf(paramName5,sizeof(paramName5), "dmsb.l2net.%d.Members.Moca", InstanceNumber);
    char expectedValue5[128] = "";
    EXPECT_CALL(*g_psmMock, PSM_Get_Record_Value2( _, _, StrEq(paramName5), _, _))
        .Times(1)
        .WillOnce(::testing::DoAll(
            SetPsmValueArg4(&expectedValue5),
            ::testing::Return(100)
        ));

    char paramName6[128] = {0};
    snprintf(paramName6,sizeof(paramName6), "dmsb.l2net.%d.Members.WiFi", InstanceNumber);
    char expectedValue6[128] = "wifi0 wifi1";
    EXPECT_CALL(*g_psmMock, PSM_Get_Record_Value2( _, _, StrEq(paramName6), _, _))
        .Times(1)
        .WillOnce(::testing::DoAll(
            SetPsmValueArg4(&expectedValue6),
            ::testing::Return(100)
        ));

    char paramName7[128] = {0};
    snprintf(paramName7,sizeof(paramName7), "dmsb.l2net.%d.Members.Gre", InstanceNumber);
    char expectedValue7[128] = "";
    EXPECT_CALL(*g_psmMock, PSM_Get_Record_Value2( _, _, StrEq(paramName7), _, _))
        .Times(1)
        .WillOnce(::testing::DoAll(
            SetPsmValueArg4(&expectedValue7),
            ::testing::Return(100)
        ));

    char event1[64] = {0} , value1[64] = {0};
    snprintf(event1,sizeof(event1),"multinet_%d-name",InstanceNumber);
    strcpy(value1, "brlan1");
    EXPECT_CALL(*g_syseventMock, sysevent_set(_, _, StrEq(event1), StrEq(value1), _))
	 .Times(1)
        .WillOnce(Return(0));
    char event2[64] = {0} , value2[64] = {0};
    snprintf(event2,sizeof(event2),"multinet_%d-vid",InstanceNumber);
    snprintf(value2,sizeof(value2),"%d",101);
    EXPECT_CALL(*g_syseventMock, sysevent_set(_, _, StrEq(event2), StrEq(value2), _))
	 .Times(1)
        .WillOnce(Return(0));

    EXPECT_CALL(*g_bridgeUtilsGenericMock, HandlePreConfigVendorGeneric(_, _))
	 .Times(1)
        .WillOnce(Return(0));
    char expectedCmd[256] = {0};
    memset(expectedCmd,0,sizeof(expectedCmd));
    snprintf(expectedCmd,sizeof(expectedCmd),"ovs-vsctl list-ifaces %s | tr \"\n\" \" \" ",expectedValue1);
    char expectedIfList[] = "wifi0 wifi1";
    FILE * expectedFd = (FILE *)0xffffffff;
    EXPECT_CALL(*g_fileIOMock, popen(StrEq(expectedCmd), StrEq("r")))
       .Times(1)
       .WillOnce(::testing::Return(expectedFd));
    EXPECT_CALL(*g_fileIOMock, fgets(_, _, expectedFd))
        .Times(1)
        .WillOnce(::testing::DoAll(
            SetArgNPointeeTo<0>(std::begin(expectedIfList), sizeof(expectedIfList)),
            ::testing::Return((char*)expectedIfList)
        ));
    EXPECT_CALL(*g_fileIOMock, pclose(expectedFd))
       .Times(1)
       .WillOnce(::testing::Return(0));
    EXPECT_CALL(*g_bridgeUtilsGenericMock, HandlePostConfigVendorGeneric(_, _))
	 .Times(1)
        .WillOnce(Return(0));
        char event21[64] = {0} , value21[64] = {0};
    snprintf(event21,sizeof(event21),"multinet_%d-localready",InstanceNumber);
    strcpy(value21, "1");
    EXPECT_CALL(*g_syseventMock, sysevent_set(_, _, StrEq(event21), StrEq(value21), _))
	 .Times(1)
        .WillOnce(Return(0));
    char event3[64] = {0} , value3[64] = {0};
    snprintf(event3,sizeof(event3),"multinet_%d-status",InstanceNumber);
    strcpy(value3, "ready");
    EXPECT_CALL(*g_syseventMock, sysevent_set(_, _, StrEq(event3), StrEq(value3), _))
	 .Times(1)
        .WillOnce(Return(0));
    char event4[64] = {0} ;
    snprintf(event4,sizeof(event4),"firewall-restart");
    EXPECT_CALL(*g_syseventMock, sysevent_set(_, _, StrEq(event4), _, _))
	 .Times(1)
        .WillOnce(Return(0));
    SyncBrInterfaces();
}

TEST_F(BridgeUtilsTestFixture, SyncBrInterfacesBridgeMode)
{
    InstanceNumber = 1;
    BridgeOprInPropgress = CREATE_BRIDGE;
    ovsEnable = 1;
    DeviceMode = 1;
    ethWanEnabled = 1;
    wan_mode = 0;
    skipMoCA = 0;
    skipWiFi = 0;
    char event[64] = {0} , value[64] = {0};
    snprintf(event,sizeof(event),"multinet_%d-status",InstanceNumber);
    strcpy(value, "partial");
    EXPECT_CALL(*g_syseventMock, sysevent_set(_, _, StrEq(event), StrEq(value), _))
	 .Times(1)
        .WillOnce(Return(0));
    char paramName1[128] = {0};
    snprintf(paramName1,sizeof(paramName1), "dmsb.l2net.%d.Name", InstanceNumber);
    char expectedValue1[128] = "brlan0";
    EXPECT_CALL(*g_psmMock, PSM_Get_Record_Value2( _, _, StrEq(paramName1), _, _))
        .Times(1)
        .WillOnce(::testing::DoAll(
            SetPsmValueArg4(&expectedValue1),
            ::testing::Return(100)
        ));

    char paramName2[128] = {0};
    snprintf(paramName2,sizeof(paramName2), "dmsb.l2net.%d.Vid", InstanceNumber);
    char expectedValue2[128] = "100";
    EXPECT_CALL(*g_psmMock, PSM_Get_Record_Value2( _, _, StrEq(paramName2), _, _))
        .Times(1)
        .WillOnce(::testing::DoAll(
            SetPsmValueArg4(&expectedValue2),
            ::testing::Return(100)
        ));

    char paramName3[128] = {0};
    snprintf(paramName3,sizeof(paramName3), "dmsb.l2net.%d.Members.Link", InstanceNumber);
    char expectedValue3[128] = "llan0 lbr0 ler0";
    EXPECT_CALL(*g_psmMock, PSM_Get_Record_Value2( _, _, StrEq(paramName3), _, _))
        .Times(1)
        .WillOnce(::testing::DoAll(
            SetPsmValueArg4(&expectedValue3),
            ::testing::Return(100)
        ));

    char paramName4[128] = {0};
    snprintf(paramName4,sizeof(paramName4), "dmsb.l2net.%d.Members.Eth", InstanceNumber);
    char expectedValue4[128] = "";
    EXPECT_CALL(*g_psmMock, PSM_Get_Record_Value2( _, _, StrEq(paramName4), _, _))
        .Times(1)
        .WillOnce(::testing::DoAll(
            SetPsmValueArg4(&expectedValue4),
            ::testing::Return(100)
        ));

    char paramName5[128] = {0};
    snprintf(paramName5,sizeof(paramName5), "dmsb.l2net.%d.Members.Moca", InstanceNumber);
    char expectedValue5[128] = "nmoca0";
    EXPECT_CALL(*g_psmMock, PSM_Get_Record_Value2( _, _, StrEq(paramName5), _, _))
        .Times(1)
        .WillOnce(::testing::DoAll(
            SetPsmValueArg4(&expectedValue5),
            ::testing::Return(100)
        ));

    char paramName6[128] = {0};
    snprintf(paramName6,sizeof(paramName6), "dmsb.l2net.%d.Members.WiFi", InstanceNumber);
    char expectedValue6[128] = "wifi0 wifi1";
    EXPECT_CALL(*g_psmMock, PSM_Get_Record_Value2( _, _, StrEq(paramName6), _, _))
        .Times(1)
        .WillOnce(::testing::DoAll(
            SetPsmValueArg4(&expectedValue6),
            ::testing::Return(100)
        ));

    char paramName7[128] = {0};
    snprintf(paramName7,sizeof(paramName7), "dmsb.l2net.%d.Members.Gre", InstanceNumber);
    char expectedValue7[128] = "";
    EXPECT_CALL(*g_psmMock, PSM_Get_Record_Value2( _, _, StrEq(paramName7), _, _))
        .Times(1)
        .WillOnce(::testing::DoAll(
            SetPsmValueArg4(&expectedValue7),
            ::testing::Return(100)
        ));

    EXPECT_CALL(*g_fileIOMock, access(StrEq("/tmp/autowan_iface_finalized"), _))
	 .Times(1)
        .WillOnce(Return(1));

    char event1[64] = {0} , value1[64] = {0};
    snprintf(event1,sizeof(event1),"multinet_%d-name",InstanceNumber);
    strcpy(value1, "brlan0");
    EXPECT_CALL(*g_syseventMock, sysevent_set(_, _, StrEq(event1), StrEq(value1), _))
	 .Times(1)
        .WillOnce(Return(0));
    char event2[64] = {0} , value2[64] = {0};
    snprintf(event2,sizeof(event2),"multinet_%d-vid",InstanceNumber);
    snprintf(value2,sizeof(value2),"%d",100);
    EXPECT_CALL(*g_syseventMock, sysevent_set(_, _, StrEq(event2), StrEq(value2), _))
	 .Times(1)
        .WillOnce(Return(0));

    EXPECT_CALL(*g_bridgeUtilsGenericMock, HandlePreConfigVendorGeneric(_, _))
	 .Times(1)
        .WillOnce(Return(0));
    char expectedCmd[256] = {0};
    memset(expectedCmd,0,sizeof(expectedCmd));
    snprintf(expectedCmd,sizeof(expectedCmd),"ovs-vsctl list-ifaces %s | tr \"\n\" \" \" ",expectedValue1);
    char expectedIfList[] = "wifi0 wifi1 nmoca0";
    FILE * expectedFd = (FILE *)0xffffffff;
    EXPECT_CALL(*g_fileIOMock, popen(StrEq(expectedCmd), StrEq("r")))
       .Times(1)
       .WillOnce(::testing::Return(expectedFd));
    EXPECT_CALL(*g_fileIOMock, fgets(_, _, expectedFd))
        .Times(1)
        .WillOnce(::testing::DoAll(
            SetArgNPointeeTo<0>(std::begin(expectedIfList), sizeof(expectedIfList)),
            ::testing::Return((char*)expectedIfList)
        ));
    EXPECT_CALL(*g_fileIOMock, pclose(expectedFd))
       .Times(1)
       .WillOnce(::testing::Return(0));

    EXPECT_CALL(*g_bridgeUtilsGenericMock, HandlePostConfigVendorGeneric(_, _))
	 .Times(1)
        .WillOnce(Return(0));
    Gateway_Config *pGwConfig = NULL;
    pGwConfig = (Gateway_Config*) malloc(sizeof(Gateway_Config));
    memset(pGwConfig, 0, sizeof(Gateway_Config));
    pGwConfig->if_type = OVS_OTHER_IF_TYPE;
    pGwConfig->mtu = DEFAULT_MTU;
    pGwConfig->vlan_id = DEFAULT_VLAN_ID;
    pGwConfig->if_cmd = OVS_IF_UP_CMD;
    EXPECT_CALL(*g_ovsMock, ovs_agent_api_get_config( _, _))
        .Times(3)
        .WillOnce(::testing::DoAll(
            SetGwConfigArg1((void **)&pGwConfig),
            ::testing::Return(true)
        ))
        .WillOnce(::testing::DoAll(
            SetGwConfigArg1((void **)&pGwConfig),
            ::testing::Return(true)
        ))
        .WillOnce(::testing::DoAll(
            SetGwConfigArg1((void **)&pGwConfig),
            ::testing::Return(true)
        ));
    EXPECT_CALL(*g_ovsMock, ovs_agent_api_interact( _, _))
        .Times(3)
        .WillOnce(Return(true))
        .WillOnce(Return(true))
        .WillOnce(Return(true));
        char event21[64] = {0} , value21[64] = {0};
    snprintf(event21,sizeof(event21),"multinet_%d-localready",InstanceNumber);
    strcpy(value21, "1");
    EXPECT_CALL(*g_syseventMock, sysevent_set(_, _, StrEq(event21), StrEq(value21), _))
	 .Times(1)
        .WillOnce(Return(0));
    char event3[64] = {0} , value3[64] = {0};
    snprintf(event3,sizeof(event3),"multinet_%d-status",InstanceNumber);
    strcpy(value3, "ready");
    EXPECT_CALL(*g_syseventMock, sysevent_set(_, _, StrEq(event3), StrEq(value3), _))
	 .Times(1)
        .WillOnce(Return(0));
    char event4[64] = {0} ;
    snprintf(event4,sizeof(event4),"firewall-restart");
    EXPECT_CALL(*g_syseventMock, sysevent_set(_, _, StrEq(event4), _, _))
	 .Times(1)
        .WillOnce(Return(0));
    SyncBrInterfaces();
}
