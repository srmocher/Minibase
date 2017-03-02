#ifndef _HEAPFILE_H
#define _HEAPFILE_H

#include "minirel.h"
#include "page.h"
#include "hfpage.h"
#include "scan.h"
#include "buf.h"
#include "db.h"
#include "new_error.h"

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
  int    availspace;  // HFPage returns int for avail space, so we use int here
  int    recct;       // for efficient implementation of getRecCnt()
  PageId pageId;      // obvious: id of this particular data page (a HFPage)
};

class HeapFile {

  public:

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

    PageId      firstDirPageId;   // page number of header page
    int         file_deleted;
    char       *fileName;

    Status newDataPage(DataPageInfo *dpinfop);
    Status findDataPage(const RID& rid, 
			PageId &rpDirPageId, HFPage *&rpdirpage, 
			PageId &rpDataPageId,HFPage *&rpdatapage, 
			RID &rpDataPageRid);
    Status allocateDirSpace(struct DataPageInfo * dpinfop,
                            PageId &allocDirPageId,
                            RID &allocDataPageRid);
};


#endif    // _HEAPFILE_H
