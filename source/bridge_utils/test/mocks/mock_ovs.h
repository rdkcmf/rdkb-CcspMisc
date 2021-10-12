#ifndef MOCK_OVS_H
#define MOCK_OVS_H


#include <gtest/gtest.h>
#include <gmock/gmock.h>
extern "C"
{
#include "OvsAgentApi.h"
}

class OvsInterface {
public:
	virtual ~OvsInterface() {}
	virtual bool ovs_agent_api_get_config(OVS_TABLE , void ** ) = 0;
	virtual bool ovs_agent_api_interact(ovs_interact_request * , ovs_interact_cb ) = 0;
};

class OvsMock: public OvsInterface {
public:
	virtual ~OvsMock() {}
	MOCK_METHOD2(ovs_agent_api_get_config, bool(OVS_TABLE , void ** ));
	MOCK_METHOD2(ovs_agent_api_interact, bool(ovs_interact_request * , ovs_interact_cb ));
};

#endif




