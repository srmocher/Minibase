#include <math.h> 
#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <pwd.h>

#include "buf.h"
#include "db.h"
#include "btfile.h"
#include "btree_driver.h"

#define MAX_COMMAND_SIZE 100


Status BTreeTest::runTests(){

	Status status;

	char real_logname[50];		// the dbname in the case
	char real_dbname[55];		// is followed by the login id.


	sprintf(real_logname, "/bin/rm -rf btlog");
	sprintf(real_dbname, "/bin/rm -rf BTREEDRIVER");

	system(real_logname);
	system(real_dbname);
	
        minibase_globals = new SystemDefs(status,"BTREEDRIVER", "btlog",
						  1000,500,200,"Clock");

	if (status != OK) {
		minibase_errors.show_errors();
		exit(1);
	}


	test1();
	test2();
	test3();
	test4();


	delete minibase_globals;

	sprintf(real_logname, "/bin/rm -rf btlog");
	sprintf(real_dbname, "/bin/rm -rf BTREEDRIVER");
	system(real_logname);
	system(real_dbname);
	return OK;

}

void BTreeTest::test_scan(IndexFileScan* scan)
{

	cout <<"\nStart Scan!\n"<<endl; 
	if(scan == NULL) {
		cout << "Cannot open a scan." << endl;
		minibase_errors.show_errors();
		exit(1);
	}   

	Status status;
	RID rid;
	int count = 0;
	int size;
	int*  ikey;
	char* ckey;

	status = OK;
	size = scan->keysize();
	char* temp = new char[size];

	while (status == OK) {
		status = scan->get_next(rid, temp);
		if (status == OK)  {
	    		cout << "Scanning record with [pageNo,slotNo] = ";
	    		cout << "[" << rid.pageNo<<","<<rid.slotNo<<"]";
			if (size == sizeof(int)) {
				ikey = (int *) temp;
				cout << "\tkey = " << *ikey <<";"<<endl;
		   	} else {
				ckey = (char*) temp;
				cout <<"\t key = " << ckey <<";"<<endl;
			}
			count++;
		}
		else  if (status != DONE)
	    		minibase_errors.show_errors();

	}

	delete [] temp;

        if (status != DONE){
	  cout << "Status = " << status << endl;
	  minibase_errors.show_errors();
        }



	cout << "\nNumber of records scanned = " << count << endl;
}


void BTreeTest::test1() {

    cout << "\n---------test1()  key type is Integer--random------\n";

    Status status;
    BTreeFile *btf;
    IndexFileScan* scan;
    
    int key, lokey, hikey,i;
    RID rid;
    int num;
    

    // test create()
    // if index exists, open it else create
    btf = new BTreeFile(status, "BTreeIndex", attrInteger, sizeof(int));
    if (status != OK) {
        minibase_errors.show_errors();
        exit(1);
    }
    cout << "BTreeIndex created successfully." << endl << endl;


    // test insert()
    num = 2000;
    
struct dummy{
RID r;
int key;
};

    cout << "\nstart BTreeIndex insertion" << endl << endl;

    dummy kill[410];
    for (i = 0; i < num; i++) {
        rid.pageNo = i; rid.slotNo = i+1;
	key = num - i; 
	if (i % 10 == 0) {
	  kill[(i/10)].r.pageNo = rid.pageNo;
	  kill[(i/10)].r.slotNo = rid.slotNo;
	  kill[i/10].key = key;
        }

        if (btf->insert(&key, rid) != OK) {
 	   cout << "Inserting record with key = " << key << "  [pageNo,slotNo] = ";
	   cout << "[" << rid.pageNo<<","<<rid.slotNo<<"] failed!!\n" <<endl;
            minibase_errors.show_errors();
        }
    }

    // test delete()

    cout << "\nstart BTreeIndex deletion" << endl << endl;
    int j = 0;
    for (i = 0; i < num; i++) {
        if (i % 10 == 0) {
	    j++;
	    if (btf->Delete(&kill[i/10].key, kill[(i/10)].r) != OK) {
	    	cout << " Deleting record with key = " << kill[i/10].key << "  [pageNo,slotNo] = ";
	        cout << "[" << kill[i/10].r.pageNo<<","<<kill[i/10].r.slotNo<<"] failed !!"<<endl;
	       minibase_errors.show_errors();
	    }

        }
    }

    delete btf;

    btf = new BTreeFile(status, "BTreeIndex");
    if(status == OK)
	cout<<"\n BTreeIndex opened successfully." << endl << endl;
  
    // test scan and delete_current()
    cout << "\n----------- Testing scans -------------" << endl;
    lokey = 200;
    hikey = 400;

    //AllScan
    scan = btf->new_scan(NULL,NULL);
    test_scan(scan);
    delete scan;   
	


    //MaxRangeScan
    scan = btf->new_scan(NULL, &hikey);
    test_scan(scan);
    delete scan;



    //MinRangeScan;
    scan = btf->new_scan(&lokey, NULL);
    test_scan(scan);
    delete scan;   

    
    //ExactMatch
    scan = btf->new_scan(&hikey, &hikey);
    test_scan(scan);
    delete scan;


   
    //MinMaxRangeScan with delete_current()
    scan = btf->new_scan(&lokey, &hikey);
    int count = 0;
    int size = scan->keysize();
    char* temp = new char[size];
    int* ikey;
    char* ckey;

    status = OK;

    while (status == OK) {
	    if ((status = scan->get_next(rid, temp)) == OK)  {
		count++;
		if ((status = scan->delete_current()) == OK) {
		  cout << "Record with [pageNo,slotNo] = ";
		  cout << "[" << rid.pageNo<<","<<rid.slotNo<<"]"; 
		  if (size == sizeof(int)) {
		  	ikey = (int *) temp;
		  	cout << "\tkey = " << *ikey;
		  } else {
		  	ckey = (char*) temp;
		  	cout <<"\t key = " << ckey;
		  }
		  cout << "\tdeleted !!" <<endl; 
		} else {
		  cout << "Failure to delete record...\n";
		  minibase_errors.show_errors();
                }
	    }
	    else if (status != DONE)
	   	 minibase_errors.show_errors();
    }

    delete [] temp;  

    if (status != DONE) {
      cout << "Something is wrong in test1\n";
      minibase_errors.show_errors();
    }

    delete scan;

    delete btf;

    cout << "\n---------------End of Test 1----------------------\n\n";
}

