/*
 * btreefilescan.C - function members of class BTreeFileScan
 *
 * Spring 14 CS560 Database Systems Implementation
 * Edited by Young-K. Suh (yksuh@cs.arizona.edu) 
 */

#include <btfile.h>
#include "minirel.h"
#include "buf.h"
#include "db.h"
#include "new_error.h"
#include "btfile.h"
#include "btreefilescan.h"

/*
 * Note: BTreeFileScan uses the same errors as BTREE since its code basically 
 * BTREE things (traversing trees).
 */

BTreeFileScan::~BTreeFileScan()
{
  // put your code here
}


Status BTreeFileScan::get_next(RID & rid, void* keyptr)
{
  // put your code here
  void *key;
  RID temp;
  RID tempDataRID;
  if(type == attrInteger){
    key = new int[1];
  }
  Status status = currentPage->get_next(currentRID,key,tempDataRID);
  if(status!=OK)
  {
      PageId currentId = currentPage->page_no();
      PageId nextPageId = currentPage->getNextPage();
      if(nextPageId==-1)
        return DONE;
      Page *temp;
      MINIBASE_BM->unpinPage(currentId,true,file->get_fileName().c_str());
      MINIBASE_BM->pinPage(nextPageId,temp,0,file->get_fileName().c_str());
      currentPage = (BTLeafPage *)temp;
      RID tempId;
      status = currentPage->get_first(currentRID,key,tempDataRID);
    memcpy(keyptr,key,sizeof(int));
    memcpy(&rid,&tempDataRID,sizeof(RID));
      if(highVal!= NULL && keyCompare(keyptr,highVal,type)>0)
        return DONE;
      return status;
  }
  if(highVal!=NULL && keyCompare(key,highVal,type)>0)
    return DONE;
  rid = tempDataRID;
  memcpy(keyptr,key,sizeof(int));
  memcpy(&rid,&tempDataRID,sizeof(RID));
 // currentRID = temp;
  currentKey = key;
  return OK;
}

Status BTreeFileScan::delete_current()
{
  // put your code here
  Status status = currentPage->deleteRecord(currentRID);
  return status;
}


int BTreeFileScan::keysize() 
{
  // put your code here

  return keySize;
}

void BTreeFileScan::traverseToLowValLeaf() {
  PageId rootId = file->get_root();
  Page *root;
  MINIBASE_BM->pinPage(rootId,root,0,file->get_fileName().c_str());
  SortedPage *sortedPage = (SortedPage *)root;
  Page *leftMostLeafPage;
  PageId leftMostLeaf;
  if(sortedPage->get_type()==INDEX)
  {
      BTIndexPage *rootPage = (BTIndexPage *)sortedPage;
      PageId leftLink = rootPage->getLeftLink();

      Page *temp;
      while(leftLink!=-1)
      {
          MINIBASE_BM->pinPage(leftLink,temp,0,file->get_fileName().c_str());
          if(((SortedPage *)temp)->get_type()==LEAF)
          {
              leftMostLeaf = leftLink;
              leftMostLeafPage = temp;
              break;
          }
          leftLink = ((BTIndexPage *)temp)->getLeftLink();
          MINIBASE_BM->unpinPage(leftLink,true,file->get_fileName().c_str());
      }
  }
  else
  {
    leftMostLeaf = rootId;
    leftMostLeafPage = root;
  }
  currentPage = (BTLeafPage *)leftMostLeafPage;
  if(lowVal == NULL && highVal == NULL)
      fullIndexScan = true;

  if(fullIndexScan)
  {
    void *currKey;
    RID bullshit;
    if(type == attrInteger){
      currKey = new int[1];
    }
      Status  status = currentPage->get_first(currentRID,currKey,bullshit);
  }
  if(!fullIndexScan && lowVal!=NULL)
  {
      void *currKey;
      if(type == attrInteger){
        currKey = new int[1];
      }
      RID currRID,temp;
      Status status = currentPage->get_first(temp,currKey,currRID);

      while(status==OK)
      {
           currentRID = temp;
          if(keyCompare(currKey,lowVal,type)<0)
          {
              status = currentPage->get_next(temp,currKey,currRID);
          }
      }
      currentRID = temp;
  }
}


