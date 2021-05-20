#!/bin/sh
source /etc/utopia/service.d/log_capture_path.sh
. /etc/device.properties
PORT2ENABLE=`syscfg get HomeSecurityEthernet4Flag`
BRIDGE_MODE=`syscfg get bridge_mode`
migrationCompleteFlag=0
MIGRATION_FILE="/nvram/.migration_to_psm_complete"
if [ "xcompleted" != "x`syscfg get psm_migration`" ];then
	if [ "$MODEL_NUM" = "CGM4140COM" ];then
		rm -rf "$MIGRATION_FILE"
		psmcli set dmsb.l2net.1.Members.SW ""
		psmcli set dmsb.l2net.9.Members.SW ""
		psmcli set dmsb.l2net.6.Members.WiFi "ath6 ath7 ath10 ath11"
		psmcli set dmsb.l2net.1.Members.Moca "moca0"
		psmcli set dmsb.l2net.9.Members.Moca "moca0"
		for i in 1 2 3 4 5 6 7 8 9
		do
			psmcli set dmsb.l2net."$i".Members.Link ""
			greIf=`psmcli get dmsb.l2net."$i".Members.Gre`
			if [ x"$greIf" != "x" ];then
				psmcli set dmsb.l2net."$i".Members.Gre `echo $greIf | cut -d "-" -f1`
			fi
		done
		if [ "$PORT2ENABLE" = "1" ];then
				psmcli set dmsb.l2net.1.Members.Eth "eth0"
				psmcli set dmsb.l2net.2.Members.Eth "eth1"
		else
				psmcli set dmsb.l2net.1.Members.Eth "eth0 eth1"
				psmcli set dmsb.l2net.2.Members.Eth ""
		fi 
		psmcli set dmsb.l2net.1.Port.9.LinkName ""
		psmcli set dmsb.l2net.1.Port.9.Name ""
		psmcli set dmsb.l2net.1.Port.8.LinkName "eth1"
		psmcli set dmsb.l2net.1.Port.8.Name "eth1"
		psmcli set dmsb.l2net.2.Port.2.Name "eth1"
		psmcli set dmsb.l2net.2.Port.2.LinkName "eth1"
		psmcli get dmsb.l2net.2.Members.SW ""
		
		migrationCompleteFlag=1
	fi
	if [ "$MODEL_NUM" = "TG3482G" ];then
		for i in 1 2 3 4 5 6 7 8 9
		do
			psmcli set dmsb.l2net."$i".Members.Link ""
			greIf=`psmcli get dmsb.l2net."$i".Members.Gre`
			if [ x"$greIf" != "x" ];then
				psmcli set dmsb.l2net."$i".Members.Gre `echo $greIf | cut -d "-" -f1`
			fi
		done
		if [ "$BRIDGE_MODE" -gt 0 ];then
			psmcli set dmsb.l2net.1.Members.Eth "llan0 lbr0"
		else
			psmcli set dmsb.l2net.1.Members.Eth "lbr0"
		fi
		psmcli set dmsb.l2net.1.Port.7.LinkName "llan0"
		psmcli set dmsb.l2net.2.Members.Moca ""
		psmcli set dmsb.l2net.3.Members.Moca ""
		psmcli set dmsb.l2net.4.Members.Moca ""
		psmcli set dmsb.l2net.9.Members.Moca "nmoca0"

		
		migrationCompleteFlag=1
	fi
	if [ "$migrationCompleteFlag" -eq 1 ];then
		syscfg set psm_migration "completed"
                syscfg commit
	fi
fi
#if device is FR in other builds which is not having bridgeUtil or OVS support, device will have wrong psm config. 
#need to correct psm config 
if [ "$migrationCompleteFlag" -eq 0 ];then
	if [ "$MODEL_NUM" = "CGM4140COM" ] || [ "$MODEL_NUM" = "TG3482G" ] ;then
		for i in 1 2
		do
			if [ "xl2sd0-t" = "x`psmcli get dmsb.l2net."$i".Members.Link`" ];then
				psmcli set dmsb.l2net."$i".Members.Link ""
			fi
		done
	fi
fi
