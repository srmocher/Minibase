#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <string.h>
#include <errno.h>
#include <iostream>
#include <assert.h>
#include <unistd.h>

#include "buf.h"
#include "db.h"
#include <pwd.h>


#include "BMTester.h"

//extern "C" int getpid();
//extern "C" int unlink( const char* );


BMTester::BMTester() : TestDriver( "buftest" )
{}


BMTester::~BMTester()
{}

//----------------------------------------------------
// test 1
//      Testing pinPage, unpinPage, and whether a dirty page 
//      is written to disk
//----------------------------------------------------

int BMTester::test1() 
{
	Status st;
	Page*	pg;
	int	first,last;
	char data[200]; 
	first = 5;
	last = first + NUMBUF + 5;

	cout << "--------------------- Test 1 ----------------------\n";
        st = OK;
	for (int i=first;i<=last;i++){
		if (MINIBASE_BM->pinPage(i,pg,0)!=OK)  {
       		        st = FAIL;
			MINIBASE_SHOW_ERRORS();
        	}
        	cout<<"after pinPage" << i <<endl;
		sprintf(data,"This is test 1 for page %d\n",i);
		strcpy((char*)pg,data);
		if (MINIBASE_BM->unpinPage(i,1,0)!=OK) {
            		st = FAIL;
			MINIBASE_SHOW_ERRORS();
        	}
        	cout<<"after unpinPage"<< i << endl;
    	}

	cout << "\n" << endl;

    	for (int i=first;i<=last;i++){
       		if (MINIBASE_BM->pinPage(i,pg,0)!=OK) {
            		st = FAIL;
			MINIBASE_SHOW_ERRORS();
        	}
        	cout<<"PAGE["<<i<<"]: "<<(char *)pg;
        	sprintf(data,"This is test 1 for page %d\n",i);
        	if (strcmp(data,(char*)pg)) {
            		st = FAIL;
            		cerr << "Error: page content incorrect!\n";
        	}
        	if (MINIBASE_BM->unpinPage(i,FALSE,FALSE)!=OK)  {
            		st = FAIL;
			MINIBASE_SHOW_ERRORS();
        	}
        }
	minibase_errors.clear_errors();
	return st == OK;
}


//-----------------------------------------------------------
// test 2
//      Testing love/hate replacement policy
//------------------------------------------------------------

int BMTester::test2()
{
    Status st;
    Page*   pg;
    int j,i;
    int frame[NUMBUF];  
    bool hated;


    cout << "--------------------- Test 2 ----------------------\n";
    st = OK;

    // Pin and unpin a series of pages, the first half are loved,
    // the latter half are hated.

    for (i=1;i<=NUMBUF;i++){
        if (MINIBASE_BM->pinPage(i+5,pg,0)!=OK) {
                st = FAIL;
	     	MINIBASE_SHOW_ERRORS();
        }
        j= pg - MINIBASE_BM->bufPool;
        frame[i-1] = j; 
    }

    for (i=1;i<=NUMBUF;i++){
	if (i <= (NUMBUF/2)) 
	  hated = FALSE;
	else 
	  hated = TRUE;
	 	  
	if (MINIBASE_BM->unpinPage(i+5,1,hated)!=OK) {
	  st = FAIL;
	  MINIBASE_SHOW_ERRORS();
	}
	
        cout << "Page " << i+5 <<" at frame "<< frame[i-1];
	if (hated)
	  cout << " is unpinned as hated." << endl;
	else 
	  cout << " is unpinned as loved." << endl;
    }

    // Now pin a new set of pages which should exactly 
    // replace the hated pages.  Check that this is the 
    // case, and that hated pages are replaced in MRU order
    // Unpin as loved so they don't interfere with our testing.

    cout << endl << "Testing replacement policy with respect to hated pages..." << endl;
    cout << "Pinning " << (NUMBUF - NUMBUF/2) << " unseen pages." << endl;

    for (i=2*NUMBUF; i>= NUMBUF + (NUMBUF/2 + 1); i--){
        if (MINIBASE_BM->pinPage(i+5,pg,0)!=OK) {
            	st = FAIL;
	     	MINIBASE_SHOW_ERRORS();
        }
        j= pg - MINIBASE_BM->bufPool;
        if (MINIBASE_BM->unpinPage(i+5,1,FALSE)!=OK) {
            st = FAIL;
            MINIBASE_SHOW_ERRORS();
        }
        cout << "Page " << i+5 <<" pinned in frame "<< j <<endl;
        if (j != frame[i-NUMBUF-1]) {
            st = FAIL;
            cerr<<"Error: frame number incorrect (under MRU for hated pages)!\n";
        }
    }

    // Now pin a new set of pages which should exactly 
    // replace the loved pages.  Check that this is the 
    // case, and that loved pages are replaced in LRU order.
    // Unpin as loved so they don't interfere with our testing.

    cout << endl << "Testing replacement policy with respect to loved pages..." << endl;
    cout << "Pinning an additional " << (NUMBUF/2) << " unseen pages." << endl;

    for (i=NUMBUF+1; i<=NUMBUF + NUMBUF/2; i++){
        if (MINIBASE_BM->pinPage(i+5,pg,0)!=OK) {
            	st = FAIL;
	     	MINIBASE_SHOW_ERRORS();
        }
        j= pg - MINIBASE_BM->bufPool;
        if (MINIBASE_BM->unpinPage(i+5,1,FALSE)!=OK) {
            st = FAIL;
            MINIBASE_SHOW_ERRORS();
        }
        cout << "Page " << i+5 <<" pinned in frame "<< j <<endl;
        if (j != frame[i-NUMBUF-1]) {
            st = FAIL;
            cerr<<"Error: frame number incorrect (under LRU for loved pages)!\n";
        }
    }

    minibase_errors.clear_errors();
    return st == OK;
}

