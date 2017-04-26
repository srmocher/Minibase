
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


    Sort sortFirst(filename1,"firstFile",len_in1,in1,t1_str_sizes,join_col_in1,order,amt_of_mem,status);
    Sort sortSecond(filename2,"secondFile",len_in2,in2,t2_str_sizes,join_col_in2,order,amt_of_mem,status);

    firstFile = new HeapFile("firstFile",status);

    secondFile = new HeapFile("secondFile",status);
    mergedFile = new HeapFile(filename3,status);

    Scan *first = firstFile->openScan(status);
    Scan *second = secondFile->openScan(status);
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
    status1 = first->getNext(firstRID,firstRecord,firstLength);
    status2 = second->getNext(secondRID,secondRecord,secondLength);
    while(status1==OK && status2 == OK)
    {
        memcpy(joinFirstColumn,firstRecord+firstOffset,t1_str_sizes[join_col_in1]);
        memcpy(joinSecondColumn,secondRecord+secondOffset,t2_str_sizes[join_col_in2]);
        int comp = tupleCmp(joinFirstColumn,joinSecondColumn);
        if(comp==0)
        {
            //merge both records and insert
            char *mergedRecord = new char[firstLength + secondLength - t1_str_sizes[join_col_in1]];
            memcpy(mergedRecord,firstRecord,firstLength);
            memcpy(mergedRecord+firstLength,secondRecord,secondOffset);
            int remainingLength = secondLength - t2_str_sizes[join_col_in2] - secondOffset;
            memcpy(mergedRecord+firstLength+secondOffset,secondRecord+secondOffset+t2_str_sizes[join_col_in2],remainingLength);
            RID outRID;
            Status insertStatus = mergedFile->insertRecord(mergedRecord,firstLength+secondLength - t1_str_sizes[join_col_in1],outRID);
            if(insertStatus!=OK)
            {
                s = DONE;
                return;
            }
            status1 = first->getNext(firstRID,firstRecord,firstLength);
            status2 = second->getNext(secondRID,secondRecord,secondLength);
        }
        else if(comp<0)
        {
            status1 = first->getNext(firstRID,firstRecord,firstLength);
        }
        else
        {
            status2 = second->getNext(secondRID,secondRecord,secondLength);
        }
    }
    s = OK;

}

// sortMerge destructor
sortMerge::~sortMerge()
{
	// fill in the body
}
