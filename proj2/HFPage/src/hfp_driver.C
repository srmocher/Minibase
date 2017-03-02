//*****************************************
//  Driver program for heapfiles
//****************************************

using namespace std;

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <iostream>
#include <string.h>
#include <stdlib.h>
#include <assert.h>

#include "db.h"
#include "scan.h"
#include "hfp_driver.h"
#include "buf.h"
#include "hfpage.h"


static const int namelen = 24;
struct Rec
{
    int ival;
    float fval;
    char name[namelen];
};

static const int reclen = sizeof(Rec);



HfpDriver::HfpDriver() : TestDriver( "hf-johan" )
{
    choice = 100;       // big enough for file to occupy >1 page
}

HfpDriver::~HfpDriver()
{}


Status HfpDriver::runTests()
{
    return TestDriver::runTests();
}

Status HfpDriver::runAllTests()
{
    Status answer;
    minibase_globals = new SystemDefs(answer,dbpath,logpath,
				      100,500,100,"Clock");
    if ( answer == OK )
        answer = TestDriver::runAllTests();

    delete minibase_globals;
    return answer;
}

const char* HfpDriver::testName()
{
    return "HFPage Tests";
}


//********************************************

int HfpDriver::test1()
{   HFPage hfp;
    Status status = OK;
    PageId tmpPID = 7; 

    cout << "\n  Test 1: Page Initialization Checks\n";
    hfp.init(tmpPID);
    tmpPID=8; hfp.setNextPage(tmpPID);
    tmpPID=INVALID_PAGE; hfp.setPrevPage(tmpPID);
    
    cout <<"Current Page No.: " <<hfp.page_no() <<", "
         <<"Next Page No.: " <<hfp.getNextPage() <<", "
         <<"Prev Page No.: " <<hfp.getPrevPage() <<", "
         <<"Available Space: " <<hfp.available_space() <<endl;
    if (hfp.empty() != true) {
        cout <<"ERROR: page should be empty.\n";
        status = FAIL;
    } else 
        cout << "Page Empty as expected.\n";
    hfp.dumpPage();
    return (status == OK);
}


//***************************************************

int HfpDriver::test2()
{   const int BuffSize = 20;
    const int Limit    = 20;
    HFPage hfp;
    PageId tmpPID = 7; 
    Status status = OK;
    RID Rid, CurRid;
    int i, size; 
    char TmpBuf[BuffSize];

    hfp.init(tmpPID);
    tmpPID=8; hfp.setNextPage(tmpPID);
    tmpPID=INVALID_PAGE; hfp.setPrevPage(tmpPID);

    cout << "\n  Test 2: Insert and traversal of records\n";
    for (i=0; i <Limit; i++)
    { status = hfp.insertRecord ((char*) &i, sizeof(int), Rid);
      if (status != OK) { 
        cout << "ERROR: Insertion Failed!" << endl;
    	cout <<"    Current Page No.: " <<hfp.page_no() <<", "
             <<"Available Space: " <<hfp.available_space() <<endl;  
      } else {
         cout << "Inserted record, RID " << Rid.pageNo << ", "
              << Rid.slotNo << endl;
      }
    }
    if (hfp.empty() == true) cout <<"ERROR: The page cannot be empty!\n";
    status = hfp.firstRecord(Rid);
    while (status == OK)
    { status = hfp.getRecord(Rid, TmpBuf, size); 
      if (status != OK) { 
         cout << "ERROR: getting record.\n"; 
         break; 
      } else {
         cout << "Retrieved record, RID " << Rid.pageNo << ", "
              << Rid.slotNo << endl;
      }
      if (size != sizeof(int)) 
          cout << "ERROR: Incorrect size reported.\n"; 
      CurRid= Rid;
      status=hfp.nextRecord(CurRid, Rid);
    }
    if ( status != DONE ) 
       cout << "ERROR: failed to traverse all records.\n";
    else status = OK;
    return (status == OK);
}


//********************************************************