//---------------------------------------------------------
// test 3
//      Testing  newPage,pinPage, freePage, error protocol
//---------------------------------------------------------


int BMTester::test3() 
{
  Status st;
  int pages[30];
  Page* pagesptrs[30];
  Page* pgptr;
  int i;
   
  cout << "--------------------- Test 3 ----------------------\n";
  st=OK;
  // Allocate 10 pages from database
  for ( i = 0; i < 10; i++)
    {
      if (MINIBASE_BM->newPage(pages[i], pagesptrs[i]) !=OK) {
        st = FAIL;
	cout << "\tnewPage failed...\n";
      }
    }
  

  // Pin first 10 pages a second time 
  for (i = 0; i < 10; i++)
    {
      cout << "Pinning page " << i << " " << pages[i] << endl;
      if (MINIBASE_BM->pinPage(pages[i], pgptr,0)!= OK) {
        st = FAIL;
	cout << "\tpinPage failed...\n";
      }
      if (pgptr != pagesptrs[i]) {
        st = FAIL;
	cout << "\tPinning error in a second time ...\n";
      }
    }


  // Try to free pinned pages
  for (i = 5; i < 10; i++)
    {
      cout << "Freeing page " << pages[i] << endl;
      if (MINIBASE_BM->freePage(pages[i]) == OK) {
        st = FAIL;
	cerr << "Error: pinned page freed!\n";
      }
    }

  // Now free page 0 thru 9 by first unpinning each page twice


  for (i = 0; i < 10; i++)
  {
   
    if (MINIBASE_BM->unpinPage(pages[i],FALSE,FALSE) !=OK) {
      st = FAIL;
      MINIBASE_SHOW_ERRORS();
    }
    if (MINIBASE_BM->unpinPage(pages[i],FALSE,FALSE) !=OK) {
      st = FAIL;
      MINIBASE_SHOW_ERRORS();
    }
    if (MINIBASE_BM->freePage(pages[i]) !=OK) {
      st = FAIL;
      MINIBASE_SHOW_ERRORS();
    }
    cout << "free  page " << pages[i] << endl;
  }

  // Get 14 more pages
  for (i = 10; i < 24; i++)
  {
    if(MINIBASE_BM->newPage(pages[i], pagesptrs[i])!=OK) {
      st = FAIL;
      MINIBASE_SHOW_ERRORS();
    }
     cout << "new  page " << i << "," << pages[i] << endl;
  }
  minibase_errors.clear_errors();
  return st == OK;
}




//-------------------------------------------------------------
// test 4
//	Test newPage,pinPage, unpinPage, and whether a dirty page 
//	is written to disk 
//-------------------------------------------------------------

