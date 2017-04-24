#ifndef _HEAPFILE_H
#define _HEAPFILE_H

#include "minirel.h"
#include "page.h"
#include "hfpage.h"
#include "scan.h"
#include "buf.h"
#include "db.h"
#include "new_error.h"

//  This heapfile implementation is directory-based. We maintain a
//  directory of info about the data pages (which are of type HFPage
//  when loaded into memory).  The directory itself is also composed
//  of HFPages, with each record being of type DataPageInfo
//  as defined below.
//
//  The first directory page is a header page for the entire database
//  (it is the one to which our filename is mapped by the DB).
//  All directory pages are in a doubly-linked list of pages, each
//  directory entry points to a single data page, which contains
//  the actual records.
//
//  The heapfile data pages are implemented as slotted pages, with
//  the slots at the front and the records in the back, both growing
//  into the free space in the middle of the page.
//  See the file 'hfpage.h' for specifics on the page implementation.
//
//  We can store roughly pagesize/sizeof(DataPageInfo) records per
//  directory page; for any given HeapFile insertion, it is likely
//  that at least one of those referenced data pages will have
//  enough free space to satisfy the request.

// Error codes for HEAPFILE.
enum heapErrCodes {
    BAD_RID,
    BAD_REC_PTR,
    HFILE_EOF,
    INVALID_UPDATE,
    NO_SPACE,
    NO_RECORDS,
    END_OF_PAGE,
    INVALID_SLOTNO,
    ALREADY_DELETED,
};

// DataPageInfo: the type of records stored on a directory page:

struct DataPageInfo {
  int    availspace;  // total available space of a page: HFPage returns int for avail space, so we use int here
  int    recct;       // number of records in the page: for efficient implementation of getRecCnt()
  PageId pageId;      // page id: id of this particular data page (a HFPage)
};

class HeapFile {

  public:

    // Initialize.  A null name produces a temporary heapfile which will be
    // deleted by the destructor.  
    // If the name already denotes a file, the
    // file is opened; otherwise, a new empty file is created.
    HeapFile( const char *name, Status& returnStatus ); 
    ~HeapFile();

    // return number of records in file
    int getRecCnt();
    
    // insert record into file
    Status insertRecord(char *recPtr, int recLen, RID& outRid); 
    
    // delete record from file
    Status deleteRecord(const RID& rid); 

    // updates the specified record in the heapfile.
    Status updateRecord(const RID& rid, char *recPtr, int reclen);

    // read record from file, returning pointer and length
    Status getRecord(const RID& rid, char *recPtr, int& recLen); 

    // initiate a sequential scan
    class Scan *openScan(Status& status);

    // delete the file from the database
    Status deleteFile();


  private:
    friend class Scan;

    PageId      firstDirPageId;  // page number of header page
    int         file_deleted;	 // flag for whether file is deleted (initialized to be false in constructor)
    char       *fileName;	 // heapfile name

    // get new data pages through buffer manager
    // (dpinfop stores the information of allocated new data pages)
    Status newDataPage(DataPageInfo *dpinfop);
    
    // return a data page (rpDataPageId, rpdatapage) containing a given record (rid) 
    // as well as a directory page (rpDirPageId, rpdirpage) containing the data page and RID of the data page (rpDataPageRid)
    Status findDataPage(const RID& rid, 
			PageId &rpDirPageId, HFPage *&rpdirpage, 
			PageId &rpDataPageId,HFPage *&rpdatapage, 
			RID &rpDataPageRid);

    // put data page information (dpinfop) into a dir page(s)
    Status allocateDirSpace(struct DataPageInfo * dpinfop,/* data page information*/
                            PageId &allocDirPageId,/*Directory page having the first data page record*/
                            RID &allocDataPageRid /*RID of the first data page record*/);
};


#endif    // _HEAPFILE_H
