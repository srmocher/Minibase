/*
 * implementation of class Scan for HeapFile project.
 * $Id: scan.C,v 1.1 1997/01/02 12:46:42 flisakow Exp $
 */

#include <stdio.h>
#include <stdlib.h>

#include "heapfile.h"
#include "scan.h"
#include "hfpage.h"
#include "buf.h"
#include "db.h"

extern vector<HFPage*> directoryPages;
extern string FileName;
// *******************************************
// The constructor pins the first page in the file
// and initializes its private data members from the private data members from hf
Scan::Scan (HeapFile *hf, Status& status)
{
  // put your code here
  init(hf);
  status = OK;
}

// *******************************************
// The deconstructor unpin all pages.
Scan::~Scan()
{

  // put your code here
}

// *******************************************
// Retrieve the next record in a sequential scan.
// Also returns the RID of the retrieved record.
Status Scan::getNext(RID& rid, char *recPtr, int& recLen)
{
    RID curr;
    Status status = dataPage->nextRecord(this->userRid,curr);
    if(status!=OK)
    {
       status = nextDataPage();
        if(status!=OK)
            return status;
        userRid = rid;
       status = dataPage->returnRecord(rid,recPtr,recLen);
        if(status!=OK)
            return status;
        return OK;
    }
    rid = curr;
    status= dataPage->returnRecord(rid,recPtr,recLen);
  // put your code here
      return status;
}

// *******************************************
// Do all the constructor work.
Status Scan::init(HeapFile *hf)
{
  // put your code here
  this->_hf = hf;
  firstDataPage();
  return OK;
}

// *******************************************
// Reset everything and unpin all pages.
Status Scan::reset()
{
  // put your code here
  return OK;
}

// *******************************************
// Copy data about first page in the file.
Status Scan::firstDataPage()
{
    this->dirPage = directoryPages[0];
    Page *page = (Page *)this->dirPage;
    MINIBASE_BM->pinPage(dirPage->page_no(),page,0,_hf->fileName);
    Status status = dirPage->firstRecord(this->dataPageRid);
    if(status!=OK)
        return status;
    this->dirPageId = dirPage->page_no();
    char *record;
    int recLen;
    status = dirPage->returnRecord(this->dataPageRid,record,recLen);
    if(status!=OK)
        return status;
    DataPageInfo *info = (DataPageInfo *)record;
    this->dataPageId = info->pageId;
    Page *dp;
    MINIBASE_BM->pinPage(this->dataPageId,dp,0,_hf->fileName);
    this->dataPage = (HFPage *)dp;
    scanIsDone=0;
    this->nxtUserStatus= this->dataPage->firstRecord(this->userRid);

}

// *******************************************
// Retrieve the next data page.
Status Scan::nextDataPage(){
   RID nextRid;
   Status status = this->dirPage->nextRecord(this->dataPageRid,nextRid);
   if(status!=OK)
   {
      return nextDirPage();
   }
   MINIBASE_BM->unpinPage(this->dataPageId,0,_hf->fileName);
   char *record;
   int recLen;
   this->dirPage->returnRecord(nextRid,record,recLen);
    DataPageInfo *info = (DataPageInfo *)record;
    this->dataPageId = info->pageId;
    Page *pg;
    MINIBASE_BM->pinPage(this->dataPageId,pg,0,_hf->fileName);
    this->dataPage = (HFPage *)pg;
    this->nxtUserStatus= this->dataPage->firstRecord(this->userRid);

    return OK;
}

// *******************************************
// Retrieve the next directory page.
Status Scan::nextDirPage() {
    int currId = this->dirPageId;
    int index;
    for(int i=0;i<directoryPages.size();i++)
    {
        HFPage *pg = directoryPages[i];
        if(pg->page_no()==currId) {
            index = i;
            break;
        }
    }
    if(index==directoryPages.size()-1)
        return FAIL;
    MINIBASE_BM->unpinPage(currId,0,_hf->fileName);
    MINIBASE_BM->unpinPage(dataPage->page_no(),0,_hf->fileName);
    index++;
    this->dirPageId = directoryPages[index]->page_no();
    Page *pg = (Page *)directoryPages[index];
    MINIBASE_BM->pinPage(this->dirPageId,pg,0,_hf->fileName);
    HFPage *dirPage = directoryPages[index];
    dirPage->firstRecord(this->dataPageRid);
    char *record;
    int recLen;
    dirPage->returnRecord(this->dataPageRid,record,recLen);
    DataPageInfo *info = (DataPageInfo *)record;
    this->dataPageId = info->pageId;
    Page *dp;
    MINIBASE_BM->pinPage(info->pageId,dp,0,_hf->fileName);
    this->dataPage = (HFPage *)dp;
    this->nxtUserStatus=this->dataPage->firstRecord(this->userRid);

  // put your code here
  return OK;
}