int BMTester::test4(){
 Status st;
 int page;
 Page* pagePtr;
 char data[30] = "This page is dirty!\n";

  cout << "--------------------- Test 4 ----------------------\n";
  st=OK;
  if (MINIBASE_BM->newPage(page, pagePtr) != OK) {
        st = FAIL;
	cout << "\tERROR -- newPage failed\n";
  } else {
	cout << "\tnewPage successful\n";
	// dirty page
	sprintf((char*)pagePtr,"%s",data);
  }
  if (MINIBASE_BM->unpinPage(page,1,FALSE) != OK) {
    st = FAIL;
    cout << "\tERROR -- unpinning of page failed\n";
  } else
    cout << "\tUnpinning of page successful\n";

  delete MINIBASE_BM;
  MINIBASE_BM = new BufMgr(NUMBUF);
  if (MINIBASE_BM->pinPage(page, pagePtr,0) != OK) {
    st = FAIL;
    cout << "\tERROR -- pinning of page failed\n";
  }
  else
    cout << "\tPinning of page successful\n";
  if (strncmp(data,(char*)pagePtr,20) != 0) {
    st = FAIL;
    cout << "\tERROR -- Dirtied page not written to disk\n";
  }
  else
    cout << "\tDirtied page written to disk\n";

  minibase_errors.clear_errors();
  return st == OK;
}

//-------------------------------------------------------------
// test 5
//	Testing newPage, pinPage, unpinPage, freePage
//-------------------------------------------------------------

int BMTester::test5(){

  Status st, st2;
  int pages[30];
  Page* pagesptrs[30];
  int i;

  cout << "--------------------- Test 5 ----------------------\n";
  st2 = st = OK;

  // Allocate 10 pages
  for (i = 0; i < 10; i++) {
   if (MINIBASE_BM->newPage(pages[i], pagesptrs[i]) !=OK)
   {
     cout << "\tERROR -- newPage failed...\n";
     st = FAIL; 
   }
  }
  if ( st == OK)
     cout << "\tAllocate 10 pages successful\n";
  else st2 = st;

  // Try to unpin a pinned page twice
  if (MINIBASE_BM->unpinPage(pages[0],FALSE,FALSE) !=OK) {
    st2 = FAIL;
    cout << "\tERROR -- unpinning of a pinned page failed\n";
  }
  else cout << "\tUnpinning of a pinned page successful\n";

  // Try to unpin an unpinned page 
  if (MINIBASE_BM->unpinPage(pages[0],FALSE,FALSE) ==OK) {
    st2 = FAIL;
    cout << "\tERROR -- unpinning of an unpinned page succeeded\n";
  } else cout << "\tUnpinning of an unpinned page failed\n";

 // Pin a nonexistent page
  Page*	tmpPage;
  if (MINIBASE_BM->pinPage(999, tmpPage,0) == OK) {
    st2 = FAIL;
    cout << "\tERROR -- pinning of a non-existent page succeeded\n";
  } else  cout << "\tPinning of a non-existent page failed\n";

 // Unpin a nonexistent page
  if (MINIBASE_BM->unpinPage(999,FALSE,FALSE) == OK) { 
    st2 = FAIL;
    cout << "\tERROR -- unpinning of a non-existent page succeeded\n";
  } else cout << "\tUnpinning of a non-existent page failed\n";

 // Free a page that is still pinned
  if (MINIBASE_BM->pinPage(pages[0], pagesptrs[0],0) != OK) {
    st2 = FAIL;
    cout << "\tERROR -- pinning of page failed\n";
  } else cout << "\tPinning of page successful\n";
  if (MINIBASE_BM->freePage(pages[0]) == OK) {
    st2 = FAIL;
    cout << "\tERROR -- Freeing a pinned page succeeded \n";
  } else cout << "\tFreeing a pinned page failed \n";

  // Free all allocated pages
  st = OK;
  for (i = 0; i < 10; i++) {
	if (MINIBASE_BM->unpinPage(pages[i],FALSE,FALSE) != OK) st = FAIL;
	if (MINIBASE_BM->freePage(pages[i]) != OK) st = FAIL;
  }
  if (st == FAIL) {
	st2 = st;
	cout << "\tERROR -- Freeing allocated pages failed\n";
  } else cout << "\tFreeing allocated pages successful\n";

  delete MINIBASE_BM;
  MINIBASE_BM = new BufMgr(NUMBUF);
  int pages2[NUMBUF];
  Page* pagesptrs2[NUMBUF];

  // fill up buffer with pinned pages
  st = OK;
  for (i = 0; i < NUMBUF; i++)
	if (MINIBASE_BM->newPage(pages2[i], pagesptrs2[i]) !=OK) {
		cout << "\tERROR -- newPage failed\n";
     		st = FAIL;
   	}
  if (st == OK)
	cout << "\tAllocate page successful\n";
  else st2 = st;

  // Now try to pin one more page
  int	newPageNum;
  Page*	newPage;
   if (MINIBASE_BM->newPage(newPageNum, newPage ) ==OK) {
     st2 = FAIL;
     cout << "\tERROR -- Pinning a page in a full buffer succeeded\n";
   } else
     cout << "\tPinning a page in a full buffer failed\n";

  // Free all allocated pages
  st = OK;
  for (i = 0; i < NUMBUF; i++) {
	if (MINIBASE_BM->unpinPage(pages2[i],FALSE,FALSE) != OK) st = FAIL;
	if (MINIBASE_BM->freePage(pages2[i]) != OK) st = FAIL;
  }
  if (st == FAIL) {
	st2 = FAIL;
	cout << "\tERROR -- Freeing allocated pages failed\n";
  } else
	cout << "\tFreeing allocated pages successful\n";

  minibase_errors.clear_errors();
  return st2 == OK;
}

