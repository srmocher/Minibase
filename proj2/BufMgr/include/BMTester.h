// -*- C++ -*-
#ifndef _BMTESTER_H_
#define _BMTESTER_H_

#include "test_driver.h"


class BMTester : public TestDriver
{
public:
      // This constructs the tester.  You then test it by calling runTests().
    BMTester();
   ~BMTester();

    Status runTests();

private:
    int test1();
    int test2();
    int test3();
    int test4();
    int test5();
    int test6();
    const char* testName();
    void runTest( Status& status, testFunction test );
    Status runAllTests();
};


#endif