int HfpDriver::test3()
{   const int BuffSize = 20;
    const int Limit    = 20;
    HFPage hfp;
    PageId tmpPID = 7; 
    Status status = OK;
    RID Rid, CurRid, DelList[Limit];
    int i, size, deleteListSize=0; 
    char TmpBuf[BuffSize]; 

    hfp.init(tmpPID);
    tmpPID=8; hfp.setNextPage(tmpPID);
    tmpPID=INVALID_PAGE; hfp.setPrevPage(tmpPID);

    cout << "\n  Test 3: Insert and Delete fixed-sizerecords\n";

    for (i=0; i < Limit; i++)
    { status = hfp.insertRecord ((char*) &i, sizeof(int), Rid);
      if (status != OK) {
        cout << "ERROR: Insertion Failed!" << endl;
        cout <<"    Current Page No.: " <<hfp.page_no() <<", "
             <<"Available Space: " <<hfp.available_space() <<endl;
      } else {
         cout << "Inserted record, RID " << Rid.pageNo << ", "
              << Rid.slotNo << endl;
      }
    }
    
    if (hfp.empty() == true) 
        cout <<"ERROR: The page cannot be empty!\n";

    status = hfp.firstRecord(Rid); i=0;
    while (status == OK)
    { status = hfp.getRecord(Rid, TmpBuf, size); 
      if (status != OK) { cout << "ERROR: getting record.\n"; break; }
      if (size != sizeof(int)) 
         cout << "ERROR: incorrect size!\n"; 
      CurRid=Rid; 
      if ((i%4 == 0)&&(4*i<3*Limit)) { 
         DelList[deleteListSize]=Rid; deleteListSize++; 
      } 
      //if compaction is taking place, we shall see.
      status=hfp.nextRecord(CurRid, Rid);
      i++;
    } 
    if ( status != DONE ) 
        cout << "ERROR: failed to traverse all records.\n";
    
    DelList[deleteListSize]=CurRid; 
    deleteListSize++; //picks up the last one;

    cout << "Deleting Records"<<endl;
    for (i=0; i<deleteListSize; i++) 
    { Rid=DelList[i]; 
      cout<<"Deleting Rid("<<Rid.pageNo<<","<< Rid.slotNo<<").\n";
      status = hfp.deleteRecord(Rid);
      if (status != OK) { 
        cout << "ERROR: Deletion of Rid(" << Rid.pageNo << "," << 
        Rid.slotNo<<") failed\n";
      }
    }
    
    cout << "After Deletion.\n";
    status = hfp.firstRecord(Rid); 
    while (status == OK)
    { 
      cout << "Retrieving record " << Rid.pageNo << ", " 
           << Rid.slotNo << endl;
      status = hfp.getRecord(Rid, TmpBuf, size); 
      if (status != OK) { 
         cout << "ERROR: getting record.\n"; 
         break; 
      }
      if (size != sizeof(int)) 
         cout << "ERROR: incorrect size \n"; 
      CurRid=Rid; 
      status=hfp.nextRecord(CurRid, Rid);
    } 
    if ( status != DONE ) 
        cout << "ERROR: failed to traverse all records.\n";
    else status=OK; 
    return (status == OK);
}


//*****************************************************************

int HfpDriver::test4()
{   const int BuffSize = 50;
    const int Limit    = 20;
    HFPage hfp;
    PageId tmpPID = 7; 
    Status status = OK;
    RID Rid, CurRid, DelList[Limit];
    int i, j, size; 
    char TmpBuf1[BuffSize] = "Alphabet:\0"; 
    char TmpBuf2[BuffSize] = "Junk:\0"; 
    char Letter[BuffSize]="A\0"; char* temp; 

    hfp.init(tmpPID);
    tmpPID=8; hfp.setNextPage(tmpPID);
    tmpPID=INVALID_PAGE; hfp.setPrevPage(tmpPID);

    cout << "\n  Test 4: Insert and Delete variable-length records\n";

    for (i=0; i < Limit; i++)
    { Letter[0]=(char)(i+65);

      if (i%2 == 0)
      { strcat(TmpBuf1, Letter); size = strlen(TmpBuf1);
	status = hfp.insertRecord (TmpBuf1, size+1, Rid); }
      else 
      { strcat(TmpBuf2, Letter); size = strlen(TmpBuf2);
        status = hfp.insertRecord (TmpBuf2, size+1, Rid); }

      if (status != OK) 
      { cout << "ERROR: Insertion Failed!" << endl;
    	cout <<"   Current Page No.: " <<hfp.page_no() <<", "
             <<"Available Space: " <<hfp.available_space() <<endl;  
      }
    }
    
    if (hfp.empty() == true) 
        cout << "ERROR: The page cannot be empty!\n";
    cout << "Records in the page before deletion.\n";
    status = hfp.firstRecord(Rid); i=0; j=0; 
    while (status == OK)
    { status = hfp.returnRecord(Rid, temp, size); CurRid=Rid; 
      if (status != OK) { 
         cout << "ERROR: getting record.\n"; 
         break; 
      } else {
         cout << "Retrieved record, RID " << Rid.pageNo << ", "
              << Rid.slotNo << endl;
      }
      if ((j%4 == 0)&&(4*j<3*Limit)) { DelList[i]=Rid; i++; } 
      //if compaction is taking place, we shall see.
      status=hfp.nextRecord(CurRid, Rid); j++; 
    } 
    if ( status != DONE ) 
       cout << "ERROR: failed to traverse all records.\n";
    
    DelList[i]=CurRid; j=i+1; //picks up the last one;
    cout << "Deleting Records"<<endl;
    for (i=0; i<j; i++) 
    { Rid=DelList[i]; 
      cout<<"Deleting Rid("<<Rid.pageNo<<","<< Rid.slotNo<<").\n";
      status = hfp.deleteRecord(Rid);
      if (status != OK) 
         cout << "ERROR: Deletion of Rid(" << Rid.pageNo << ","
              << Rid.slotNo << ") failed\n";
    }
    
    cout << "After Deletion. \n";
    status = hfp.firstRecord(Rid); 
    while (status == OK)
    { 
      cout << "Retrieving record " << Rid.pageNo << ", "
           << Rid.slotNo << endl;
      status = hfp.returnRecord(Rid, temp, size); CurRid=Rid; 
      if (status != OK) { cout << "ERROR: getting record.\n"; break; }
      status=hfp.nextRecord(CurRid, Rid);
    } 
    if ( status != DONE ) cout << "failed to traverse all records.\n";
    else status=OK; 
    return (status == OK);
}



