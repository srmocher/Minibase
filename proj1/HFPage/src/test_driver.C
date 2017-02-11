#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <string.h>
#include <errno.h>
#include <iostream>
#include <assert.h>

#include "new_error.h"
#include "test_driver.h"
#include <pwd.h>
#include <unistd.h>

using namespace std;

//<<<<<<< test_driver.C
//=======
//extern "C" int getpid();
//>>>>>>> 1.5


TestDriver::TestDriver( const char* nameRoot )
{
    unsigned len = strlen(nameRoot);
    char basename[len+20];
    char dbfname[len+20];
    char logfname[len+20];

    sprintf( basename, "%s%d", nameRoot, getpid() );
    sprintf( dbfname, "/tmp/%s.minibase-db", basename );
    sprintf( logfname, "/tmp/%s.minibase-log", basename );

    dbpath = strdup(dbfname);
    logpath = strdup(logfname);
}


TestDriver::~TestDriver()
{
    ::free(dbpath);
    ::free(logpath);
}


int TestDriver::test1()
{
    return TRUE;
}

int TestDriver::test2()
{
    return TRUE;
}

int TestDriver::test3()
{
    return TRUE;
}

int TestDriver::test4()
{
    return TRUE;
}

int TestDriver::test5()
{
    return TRUE;
}

int TestDriver::test6()
{
    return TRUE;
}


const char* TestDriver::testName()
{
    return "*** unknown ***";   // A little reminder to subclassers.
}


void TestDriver::testFailure( Status& status, Status expectedStatus,
                              const char* activity, int postedErrExpected )
{
    if ( status == OK )
      {
        status = FAIL;
        cerr << "*** " << activity << " did not return a failure status.\n";
      }
    else if ( status != expectedStatus )
        cerr << "*** " << activity << " correctly returned a failure status,\n"
             << "    but not the expected one.\n";
    else if ( postedErrExpected && !minibase_errors.error() )
        cerr << "*** " << activity << " correctly returned a failure status,\n"
             << "    but did not log the error.\n";
    else if ( !postedErrExpected && minibase_errors.error() )
        cerr << "*** " << activity << " correctly returned a failure status,\n"
             << "    but unexpectedly logged the error.\n";
    else
      {
        status = OK;
        cout << "    --> Failed as expected\n";
      }

    if ( status != OK )
        minibase_errors.show_errors();
    minibase_errors.clear_errors();
}



void TestDriver::runTest( Status& status, TestDriver::testFunction test )
{
    minibase_errors.clear_errors();
    int result = (this->*test)();
    if ( !result || minibase_errors.error() )
      {
        status = FAIL;
        if ( minibase_errors.error() )
            cerr << (result? "*** Unexpected error(s) logged, test failed:\n"
                     : "Errors logged:\n");
        minibase_errors.show_errors(cerr);
      }
}


Status TestDriver::runTests()
{
    cout << "\nRunning " << testName() << " tests...\n";


    // Kill anything that might be hanging around.

    char* newdbpath;
    char* newlogpath;
   
	char remove_logcmd[100];
	char remove_dbcmd[100];

    newdbpath = new char[ strlen(dbpath) + 20];
    newlogpath = new char[ strlen(logpath) + 20];
    strcpy(newdbpath,dbpath); 
    strcpy(newlogpath, logpath);

	sprintf(remove_logcmd, "/bin/rm -rf %s", logpath);
	sprintf(remove_dbcmd, "/bin/rm -rf %s", dbpath);
	
	system(remove_logcmd);
	system(remove_dbcmd);

  sprintf(newdbpath, "%s", dbpath);
  sprintf(newlogpath, "%s", logpath);


    unlink( newdbpath );
    unlink( newlogpath );

    minibase_errors.clear_errors();


      // Run the tests.
    Status answer = runAllTests();


      // Clean up.
    unlink( newdbpath );
    unlink( newlogpath );
    minibase_errors.clear_errors();

    cout << "\n..." << testName() << " tests "
         << (answer == OK ? "completed successfully" : "failed")
         << ".\n\n";

    delete newdbpath; delete newlogpath;

    return answer;
}


Status TestDriver::runAllTests()
{
    Status answer = OK;
    runTest( answer, &TestDriver::test1 );
    runTest( answer, &TestDriver::test2 );
    runTest( answer, &TestDriver::test3 );
    runTest( answer, &TestDriver::test4 );
    runTest( answer, &TestDriver::test5 );
    runTest( answer, &TestDriver::test6 );
    return answer;
}
