#ifndef MOCK_BRIDGEUTILS_GENERIC_H
#define MOCK_BRIDGEUTILS_GENERIC_H


#include <gtest/gtest.h>
#include <gmock/gmock.h>

class GenericInterface {
public:
	virtual ~GenericInterface() {}
	virtual int HandlePreConfigVendorGeneric(void *,int ) = 0;
	virtual int HandlePostConfigVendorGeneric(void *,int ) = 0;
};

class BridgeUtilsGenericMock: public GenericInterface {
public:
	virtual ~BridgeUtilsGenericMock() {}
	MOCK_METHOD2(HandlePreConfigVendorGeneric, int(void *, int));
	MOCK_METHOD2(HandlePostConfigVendorGeneric, int(void *, int));
};

#endif

