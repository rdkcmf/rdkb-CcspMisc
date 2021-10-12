#ifndef MOCK_UTIL_H
#define MOCK_UTIL_H


#include <gtest/gtest.h>
#include <gmock/gmock.h>

class UtilInterface {
public:
	virtual ~UtilInterface() {}
	virtual int system(const char *) = 0;
};

class UtilMock: public UtilInterface {
public:
	virtual ~UtilMock() {}
	MOCK_METHOD1(system, int(const char *));
};

#endif




