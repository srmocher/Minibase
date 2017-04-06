// -*- C++ -*-
/* This file declares TestDriver, a base class for various test driver
   objects. */
#ifndef _TEST_DRIVER_H_
#define _TEST_DRIVER_H_

#include "minirel.h"
//extern "C" int unlink( const char* );

class TestDriver
{
public:
      // This runs a series of tests.  If it returns OK, then everything worked
      // as advertised.  Otherwise, there were errors reported.
    virtual Status runTests();


protected:
      // This constructs the tester.  You supply a name like "dbtest" as the
      // root of the database and log file names.
    TestDriver( const char* nameRoot );
    virtual ~TestDriver();

    char* dbpath;
    char* logpath;

      // Subclasses override these tests.
    virtual int test1();
    virtual int test2();
    virtual int test3();
    virtual int test4();
    virtual int test5();
    virtual int test6();

      // ...and this method, which is printed as the kind of test being done,
      // for example "Disk Space Management".
    virtual const char* testName();

    void testFailure( Status& status, Status expectedStatus,
                      const char* activity, int postedErrExpected = TRUE );

    typedef int (TestDriver::*testFunction)();
    virtual void runTest( Status& status, testFunction test );

    virtual Status runAllTests();
};



#endif