int HfpDriver::test5()
{   HFPage hfp;
    Status status = OK;
    RID Rid, CurRid;
    PageId tmpPID = 7; 
    int i; 
    char LongStr[2001];

    hfp.init(tmpPID);
    for (i=0; i < 2000; i++) LongStr[i]=(char)(i%26+65); LongStr[2000]='\0';

    cout << "\n  Test 5: Test some error conditions\n";
    //Get next/prev/etc. pages without setting them
    cout <<"Current Page No.: " <<hfp.page_no() <<", "
         <<"Next Page No.: " <<hfp.getNextPage() <<", "
         <<"Prev Page No.: " <<hfp.getPrevPage() <<", "
         <<"Available Space: " <<hfp.available_space() <<endl;

    //Deletion of empty page
    Rid.pageNo=tmpPID; Rid.slotNo=-6; status = hfp.deleteRecord(Rid);
    if (status == OK)
        cout << "ERROR: empty space deletion.\n" << endl;
    else 
	cout << "No record is deleted.\n"<<endl; 

    Rid.pageNo=tmpPID; Rid.slotNo=6; status = hfp.deleteRecord(Rid);
    if (status == OK)
        cout << "ERROR: improper deletion.\n" << endl;
    else 
       cout << "No record is deleted. \n" <<endl;

    //FirstRecord of empty page
    status=hfp.firstRecord(Rid); 
    if (status != DONE) 
       cout<<"ERROR: DONE must be used for an empty page.\n";
    else cout <<"FirstRecord in an empty page is handled correctly. \n"; 

    //NextRecord of empty page
    CurRid = Rid; status = hfp.nextRecord(CurRid, Rid);
    if (status != FAIL) 
       cout<<"ERROR: There could not be more records(1).\n";
    CurRid.pageNo=tmpPID; CurRid.slotNo=-6; 
    status = hfp.nextRecord(CurRid, Rid);
    if (status != FAIL)
       cout<<"ERROR: There could not be more records(2).\n";

    //Overfilling pages
    status = hfp.insertRecord(LongStr, strlen(LongStr)+1, Rid);
    if (status == OK) 
        cout << "ERROR: This should not be possible!\n";
    else cout << "Overflow handled correctly.\n"<<endl;

    //Get next/prev/etc. pages without setting them
    cout <<"Current Page No.: " <<hfp.page_no() <<", "
         <<"Next Page No.: " <<hfp.getNextPage() <<", "
         <<"Prev Page No.: " <<hfp.getPrevPage() <<", "
         <<"Available Space: " <<hfp.available_space() <<endl;

    minibase_errors.clear_errors(); 
    return (status !=OK);
}

struct td {
   bool active;
   RID rid;
   double value;
   int length;
};

int HfpDriver::test6()
{   
    Status status = OK;
    return (status == OK);
}

