#include <gtest/gtest.h>
#include <gmock/gmock.h>
typedef unsigned int token_t;

class SyseventInterface {
public:
	virtual ~SyseventInterface() {}
	virtual int sysevent_open(char *, unsigned short, int, char *, token_t*) = 0;
	virtual int sysevent_close(const int, const token_t) = 0;
	virtual int sysevent_get(const int, const token_t, const char *, char *, int) =0;
	virtual int sysevent_set(const int, const token_t, const char *, const char *, int) = 0;
};

class SyseventMock : public SyseventInterface {
public:
	virtual ~SyseventMock() {}
	MOCK_METHOD5(sysevent_open, int(char *, unsigned short, int, char *, token_t*));
	MOCK_METHOD2(sysevent_close, int(const int, const token_t));
	MOCK_METHOD5(sysevent_get, int(const int, const token_t, const char *, char *, int));
	MOCK_METHOD5(sysevent_set, int(const int, const token_t, const char *, const char *, int));
};


