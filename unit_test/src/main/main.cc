#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <errno.h>
#include <stdexcept>

#include "loghelper.h"
#include "gtest/gtest.h"

using ::testing::EmptyTestEventListener;
using ::testing::InitGoogleTest;
using ::testing::Test;
using ::testing::TestCase;
using ::testing::TestEventListeners;
using ::testing::TestInfo;
using ::testing::TestPartResult;
using ::testing::UnitTest;

static const int max_argc = 16;
static const int argv_buf_len = 128;
int g_argc = 0;
char *g_argv[max_argc];

void sig_ignore(int n) {
    //fprintf(stdout, "%s, signal[%d]\n", __FUNCTION__, n);
}
void sig_exit(int n) {
    fprintf(stdout, "%s, signal[%d]\n", __FUNCTION__, n);
    exit(0);
}

int main(int argc, char **argv)
{
    fprintf(stdout, "---- __cplusplus[%ld]----- \n", __cplusplus);
    #ifdef __GNUC__
    fprintf(stdout, "---- GNU[%d]----- \n", __GNUC__);
    #endif

    //SET_LOG_FILE("./log/all.log");
    SET_LOG_LEVEL("all");

    bcus::signal(SIGPIPE, sig_ignore);
    bcus::signal(SIGINT,  sig_exit);
    bcus::signal(SIGBUS, sig_ignore);

    if(argc > max_argc) {
        throw std::runtime_error("too much options");
    }
    g_argc = argc;
    for(int i = 0; i < argc; i++) {
        g_argv[i] = new char[argv_buf_len];
        memcpy(*(g_argv+i), *(argv+i), argv_buf_len);
    }

    int ret  =  0;
    try {
        InitGoogleTest(&argc, argv);
        ret = RUN_ALL_TESTS();
    } catch(std::runtime_error e) {
        fprintf(stdout, "exception[%s]\n", e.what());
    }

#ifdef WIN32
    system("pause");
#endif

    return ret;

}




