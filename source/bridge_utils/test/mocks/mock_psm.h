#include <gtest/gtest.h>
#include <gmock/gmock.h>


class PsmInterface {
public:
	virtual ~PsmInterface() {}
	virtual int PSM_Get_Record_Value2(void*, char const * const, char const * const, unsigned int *, char**) = 0;
};

class PsmMock : public PsmInterface {
public:
	virtual ~PsmMock() {}
	MOCK_METHOD5(PSM_Get_Record_Value2, int(void*, char const * const, char const * const, unsigned int *, char**));
};
