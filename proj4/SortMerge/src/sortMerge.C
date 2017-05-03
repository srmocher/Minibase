
#define DEBUG
#include <string.h>
#include <assert.h>
#include "sortMerge.h"


// Error Protocall:


enum ErrCodes {SORT_FAILED, HEAPFILE_FAILED};

static const char* ErrMsgs[] =  {
  "Error: Sort Failed.",
  "Error: HeapFile Failed."
  // maybe more ...
};

static error_string_table ErrTable( JOINS, ErrMsgs );

// sortMerge constructor
sortMerge::sortMerge(
    char*           filename1,      // Name of heapfile for relation R
    int             len_in1,        // # of columns in R.
    AttrType        in1[],          // Array containing field types of R.
    short           t1_str_sizes[], // Array containing size of columns in R
    int             join_col_in1,   // The join column of R 

    char*           filename2,      // Name of heapfile for relation S
    int             len_in2,        // # of columns in S.
    AttrType        in2[],          // Array containing field types of S.
    short           t2_str_sizes[], // Array containing size of columns in S
    int             join_col_in2,   // The join column of S

    char*           filename3,      // Name of heapfile for merged results
    int             amt_of_mem,     // Number of pages available
    TupleOrder      order,          // Sorting order: Ascending or Descending
    Status&         s               // Status of constructor
){
	// fill in the body
    Status status;

    char firstName[20] = "";
    strcat(firstName,filename1);
    strcat(firstName,"sorted");
    char secondName[20] = "secondFile";
    strcat(secondName,"sorted");
    Sort sortFirst(filename1,firstName,len_in1,in1,t1_str_sizes,join_col_in1,order,amt_of_mem,status);
    Sort sortSecond(filename2,secondName,len_in2,in2,t2_str_sizes,join_col_in2,order,amt_of_mem,status);

    firstFile = new HeapFile(firstName,status);
    secondFile = new HeapFile(secondName,status);
    mergedFile = new HeapFile(filename3,status);

    AttrType firstAttr = in1[join_col_in1];
    AttrType  secondAttr = in2[join_col_in2];

    int firstSize = t1_str_sizes[join_col_in1];
    int secondSize = t2_str_sizes[join_col_in2];
    if(firstAttr!=secondAttr) {
        s = DONE;
        return;
    }
    RID firstRID,secondRID;
    char *firstRecord, *secondRecord;
    int firstLength,secondLength;
    firstRecord = new char[100];
    secondRecord = new char[100];
    int firstOffset = 0,secondOffset = 0;
    int i=0;
    while(i < join_col_in1)
    {
        firstOffset += t1_str_sizes[i];
        i++;
    }
    char *joinFirstColumn = new char[t1_str_sizes[i]];
    i=0;
    while (i < join_col_in2)
    {
        secondOffset += t2_str_sizes[i];
        i++;
    }
    char *joinSecondColumn = new char[t2_str_sizes[i]];
    Status status1,status2;

    int size;

    size = t1_str_sizes[join_col_in1];

    Status st1,st2;
    Scan *first = firstFile->openScan(st1);
    for(st1=first->getNext(firstRID,firstRecord,firstLength);st1==OK; st1=first->getNext(firstRID,firstRecord,firstLength))
    {
        Scan *second = secondFile->openScan(st2);
        for(st2=second->getNext(secondRID,secondRecord,secondLength);st2==OK;st2=second->getNext(secondRID,secondRecord,secondLength))
        {
             memcpy(joinFirstColumn,firstRecord+firstOffset,t1_str_sizes[join_col_in1]);
             memcpy(joinSecondColumn,secondRecord+secondOffset,t2_str_sizes[join_col_in2]);
            int *num1 = (int *) (firstRecord + firstOffset);
            int *num2 = (int *) (secondRecord + secondOffset);
            int comp = tupleCmp(joinFirstColumn,joinSecondColumn);
            if(comp ==0)
            {

                char *mergedRecord = new char[firstLength + secondLength];
                memcpy(mergedRecord,firstRecord,firstLength);
                memcpy(mergedRecord+firstLength,secondRecord,secondLength);
                RID outRID;
                Status insertStatus = mergedFile->insertRecord(mergedRecord,firstLength+secondLength,outRID);
                if(insertStatus!=OK)
                {
                    s = DONE;
                    return;
                }
            }
        }
    }
    secondFile->deleteFile();
    firstFile->deleteFile();
    s = OK;

}

// sortMerge destructor
sortMerge::~sortMerge()
{
	// fill in the body
}
