// minirel.h

#ifndef _MINIREL_H
#define _MINIREL_H

#include <iostream>

#include "da_types.h"
#include "new_error.h"
#include "system_defs.h"


enum AttrType { attrString, attrInteger, attrReal,  attrSymbol, attrNull };
enum AttrOperator {
	aopEQ, aopLT, aopGT, aopNE, aopLE, aopGE, aopNOT, aopNOP, aopRANGE };
enum LogicalOperator { lopAND, lopOR, lopNOT };
enum TupleOrder { Ascending, Descending, Random };
enum IndexType {None, B_Index, Hash};
enum SelectType { selRange, selExact, selBoth, selUndefined };


typedef int PageId;

struct RID{
	PageId  pageNo;
	int  slotNo;
	int operator==(const RID rid) const
	  {return pageNo==rid.pageNo && slotNo==rid.slotNo;};
	int operator!=(const RID rid) const
	  {return pageNo!=rid.pageNo || slotNo!=rid.slotNo;};
};


const int MINIBASE_PAGESIZE = 1024;           // in bytes
const int MINIBASE_BUFFER_POOL_SIZE = 1024;   // in Frames
const int MINIBASE_DB_SIZE = 10000;           /* in Pages => the DBMS Manager 
						 tells the DB how much disk 
						 space is available for the 
						 database.
					      */

const int MINIBASE_MAX_TRANSACTIONS = 100;
const int MINIBASE_DEFAULT_SHAREDMEM_SIZE = 1000;

const int MAXFILENAME = 15;          // also the name of a relation
const int MAXINDEXNAME = 40;         // 
const int MAXATTRNAME = 15;          //


#endif

