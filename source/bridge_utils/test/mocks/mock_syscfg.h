#ifndef MOCK_SYSCFG_H
#define MOCK_SYSCFG_H


#include <gtest/gtest.h>
#include <gmock/gmock.h>

class SyscfgInterface {
public:
	virtual ~SyscfgInterface() {}
	virtual int syscfg_init() = 0;
	virtual int syscfg_get(const char *, const char *, char *, int) = 0;
};

class SyscfgMock: public SyscfgInterface {
public:
	virtual ~SyscfgMock() {}
	MOCK_METHOD0(syscfg_init, int(void));
	MOCK_METHOD4(syscfg_get, int(const char *, const char *, char *, int));
};

#endif




