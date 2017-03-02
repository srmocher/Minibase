// -*- C++ -*-
#ifndef _HFP_DRIVER_H_
#define _HFP_DRIVER_H_

#include "test_driver.h"

class HfpDriver : public TestDriver
{
public:

    HfpDriver();
    ~HfpDriver();

    Status runTests();

private:
    int choice;

    int test1();
    int test2();
    int test3();
    int test4();
    int test5();
    int test6();

    Status runAllTests();
    const char* testName();
};

#endif
