// -*- C++ -*-
#ifndef _SMJTESTER_H_
#define _SMJTESTER_H_

#include "test_driver.h"


class SMJTester : public TestDriver
{
public:
      // This constructs the tester.  You then test it by calling runTests().
    SMJTester();
   ~SMJTester();

    Status runTests();

private:
    int test1();
    int test2();
    int test3();
    int test4();
    int test5();
    int test6();
    const char* testName();
    Status runAllTests();
};


#endif
