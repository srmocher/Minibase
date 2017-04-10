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

  if(headerPage->pageId == -1) //empty tree, no records
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
      if(headerPage->numLevels==0) //only root, one leaf
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
              BTLeafPage *first,*second;
              PageId firstId,secondId;
              MINIBASE_BM->newPage(firstId,(Page *)first,1);
              MINIBASE_BM->newPage(secondId,(Page *)second,1);
              Status status = split_page(page,first,second,LEAF);

          }
      }
      else // root is an Index page
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

int BTreeFile::keysize(){
  // put your code here
  return headerPage->keyLength;
}

Status BTreeFile::split_page(SortedPage *page, SortedPage *first,SortedPage *second,nodetype type)
{
    int recordCount = page->numberOfRecords();
    int firstCount,secondCount;
    if(recordCount%2==0)
    {
        firstCount = recordCount/2;
        secondCount = recordCount/2 - 1;
    }
    else
    {
        firstCount = secondCount = recordCount/2;
    }

    if(type == LEAF) // splitting a leaf page
    {
        RID rid;
        BTLeafPage *leafPage = (BTLeafPage *)page;
        BTIndexPage *firstPage = (BTIndexPage *) first;
        BTIndexPage *secondPage = (BTIndexPage *) second;
        int i=0;
        void *key;
        RID dataRID;
        Status getStatus =leafPage->get_first(rid,key,dataRID);
        int recLen;
        while(i<firstCount)
        {
            RID newRid;
            char *rec = create_key_data_record(key,dataRID,recLen);
            firstPage->insertRecord(headerPage->keyType,rec,recLen,newRid);
            leafPage->get_next(rid,key,dataRID);
            i++;
        }
        char *middleRecord;
        leafPage->get_next(rid,key,dataRID);
        PageId first = firstPage->page_no();
        middleRecord = create_key_index_record(key,first,recLen);
        int middleLength = recLen,middleDataRecordLength;
        char *middleDataRecord = create_key_data_record(key,dataRID,middleDataRecordLength);
        RID temp;
        firstPage->insertRecord(headerPage->keyType,middleDataRecord,middleDataRecordLength,temp);
        i =0;
        while (i<secondCount)
        {
            RID newRid;
            char *rec= create_key_data_record(key,dataRID,recLen);
            secondPage->insertRecord(headerPage->keyType,rec,recLen,newRid);
            leafPage->get_next(rid,key,dataRID);
            i++;
        }


        //delete all records in node to be split and add the middle record
        Status status = leafPage->get_first(rid,key,dataRID);
        while (status == OK)
        {
            leafPage->deleteRecord(rid);
            status = leafPage->get_next(rid,key,dataRID);
        }
        leafPage->insertRecord(headerPage->keyType,middleRecord,middleLength,rid);
    }
    else //splitting a non-leaf page
    {
        BTIndexPage *indexPage = (BTIndexPage *)page;
        BTIndexPage *firstPage = (BTIndexPage *) first;
        BTIndexPage *secondPage = (BTIndexPage *) second;
    }
}

char* BTreeFile::create_key_data_record(void *key, RID dataRId,int& recLen)
{
    if(headerPage->keyType==attrInteger){
        int recordLength = sizeof(int) + sizeof(dataRId);
        char *record = new char[recordLength];
        int *k = (int *)key;
        recLen = recordLength;
        memcpy(record,k,sizeof(int));


        memcpy(record+ sizeof(int),(char *)&dataRId,sizeof(RID));
        return record;
    }
    else
    {
        char *k = (char *)key;
        int recordLength = strlen(k)+sizeof(RID);
        recLen = recordLength;
        char *record = new char[recordLength];
        memcpy(record,k,strlen(k));
        memcpy(record+strlen(k),(char *)&dataRId,sizeof(RID));
        return record;
    }
}

char* BTreeFile::create_key_index_record(void *key, PageId pageNum, int &recLen)
{
    if(headerPage->keyType == attrInteger)
    {
        int *k = (int *)key;
        recLen = sizeof(int) + sizeof(PageId);
        char *rec = new char[recLen];
        memcpy(rec,k,sizeof(int));
        memcpy(rec+sizeof(int),(char *)&pageNum,sizeof(PageId));
        return rec;
    }
    else
    {
        char *k = (char *)key;
        recLen = strlen(k) + sizeof(PageId);
        char *rec = new char[recLen];
        memcpy(rec,k,strlen(k));
        memcpy(rec+strlen(k),(char *)&pageNum,sizeof(PageId));
        return rec;
    }
}