void BTreeTest::test2() {

	cout << "\n-------------test2()  key type is Integer------------\n";

	Status status;
	BTreeFile *btf;
	IndexFileScan* scan;

	int lokey, hikey;
	RID rid;

	// test open()

	btf = new BTreeFile(status, "BTreeIndex");
	if (status != OK) {
		minibase_errors.show_errors();
		cout << "You should run test1 first to create the index!\n";
		return;
	}
	cout << "BTreeIndex opened successfully." << endl;

	//test abnormal scans
	//lokey > hikey
	lokey = 1000;
	hikey = 100;
	scan = btf->new_scan(&lokey,&hikey);
	cout << "\n-----Start MinMaxRangeScan with lokey = " << lokey \
			 << " hikey = " << hikey << "-----\n";


	char* temp = new char[scan->keysize()];   // BK
	if ((status = scan->get_next(rid, temp)) == OK)  {
		cout << "Error: find next??? no way!" << endl;
        	minibase_errors.show_errors();
		exit(1);
	}

	if(status != DONE)
	   minibase_errors.show_errors();
	delete scan;
    
	cout << " Failed as expected " << endl;

	//lokey > largest key
	lokey = 10000;
	hikey = 10010;
	scan = btf->new_scan(&lokey,&hikey);
	cout << "\n-----Start MinMaxRangeScan with lokey = " << lokey \
			 << " hikey = " << hikey << "-----\n";

	if ((status = scan->get_next(rid, temp)) == OK)  {
		cout << "Error: find next??? no way!" << endl;
       		 minibase_errors.show_errors();
		exit(1);
	}

	
	if(status != DONE)
	   minibase_errors.show_errors();
	delete scan;

	cout << " Failed as expected " << endl;

	//hikey < smallest key
	lokey = -100;
	hikey = -50;
	scan = btf->new_scan(&lokey,&hikey);
		cout << "\n-----Start MinMaxRangeScan with lokey = " << lokey \
			 << " hikey = " << hikey << "------\n";


	if ((status = scan->get_next(rid, temp)) == OK)  {
		cout << "Error: find next??? no way!" << endl;
        minibase_errors.show_errors();
		exit(1);
	}

	if(status != DONE)
	   minibase_errors.show_errors();
	delete scan;

	cout << " Failed as expected " << endl;

	// test destroyFile()
	cout << "\n------Start to destroy the index------" << endl;

	status = btf->destroyFile();
	if (status != OK)
		minibase_errors.show_errors();
	cout << "\n------Destroyed the index without any errors---" << endl;

	delete [] temp;  
	delete btf;
	cout << "\n ----End of Test 2----------------------\n\n";
}

