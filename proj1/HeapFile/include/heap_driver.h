// -*- C++ -*-
#ifndef _HEAP_DRIVER_H_
#define _HEAP_DRIVER_H_

#include "test_driver.h"

class HeapDriver : public TestDriver
{
public:

    HeapDriver();
   ~HeapDriver();

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