//----------------------------------------------------------
// Test 6
//      Testing pinPage, unpinPage, flushPage
//-----------------------------------------------------------

int BMTester::test6()
{
	Status st;
	Page*	pg;
	char data[30]; 
	int i;

	cout << "--------------------- Test 6 ----------------------\n";
        st = OK;
	for (i=1;i<=12;i++){
		if (MINIBASE_BM->pinPage(i,pg,0)!=OK)  {
       		        st = FAIL;
			MINIBASE_SHOW_ERRORS();
        	}
        	cout<<"after pinPage" << i <<endl;
		sprintf(data,"This is test 6 for page %d\n",i);
		strcpy((char*)pg,data);
		if (MINIBASE_BM->flushPage(i)!=OK) {
            		st = FAIL;
			MINIBASE_SHOW_ERRORS();
        	}
        	cout<<"after flushPage"<< i << endl;
		if (MINIBASE_BM->unpinPage(i,FALSE,FALSE)!=OK) {
			st = FAIL;
			cout<<"\tError----Unpin an unpinned page error!"<< endl;
		}
    	}

	cout << "\n" << endl;

    	for (i=1;i<=12;i++){
       		if (MINIBASE_BM->pinPage(i,pg,0)!=OK) {
            		st = FAIL;
			MINIBASE_SHOW_ERRORS();
        	}
        	cout<<"PAGE["<<i<<"]: "<<(char *)pg;
        	sprintf(data,"This is test 6 for page %d\n",i);
        	if (strcmp(data,(char*)pg)) {
            		st = FAIL;
            		cerr << "\tError: page content incorrect!\n";
        	}
        	if (MINIBASE_BM->unpinPage(i,FALSE,FALSE)!=OK)  {
            		st = FAIL;
			MINIBASE_SHOW_ERRORS();
        	}
        }

        minibase_errors.clear_errors();
	return st == OK;
}

const char* BMTester::testName()
{
    return "Buffer Management";
}


void BMTester::runTest( Status& status, TestDriver::testFunction test )
{
    minibase_globals = new SystemDefs( status, dbpath, logpath, 
				  NUMBUF+50, 500, NUMBUF, "Clock" );

    if ( status == OK )
      {
        TestDriver::runTest(status,test);
        delete minibase_globals; 
	minibase_globals = 0;
      }

    char* newdbpath;
    char* newlogpath;
    char remove_logcmd[50];
    char remove_dbcmd[50];

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

    delete newdbpath; delete newlogpath;

}


Status BMTester::runTests()
{
    return TestDriver::runTests();
}


Status BMTester::runAllTests()
{
    return TestDriver::runAllTests();
}
