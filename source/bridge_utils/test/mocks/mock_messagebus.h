#ifndef MOCK_MESSAGEBUS_H
#define MOCK_MESSAGEBUS_H


#include <gtest/gtest.h>
#include <gmock/gmock.h>

typedef void*(*CCSP_MESSAGE_BUS_MALLOC) ( size_t size ); // this signature is different from standard malloc
typedef void (*CCSP_MESSAGE_BUS_FREE)   ( void * ptr );

class MessageBusInterface {
public:
	virtual ~MessageBusInterface() {}
	virtual int CCSP_Message_Bus_Init(char *, char *, void **, CCSP_MESSAGE_BUS_MALLOC, CCSP_MESSAGE_BUS_FREE) = 0;
};

class MessageBusMock: public MessageBusInterface {
public:
	virtual ~MessageBusMock() {}
	MOCK_METHOD5(CCSP_Message_Bus_Init, int(char *, char *, void **, CCSP_MESSAGE_BUS_MALLOC, CCSP_MESSAGE_BUS_FREE));
};

#endif