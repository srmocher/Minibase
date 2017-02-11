/*
 * class Scan
 * $Id: scan.h,v 1.1 1997/01/02 12:46:43 flisakow Exp $
 */

#ifndef _SCAN_H_
#define _SCAN_H_

#include "minirel.h"

// ***********************************************************
// A Scan object is created ONLY through the function openScan
// of a HeapFile. It supports the getNext interface which will
// simply retrieve the next record in the heapfile.
//
// An object of type scan will always have pinned one directory page
// of the heapfile.

class HeapFile;
class HFPage;

class Scan {

  public:
    // The constructor pins the first page in the file
    // and initializes its private data members from the private
    // data members from hf
    Scan(HeapFile *hf, Status& status);
   ~Scan();

    // Retrieve the next record in a sequential scan
    // Also returns the RID of the retrieved record.
    Status getNext(RID& rid, char *recPtr, int& recLen);

    // Position the scan cursor to the record with the given rid.
    // Returns OK if successful, non-OK otherwise.
    Status position(RID rid);

  private:
    /*
     * See heapfile.h for the overall description of a heapfile.
     * (Then see hfpage.h for HFPage ops.)
     */

    // The heapfile we are using.
    HeapFile  *_hf;

    // PageId of current directory page (which is itself an HFPage)
    // (the actual PageId of the dir page with current position)
    PageId dirPageId;
    
    // the actual PageId of the data page with the current record
    PageId dataPageId;

    // record ID of the DataPageInfo struct (in the directory page) which
    // describes the data page where our current record lives.
    // (the rid of the data page)
    RID dataPageRid;

    // pointer to in-core data of dirpageId (page is pinned) 
    HFPage *dirPage;

    // in-core copy (pinned) of the same
    HFPage *dataPage;

    // record ID of the current record (from the current data page)
    RID     userRid;

    // flag for whether to check scan is done (0 or 1)
    int     scanIsDone; // (may not be used)

    // status value of whether next record exists
    int     nxtUserStatus;

    // Do all the constructor work
    Status init(HeapFile *hf);

    // Reset everything and unpin all pages.
    Status reset();

    // Move over the data pages in the file (firstDataPage(), nextDataPage())

    // Get the first data pages in the file
    Status firstDataPage();
    // Get next data page
    Status nextDataPage();

    // Get next directory page
    Status nextDirPage();

    // Look ahead the next record
    Status peekNext(RID& rid) {
        rid = userRid;
        return OK;
    }

    // Move to the next record in a sequential scan.
    // Also returns the RID of the (new) current record.
    Status mvNext(RID& rid);
};

#endif  // _SCAN_H
