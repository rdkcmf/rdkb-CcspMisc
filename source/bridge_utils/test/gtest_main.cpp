#include <string.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <gtest/gtest.h>
#include <gmock/gmock.h>

extern "C" {
   #include "bridge_util.h"
}

#define GTEST_DEFAULT_RESULT_FILEPATH "/tmp/Gtest_Report/"
#define GTEST_DEFAULT_RESULT_FILENAME "bridgeUtils_gtest_report.xml"
#define GTEST_REPORT_FILEPATH_SIZE 128

class BridgeUtilsEnvironment : public ::testing::Environment
{
    public:
        virtual ~BridgeUtilsEnvironment()
        {
        }
        virtual void SetUp()
        {
            printf("%s Environment\n", __func__);
        }
        virtual void TearDown()
        {
            printf("%s Environment\n", __func__);
        }
};

GTEST_API_ int main(int argc, char *argv[])
{
    char testresults_fullfilepath[GTEST_REPORT_FILEPATH_SIZE];
    char buffer[GTEST_REPORT_FILEPATH_SIZE];

    memset( testresults_fullfilepath, 0, GTEST_REPORT_FILEPATH_SIZE );
    memset( buffer, 0, GTEST_REPORT_FILEPATH_SIZE );

    snprintf( testresults_fullfilepath, GTEST_REPORT_FILEPATH_SIZE, "xml:%s%s" , GTEST_DEFAULT_RESULT_FILEPATH , GTEST_DEFAULT_RESULT_FILENAME);
    ::testing::GTEST_FLAG(output) = testresults_fullfilepath;
    ::testing::InitGoogleMock(&argc, argv);
    return RUN_ALL_TESTS();
}
