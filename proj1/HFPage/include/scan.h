/* -*- C++ -*- */
/*
 * scan.h -  class Scan
 *
 * Johannes Gehrke & Gideon Glass  950920  CS564  UW-Madison
 *
 * modified by michael lee 1995
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
    // The constructor pins the first directory page in the file
    // and initializes its private data members from the private
    // data member from hf
    Scan(HeapFile* hf, Status& status);
   ~Scan();

    // Retrieve the next record in a sequential scan
    // Also returns the RID of the retrieved record.
    Status getNext(RID& rid, char* recPtr, int& recLen);

    // Position the scan cursor to the record with the given rid.
    // Returns OK if successful, non-OK otherwise.
    Status position(RID rid);

  private:
    /*
     * See heapfile.h for the overall description of a heapfile.
     * (Then see hfpage.h for HFPage ops.)
     *
     * Note that one record in our way-cool HeapFile implementation is
     * specified by six (6) parameters, some of which can be determined
     * from others:
     */

    // The heapfile we are using.
    HeapFile  *_hf;

    // PageId of current directory page (which is itself an HFPage)
    PageId dirpageId;

    // pointer to in-core data of dirpageId (page is pinned)
    HFPage *dirpage;

    // record ID of the DataPageInfo struct (in the directory page) which
    // describes the data page where our current record lives.
    RID datapageRid;

    // the actual PageId of the data page with the current record
    PageId datapageId;

    // in-core copy (pinned) of the same
    HFPage *datapage;

    // record ID of the current record (from the current data page)
    RID     userrid;
    Status  nxtUserStatus;


    // Do all the constructor work
    Status init(HeapFile *hf);

    // Reset everything and unpin all pages.
    Status reset();

    // Move over the data pages in the file.
    Status firstDataPage();
    Status nextDataPage();

    Status peekNext(RID& rid) {
        rid = userrid;
        return OK;
    }

    // Move to the next record in a sequential scan.
    // Also returns the RID of the (new) current record.
    Status mvNext(RID& rid);
};

#endif  // _SCAN_H
