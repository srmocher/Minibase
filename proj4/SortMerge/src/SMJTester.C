#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <string.h>
#include <errno.h>
#include <iostream>
#include <assert.h>
#include <unistd.h>

#include "sortMerge.h"
#include "db.h"
#include "buf.h"
#include "minirel.h"
#include "heapfile.h"
#include "scan.h"
#include "new_error.h"
#include <pwd.h>


#include "SMJTester.h"

#define SORTPGNUM	 10
#define NUMFILES	  5
#define NUM_COLS	  2
#define JOIN_COL	  0



SMJTester::SMJTester() : TestDriver( "SortMergeJoinTest" )
{}


SMJTester::~SMJTester()
{}


void createFiles();
Status test(int t);

struct _rec {
	int	key;
	char	filler[4];
};
AttrType 	attrType[] = { attrInteger, attrString };
short		attrSize[] = { 4, 4 };
int 		data0[] =  { 99,6,1,9,1,20,20,5,6,5,20,100,14,7,13,13 };
int 		data1[] =  { 10,1,1,99,18,6,10,10,13,12 };
int 		data2[] =  { 20, 56, 9, 78, 10, 7 };
int 		data3[] =  { 99,13,10,12,8,10,11,13,20,13,66 };
int 		data4[] =  { 2,2,2,13,77,5,11,2 };
int			dsize[] = { 16,10,6,11,8};
int*		data[] = { data0, data1, data2, data3, data4 };
char*		files[] = { "file0", "file1", "file2", "file3", "file4" };

//-------------------------------------------------------------
// createFiles
//-------------------------------------------------------------
void createFiles()
{
	struct _rec rec;
	memcpy(rec.filler,"    ",4);
	for (int i=0; i< NUMFILES; i++)
	{
		Status		s;
		RID		rid;
		HeapFile*	f = new HeapFile(files[i],s);
		if (s != OK)
			cout<<"this is ridiculous !"<<endl;
		for (int j=0; j<dsize[i]; j++)
		{
			rec.key = data[i][j];
			s = f->insertRecord((char*)&rec,sizeof(rec),rid);
			if (s != OK)
				cout<<"this is ridiculous !"<<endl;
		}
		delete f;
	}
}

//-------------------------------------------------------------
// test
//-------------------------------------------------------------
Status test(int t)
{
	char		outfile[10];
	int		Rarray[] = { 0, 1, 0, 3, 4, 4, 4 };
	int		Sarray[] = { 1, 0, 2, 2, 2, 3, 0 };
	int 		R = Rarray[t-1];
	int 		S = Sarray[t-1];
	Status		s;

	sprintf(outfile,"test%d", t);

	// Perform sort-merge on R and S
	sortMerge	sm(files[R],NUM_COLS,attrType,attrSize,JOIN_COL,files[S],NUM_COLS,attrType,attrSize,JOIN_COL,outfile,SORTPGNUM,Ascending,s);
	if (s != OK)
	{
		cout << "Test " << t << " -- sortMerge failed" << endl;
		return s;
	}

	// Write merged results to stdout
	HeapFile*	outf = new HeapFile (outfile,s);
	if (s != OK)
	{
		cout << "Test " << t << " -- result file not created " << endl;
		return s;
	}
        Scan*	scan = outf->openScan(s);
	assert(s == OK);
	int len;
	RID	rid;
	char	rec[sizeof(struct _rec)*2];
	cout << endl;
	cout << "------------ Test " << t << " ---------------" << endl;
	for (s = scan->getNext(rid, rec, len); s == OK; s = scan->getNext(rid, rec, len)) 
	{
	  cout << (*((struct _rec*)&rec)).key << "\t" << (*((struct _rec*)&rec[8])).key << endl;
	}
	cout << "-------- Test " << t << " completed --------" << endl;
	delete scan;
	s=outf->deleteFile();
	if(s!=OK) MINIBASE_CHAIN_ERROR(JOINS,s);
	delete outf;
	return s;
}

//-------------------------------------------------------------------
// test1() calls the function test(int t) to repeatly test the joins.
//-------------------------------------------------------------------
int SMJTester::test1()
{
    Status status;
    createFiles();
    for (int i=1; i<=7; i++) {
    	status=test(i);
    	if (status!=OK) {
    		cout<<"Test "<<i<<" Failed.\n"<<endl;
    		break;
    	}
    }
    return status==OK;
}

int SMJTester::test2()
{
    return true;
}

int SMJTester::test3()
{
    return true;
}

int SMJTester::test4()
{
    return true;
}

int SMJTester::test5()
{
    return true;
}

int SMJTester::test6()
{
    return true;
}


const char* SMJTester::testName()
{
    return "Sort Merge Join";
}

Status SMJTester::runTests()
{
    Status status;
    minibase_globals = new SystemDefs( status, dbpath, logpath, 
				  100,100,200,"LRU" );
    if ( status == OK )
        status=TestDriver::runTests();
    delete minibase_globals;
    return status;
}


Status SMJTester::runAllTests()
{
    return TestDriver::runAllTests();
}

