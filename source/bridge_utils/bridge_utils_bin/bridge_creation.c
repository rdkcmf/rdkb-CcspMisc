/*
 * If not stated otherwise in this file or this component's Licenses.txt file the
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

#include "bridge_creation.h"

/*********************************************************************************************

    caller:  CreateBrInterface,DeleteBrInterface,SyncBrInterfaces
    prototype:

        bool
        brctl_interact
            (
				ovs_api_request *request
            );
    	description :
			When OVS is disabled , this function is called to create/delete/update 
			bridge and bridge interfaces

	Argument : 
			ovs_api_request *request  -- It has bridge and iface related data
	return : When success returns true 
***********************************************************************************************/

bool brctl_interact(ovs_interact_request *request)
{
	char cmd[1024] = {0} ;

	Gateway_Config * gw_config = (Gateway_Config *) ((request->table_config.config) );
#if 0
	if ( OVS_INSERT_OPERATION == request->operation )
	{

		if ( gw_config->parent_bridge[0] != '\0' )
		{
			snprintf(cmd,sizeof(cmd),"check_bridge=`brctl show | grep %s` ; if [ \"$check_bridge\" = \"\" ]; \
				then \
				brctl addbr %s ; \
				ifconfig %s up ; \
				fi ;",
				gw_config->parent_bridge,
				gw_config->parent_bridge,
				gw_config->parent_bridge);
				system(cmd);
				memset(cmd,0,sizeof(cmd));

		}

		if ( ( gw_config->if_type == OVS_VLAN_IF_TYPE ) || ( gw_config->if_type == OVS_GRE_IF_TYPE ) )
		{
			snprintf(cmd,sizeof(cmd),"ifconfig %s up;\
				vconfig add %s %d ;\
				ifconfig %s.%d up ;\
				brctl addif %s %s.%d",
				gw_config->parent_ifname,
				gw_config->parent_ifname,
				gw_config->vlan_id,
				gw_config->parent_ifname,
				gw_config->vlan_id,
				gw_config->parent_bridge,
				gw_config->parent_ifname,
				gw_config->vlan_id);
				system(cmd);
				memset(cmd,0,sizeof(cmd));

		}
				
		else
		{
			snprintf(cmd,sizeof(cmd),"brctl addif %s %s ;",gw_config->parent_bridge,gw_config->if_name);
			system(cmd);
		}

		}
#endif
		if ( OVS_IF_DELETE_CMD == gw_config->if_cmd )
		{

			if ( gw_config->if_type == OVS_BRIDGE_IF_TYPE )
			{
				snprintf(cmd,sizeof(cmd),"for iface in `brctl show %s | sed '1d' | awk '{print $NF}'` ;\
					do \
					brctl delif %s $iface; \
					done ; \
					ifconfig %s down ; \
					brctl delbr %s ;",
					gw_config->if_name,
					gw_config->if_name,
					gw_config->if_name,
					gw_config->if_name);
				system(cmd);		
			}

			else if ( ( gw_config->if_type == OVS_VLAN_IF_TYPE ) || ( gw_config->if_type == OVS_GRE_IF_TYPE ) )
			{
				snprintf(cmd,sizeof(cmd),"brctl delif %s %s.%d ;\
					vconfig rem %s.%d ;",
					gw_config->parent_bridge,
					gw_config->parent_ifname,
					gw_config->vlan_id,
					gw_config->parent_ifname,
					gw_config->vlan_id);
				system(cmd);		
			}

			else 
			{
				snprintf(cmd,sizeof(cmd),"brctl delif %s %s",gw_config->parent_bridge,gw_config->if_name);
				system(cmd);		
			}


		}
		else if ( OVS_BR_REMOVE_CMD == gw_config->if_cmd )
		{
				if ( ( gw_config->if_type == OVS_VLAN_IF_TYPE ) || ( gw_config->if_type == OVS_GRE_IF_TYPE ) )
				{
					snprintf(cmd,sizeof(cmd),"brctl delif %s %s ;\
						vconfig rem %s.%d ;",
						gw_config->parent_bridge,
						gw_config->parent_ifname,
						gw_config->parent_ifname,
						gw_config->vlan_id);
					system(cmd);		
				}

				else 
				{
					snprintf(cmd,sizeof(cmd),"brctl delif %s %s",gw_config->parent_bridge,gw_config->if_name);
					system(cmd);		
				}
		}
		else if ( OVS_IF_DOWN_CMD == gw_config->if_cmd )
		{
					if ( gw_config->if_name[0] != '\0' )
					{
						snprintf(cmd,sizeof(cmd),"ifconfig %s down",gw_config->if_name);
						system(cmd);
					}
		}
		else if ( OVS_IF_UP_CMD == gw_config->if_cmd )
		{
			if ( gw_config->parent_bridge[0] != '\0' )
			{
				snprintf(cmd,sizeof(cmd),"check_bridge=`brctl show %s` ;\
					if [ \"$check_bridge\" = \"\" ];\
					then \
						brctl addbr %s ;\
						ifconfig %s up ; \
					fi ;",
					gw_config->parent_bridge,
					gw_config->parent_bridge,
					gw_config->parent_bridge);
					system(cmd);
					memset(cmd,0,sizeof(cmd));
			}

			if ( ( gw_config->if_type == OVS_VLAN_IF_TYPE ) || ( gw_config->if_type == OVS_GRE_IF_TYPE ) )
			{
				snprintf(cmd,sizeof(cmd),"for bridge in `brctl show | cut -f1 | awk 'NF > 0' | sed '1d' | grep -v %s `;\
					do \
					check_if_attached=`brctl show $bridge | grep \"%s.%d\"` ; \
					if [ \"$check_if_attached\" != \"\" ] ;\
					then\
						echo \"deleting %s.%d from $bridge\" ;\
					        brctl delif $bridge %s.%d ; \
					fi ;\
					done ;\
					check_if_exist=`brctl show %s | grep \"%s.%d\"` ; \
					if [ \"$check_if_exist\" = \"\" ]; \
					then \
						ifconfig %s up;\
						vconfig add %s %d ;\
						ifconfig %s.%d up ;\
						ifconfig %s up ;\
						brctl addif %s %s.%d ;\
					fi ;",
					gw_config->parent_bridge,
					gw_config->if_name,
					gw_config->vlan_id,
					gw_config->if_name,
					gw_config->vlan_id,
					gw_config->if_name,
					gw_config->vlan_id,
					gw_config->parent_bridge,
					gw_config->if_name,
					gw_config->vlan_id,
					gw_config->parent_ifname,
					gw_config->parent_ifname,
					gw_config->vlan_id,
					gw_config->parent_ifname,
					gw_config->vlan_id,
					gw_config->parent_bridge,	
					gw_config->parent_bridge,
					gw_config->parent_ifname,
					gw_config->vlan_id);	
			}

			else
			{
				snprintf(cmd,sizeof(cmd),"for bridge in `brctl show | cut -f1 | awk 'NF > 0' | sed '1d' | grep -v %s `;\
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
					 fi ;",
					 gw_config->parent_bridge,
					 gw_config->if_name,
					 gw_config->if_name,
					 gw_config->if_name,
					 gw_config->if_name,
					 gw_config->parent_bridge,
					 gw_config->if_name,
					 gw_config->if_name,
					 gw_config->parent_bridge,
					 gw_config->parent_bridge,
					 gw_config->if_name);	
				}

				system(cmd);

		}

		if (gw_config != NULL )
		{
			free(gw_config);
			gw_config = NULL;
		}
	return true ;
}