void BTreeTest::test3() {

    cout << "\n--------test3() key type is String---------\n";

    Status status;
    BTreeFile *btf;
    IndexFileScan* scan;
    
    int keysize = MAX_KEY_SIZE1;
    char*  key = new char[keysize];
    char*  lokey = new char[keysize];
    char*  hikey = new char[keysize];
    int	i = 0;
    RID rid, lorid;
    ifstream keysamples;

    keysamples.open("keys",ios::in);
    if (!keysamples) {
      cout << "keys not found.\n";
      cout << " there is a copy in $MINIBASE_HOME/programs/minibase "<< endl;
      return;
    }


    // test create()
    btf = new BTreeFile(status, "BTreeIndex", attrString, keysize);
    if (status != OK) {
        minibase_errors.show_errors();
        exit(1);
    }
    cout << "BTreeIndex created successfully." << endl;


    // test insert()
    cout << "\n------Start to insert records--------" << endl;

    keysamples.getline(key, keysize, '\n');
    while(!keysamples.eof()) {
		rid.pageNo = (int)(key[0]+key[1]+key[2]);
		rid.slotNo = rid.pageNo;
        if (btf->insert(key, rid) != OK) {
            minibase_errors.show_errors();
        }
	
	i++;
	if(i==20) strncpy(lokey,key,keysize);
	if(i==100) strncpy(hikey,key,keysize);
	keysamples.getline(key, keysize, '\n');
    }
    cout << "\nNumber of records inserted is " << i << endl;
    cout << "\n--------------End of insert----------------" << endl;

    // test delete()
    cout << "\n------Start to delete some records----------" << endl;

    // delete the lokey
    lorid.pageNo = (int)(lokey[0]+lokey[1]+lokey[2]);
    lorid.slotNo = lorid.pageNo;

    if (btf->Delete(lokey, lorid) != OK)
        minibase_errors.show_errors();
    else
	cout << "\nSuccessfully deleted record with key = " << lokey << endl;
    cout << "\n---------------End of delete----------------" << endl;

    delete btf;
    btf = new BTreeFile(status, "BTreeIndex");
  
    // test scan and delete_current
    //AllScan     
    scan = btf->new_scan(NULL,NULL);                                            
    cout << "\n---------------Start AllScan------------" << endl;
    test_scan(scan);                                                            
    delete scan;                                                                
    cout <<"\n------End of AllScan------" << endl;
	
    //MaxRangeScan                                                              
    scan = btf->new_scan(NULL, hikey);                                       
    cout << "\n\n------Start MaxRangeScan with hikey = "<<hikey<< "------\n";
    test_scan(scan);     
    delete scan;                                                                
    cout << "\n------End of MaxRangeScan with hikey = "<<hikey<< "------\n";
    

    //MinRangeScan;                                                        
    scan = btf->new_scan(lokey, NULL);                                         
    cout << "\n\n-----Start MinRangeScan with lokey = "<<lokey<< "------\n"; 
    test_scan(scan);                                                            
    delete scan;                                                                
    cout << "\n------End of MinRangeScan with lokey = "<<lokey<< "------\n"; 

    //ExactMatch                                                                
    scan = btf->new_scan(hikey, hikey);                                       
    cout << "\n\n------Start ExactMatch with key = " <<hikey << "------\n"; 
    test_scan(scan);                                                            
    delete scan;
    cout << "\n------End of ExactMatch with key = " <<hikey << "------\n"; 
                                             
    //MinMaxRangeScan
    scan = btf->new_scan(lokey, hikey);
    cout << "\n\n------Start MinMaxRangeScan------" << endl;
    if(scan == NULL) {
	cout << "Cannot open a scan." << endl;
    }

    cout << "\n------Start scan with lokey = "<<lokey << " hikey = "<<hikey \
		<< "-----" << endl;

    int count = 0;
    status = OK;
    while (status == OK) {
	char* temp = new char[scan->keysize()];   // BK
        if ((status = scan->get_next(rid, temp)) == OK)  {
	    count++;
            if ((status = scan->delete_current()) == OK) {
	    	cout << "Record with [pageNo,slotNo] = ";
                cout << "[" << rid.pageNo<<","<<rid.slotNo<<"] deleted"<<endl;
	    }
            else
                minibase_errors.show_errors();
        }

        delete [] temp;  // BK
    }

    if (status != DONE)
    {
     cout << "Problem...\n";
     minibase_errors.show_errors();
    }
    cout << "Number of records scanned = " << count << endl;
    cout << "\n-------End of MinMaxRangeScan------\n";
    delete scan;


    cout << "\n\n------Testing abnormal scans------\n";

	// test abnormal scans
	// lokey > hikey
	strcpy(lokey, "zabcd");
	strcpy(hikey, "abcde");
	scan = btf->new_scan(lokey, hikey);

	char *temp1 = new char[MAX_KEY_SIZE1];
	if ((status = scan->get_next(rid, temp1)) == OK) {
		cout << " Error: find next??? nothing to find!!" << endl;
		minibase_errors.show_errors();
		exit(1);
	}

	if (status != DONE)
		minibase_errors.show_errors();
	delete scan;

	cout << " Failed as expected: no records scanned " << endl;

	// lokey > the largest key 
	strcpy(lokey, "zabcd");
	strcpy(hikey, "zcdef");
	scan = btf->new_scan(lokey, hikey);
	if ((status = scan->get_next(rid, temp1)) == OK) {
		cout << " Error: find next??? nothing to find!!" << endl;
		minibase_errors.show_errors();
		exit(1);
	}

	if (status != DONE)
		minibase_errors.show_errors();
	delete scan;

	cout << " Failed as expected: no records scanned " << endl;

	// hikey < smallest key
	strcpy(lokey, "aaa");
	strcpy(hikey, "aaaaa");
	scan = btf->new_scan(lokey, hikey);
	cout << "\n----Start MinMaxRangeScan with lokey = " << lokey ;
	cout << " hikey = " << hikey << "-------\n" ;
	cout << " -------hikey is smaller than the smallest key" << endl;

	if ((status = scan->get_next(rid, temp1)) == OK) {
		cout << " Error: find next??? nothing to find!!" << endl;
		minibase_errors.show_errors();
		exit(1);
	}

	if (status != DONE)
		minibase_errors.show_errors();
	delete scan;

	cout << " Failed as expected: no records scanned " << endl;

    delete[] temp1; 
    delete btf;
    
    // test destroyFile()

    btf = new BTreeFile(status, "BTreeIndex");

    cout << "-------Start to destroy the index-----------" << endl;
    status = btf->destroyFile();
    if (status != OK)
        minibase_errors.show_errors();
    cout << "-------End to destroy the index-----------" << endl;

    delete btf;
    delete[] key;
    delete[] lokey;
    delete[] hikey;
    cout << "\n\n---------End of Test 3 ---------------------\n\n";
}


