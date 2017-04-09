/*
 * btfile.C - function members of class BTreeFile 
 * 
 * Johannes Gehrke & Gideon Glass  951022  CS564  UW-Madison
 * Edited by Young-K. Suh (yksuh@cs.arizona.edu) 03/27/14 CS560 Database Systems Implementation 
 */

#include <btfile.h>
#include "minirel.h"
#include "buf.h"
#include "db.h"
#include "new_error.h"
#include "btfile.h"
#include "btreefilescan.h"

// Define your error message here
const char* BtreeErrorMsgs[] = {
  // Possible error messages
  // _OK
  // CANT_FIND_HEADER
  // CANT_PIN_HEADER,
  // CANT_ALLOC_HEADER
  // CANT_ADD_FILE_ENTRY
  // CANT_UNPIN_HEADER
  // CANT_PIN_PAGE
  // CANT_UNPIN_PAGE
  // INVALID_SCAN
  // SORTED_PAGE_DELETE_CURRENT_FAILED
  // CANT_DELETE_FILE_ENTRY
  // CANT_FREE_PAGE,
  // CANT_DELETE_SUBTREE,
  // KEY_TOO_LONG
  // INSERT_FAILED
  // COULD_NOT_CREATE_ROOT
  // DELETE_DATAENTRY_FAILED
  // DATA_ENTRY_NOT_FOUND
  // CANT_GET_PAGE_NO
  // CANT_ALLOCATE_NEW_PAGE
  // CANT_SPLIT_LEAF_PAGE
  // CANT_SPLIT_INDEX_PAGE
};





static error_string_table btree_table( BTREE, BtreeErrorMsgs);

BTreeFile::BTreeFile (Status& returnStatus, const char *filename)
{
 // PageId headerPageId;
  Status status = MINIBASE_DB->get_file_entry(filename,headerPageId);
  if(status!=OK)
  {
    returnStatus=status;
    return;
  }
  Page *pg;
  returnStatus = MINIBASE_BM->pinPage(headerPageId,pg,0,filename);
  headerPage = (HeaderPage *)pg;
  this->fileName = filename;
  // put your code here
}

BTreeFile::BTreeFile (Status& returnStatus, const char *filename, 
                      const AttrType keytype,
                      const int keysize)
{
   // PageId headerPageId;
    Status status = MINIBASE_DB->get_file_entry(filename,headerPageId);
    this->fileName = filename;
    if(status!=OK) //index doesn't exist
    {
        Page *hPage;
        returnStatus = MINIBASE_BM->newPage(headerPageId,hPage,1);
        headerPage = (HeaderPage *)hPage;
        headerPage->keyType = keytype;
        headerPage->keyLength = keysize;
        headerPage->pageId = -1;
        headerPage->numLevels = 0;
        return;
    }
    else
    {
        Page *pg;
        returnStatus = MINIBASE_BM->pinPage(headerPageId,pg,0,filename);
        headerPage = (HeaderPage *)pg;
    }
  // put your code here
}

BTreeFile::~BTreeFile ()
{
   Page *pg = (Page *)headerPage;
   MINIBASE_BM->unpinPage(headerPageId,true,fileName.c_str());
  // put your code here
}

Status BTreeFile::destroyFile ()
{
  // put your code here
  Status status= MINIBASE_DB->delete_file_entry(fileName.c_str());
  return status;
}

Status BTreeFile::insert(const void *key, const RID rid) {
  // put your code here

  PageId rootId = headerPage->pageId;

  if(headerPage->pageId == -1)
  {
      Page *page;
      PageId pgId;
      MINIBASE_BM->newPage(pgId,page,1);
      headerPage->pageId = pgId;
      BTLeafPage *leaf = (BTLeafPage *)page;

      RID leafID;
      Status status = leaf->insertRec(key,headerPage->keyType,rid,leafID);
      MINIBASE_BM->unpinPage(pgId,TRUE,fileName.c_str());
      return status;
  }
  else
  {
      Page *rootPage;
      MINIBASE_BM->pinPage(headerPage->pageId,rootPage,false);
      if(headerPage->numLevels==0)
      {
          BTLeafPage *page = (BTLeafPage *)rootPage;
          int recLen;
          if(headerPage->keyType == attrInteger)
              recLen = sizeof(int) + sizeof(RID);
          else if(headerPage->keyType == attrString)
          {
              char *record = (char *)key;
              recLen = strlen(record) + sizeof(RID);
          }
          RID entryId;
          if(page->available_space()>recLen)
          {
             Status status =  page->insertRec(key,headerPage->keyType,rid,entryId);
             MINIBASE_BM->unpinPage(headerPage->pageId,true,fileName.c_str());
             return status;
          }
          else //split root page
          {
              BTIndexPage *page;

          }
      }
      else
      {

      }

  }
  return OK;
}

Status BTreeFile::Delete(const void *key, const RID rid) {
  // put your code here
  return OK;
}
    
IndexFileScan *BTreeFile::new_scan(const void *lo_key, const void *hi_key) {
  // put your code here
  return NULL;
}

int keysize(){
  // put your code here
  return 0;
}