struct dummy {
  RID r;
  int key;
  int sort_value1;
  int sort_value2;
};

int eval1(const void *first, const void *second)
{
  dummy *f = (dummy*)first;
  dummy *s = (dummy*)second;

  return f->sort_value1 - s->sort_value1;
}

int eval2(const void *first, const void *second)
{
  dummy *f = (dummy*)first;
  dummy *s = (dummy*)second;

  return f->sort_value2 - s->sort_value2;
}


void BTreeTest::test4() {


    Status status;
    BTreeFile *btf;
    IndexFileScan* scan;
    
    int key, lokey, hikey;
    RID rid;
    int num = 1000;
    int num_deletes = 400;
    int i;
    dummy values[num];

	cout << "\n---------test4()  key type is Integer--------------\n";
    

	// test create()
	// if index exists, open it else create
    btf = new BTreeFile(status, "BTreeIndex", attrInteger, sizeof(int));
    if (status != OK) {
        minibase_errors.show_errors();
        exit(1);
    }
    cout << "\nBTreeIndex created successfully." << endl << endl;


    cout << " Creating " << num << " random entries" << endl;

    for ( i = 0; i < num; i++) {
        values[i].key = i * 8 + (rand() % 8);
 	values[i].r.pageNo = i;
	values[i].r.slotNo = i+1;
	values[i].sort_value1 = rand() % 1000000;
        values[i].sort_value2 = rand() % 1000000;
/*
cout << " key " << values[i].key
     << " sort_value1 " << values[i].sort_value1
     << " sort_value2 " << values[i].sort_value2 << endl;
*/
    }

    // test insert()
    
    // put values in insertion order
    qsort( values, num, sizeof(dummy), eval1);
    
    cout << "\n------Start to insert " << num << "  records------" << endl;

    for (i=0; i < num; i++){
        //cout << " Inserting key " << values[i].key << " order " 
             //<< values[i].sort_value1 << endl;
       	if (btf->insert(&(values[i].key), values[i].r) != OK) {
            minibase_errors.show_errors();
        }
    }
    cout << "\n------ End of insert------" << endl;

   

    // test delete()
    cout << "\n\n------ Delete the first " << num_deletes 
         << " of the records-----" << endl;

    // place records in deletion order
    qsort(values, num, sizeof(dummy), eval2);

    for (i = 0; i < num_deletes; i++) {
/*
	    cout << "Deleting record with key = " << values[i].key 
                 << "  [pageNo,slotNo] = ";
	    cout << "[" << values[i].r.pageNo<<","
                 << values[i].r.slotNo << "]" <<endl;
*/
	    if (btf->Delete(&values[i].key, values[i].r) != OK) {
	       minibase_errors.show_errors();
	    }
    }
    cout << "Deleted  " << i << "  records " << endl;
    cout << "\n------ End of delete ------" << endl;

    delete btf;

    btf = new BTreeFile(status, "BTreeIndex");
  
    // test scan and delete_current()
    cout << "\n\n------ Testing scans ------" << endl;
    lokey = 570;
    hikey = 690;
    i= 1000;
    while (i < 1020){
        rid.pageNo = i;
	rid.slotNo = i+ 1;
	if(i < 1010) key = lokey;
	else key = hikey;
	if(btf->insert(&key,rid) != OK)
		minibase_errors.show_errors();
        i++;
    }

    //AllScan
    scan = btf->new_scan(NULL,NULL);
    cout << "\n\n------Start AllScan------" << endl;

    test_scan(scan);
    delete scan;   
	
    cout << "\n------End of AllScan------" << endl;

    //MaxRangeScan
    scan = btf->new_scan(NULL, &hikey);
    cout << "\n\n------Start MaxRangeScan with hikey = "<<hikey<<"------\n";
    test_scan(scan);
    delete scan;

    cout << "\n------End of MaxRangeScan with hikey = "<<hikey<<"------\n";

    //MinRangeScan;
    scan = btf->new_scan(&lokey, NULL);
    cout << "\n\n------Start MinRangeScan with lokey = "<<lokey<<"------\n";
    test_scan(scan);
    delete scan;   

    cout << "\n------End of MinRangeScan with lokey = "<<lokey<<"------\n";
    
    //ExactMatch
    scan = btf->new_scan(&hikey, &hikey);
    cout << "\n\n------Start ExactMatch with key = " <<hikey <<"-------\n";
    test_scan(scan);
    delete scan;
    cout << "\n------End of ExactMatch with key = " <<hikey <<"-------\n";

   
    //MinMaxRangeScan with delete_current()
    scan = btf->new_scan(&lokey, &hikey);
    cout << "\n\n------Start MinMaxRangeScan with lokey = "<<lokey   \
	    << " hikey = "<<hikey<<"------\n";
    cout << "Will also perform delete_current()\n";
  
    int count = 0;
    status = OK;
    while (status == OK) {
	    char* temp = new char[scan->keysize()];    // BK
	    if ((status = scan->get_next(rid, temp)) == OK)  {
		count++;
		if ((status = scan->delete_current()) == OK) {
		  cout << "Record with [pageNo,slotNo] = ";
		  cout << "[" << rid.pageNo<<","<<rid.slotNo<<"] deleted"<<endl;
		}
		else
		{
		  cout << "Failure to delete record...\n";
		  minibase_errors.show_errors();
                }
             delete [] temp;  // BK
	    }
    }

    if (status != DONE) {
      cout << "Something is wrong in test4\n";
      minibase_errors.show_errors();
    }

    cout << "Number of records scanned = " << count << endl;
    cout << "\n------End of MinMaxRangeScan -----------------------\n";
    delete scan;
    

    delete btf;

    // test destroyFile()
	cout << "\n\n-----------Destroying index----------\n";

    btf = new BTreeFile(status, "BTreeIndex");

    cout << "\n-------Start to destroy the index----------" << endl;
    status = btf->destroyFile();
    if (status != OK)
        minibase_errors.show_errors();

    delete btf;

    cout << "\n--------- End of destroying the index -----" <<endl;
    cout << "\n\n--------- End of test4   -------------" <<endl;
}

