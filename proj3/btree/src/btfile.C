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

//Status BTreeFile::insert(const void *key, const RID rid) {
//  // put your code here
//
//  PageId rootId = headerPage->pageId;
//
//  if(headerPage->pageId == -1) //empty tree, no records
//  {
//      Page *page;
//      PageId pgId;
//      MINIBASE_BM->newPage(pgId,page,1);
//      headerPage->pageId = pgId;
//      BTLeafPage *leaf = (BTLeafPage *)page;
//
//      RID leafID;
//      Status status = leaf->insertRec(key,headerPage->keyType,rid,leafID);
//      MINIBASE_BM->unpinPage(pgId,true,fileName.c_str());
//      return status;
//  }
//  else
//  {
//      Page *rootPage;
//      MINIBASE_BM->pinPage(headerPage->pageId,rootPage,false);
//      if(headerPage->numLevels==0) //only root, one leaf
//      {
//          BTLeafPage *page = (BTLeafPage *)rootPage;
//          int recLen;
//          if(headerPage->keyType == attrInteger)
//              recLen = sizeof(int) + sizeof(RID);
//          else if(headerPage->keyType == attrString)
//          {
//              char *record = (char *)key;
//              recLen = strlen(record) + sizeof(RID);
//          }
//          RID entryId;
//          if(page->available_space()>recLen)
//          {
//             Status status =  page->insertRec(key,headerPage->keyType,rid,entryId);
//             MINIBASE_BM->unpinPage(headerPage->pageId,true,fileName.c_str());
//             return status;
//          }
//          else //split root page
//          {
//             ;
//              PageId firstId,secondId;
//              Page *p,*r;
//              MINIBASE_BM->newPage(firstId,p,1);
//              MINIBASE_BM->newPage(secondId,r,1);
//              BTIndexPage *parent = (BTIndexPage *)p;
//              BTLeafPage *right = (BTLeafPage *)r;
//              Status status = split_page(page,parent,right,LEAF);
//              headerPage->pageId = parent->page_no();//update root
//              headerPage->numLevels++;
//          }
//      }
//      else // root is an Index page
//      {
//           BTIndexPage *root = (BTIndexPage *)rootPage;
//           RID rid;
//           void *k;
//          int  j =0;
//           PageId pageNum;
//           while(j < headerPage->numLevels)
//           {
//               Status status = root->get_first(rid, k, pageNum);
//               int prevCompare = 0, compare = 0;
//               int foundPageId = -1;
//               while (status == OK)
//               {
//                   prevCompare = compare;
//                   compare = keyCompare(k, key, headerPage->keyType);
//                   if(compare==0)
//                   {
//                       foundPageId = pageNum;
//                       break;
//                   }
//                    if(compare*prevCompare <0)
//                    {
//                        foundPageId = pageNum;
//                        break;
//                    }
//
//
//                  status = root->get_next(rid,k,pageNum);
//               }
//               MINIBASE_BM->unpinPage(root->page_no(), false,fileName.c_str());
//               Page *rt;
//               MINIBASE_BM->pinPage(foundPageId,rt,0,this->fileName.c_str());
//               root = (BTIndexPage *)rt;
//               j++;
//           }
//      }
//
//  }
//  return OK;
//}

Status BTreeFile::insert(const void *key, const RID rid) {

    PageId pageId = headerPage->pageId;
    if(pageId==-1)
    {
        BTLeafPage *root;
        Page *pg;
        MINIBASE_BM->newPage(pageId,pg,1);
        root = (BTLeafPage *)pg;
        root->init(pageId);
        headerPage->pageId = pageId;
        MINIBASE_BM->unpinPage(pageId,0,fileName.c_str());
        char *childEntry = NULL;
        insert(headerPage->pageId,key,rid,LEAF,childEntry);
        if(childEntry!=NULL)
        {
            BTIndexPage *parent;
            PageId parentId;
            Page *parent_page;
            MINIBASE_BM->newPage(parentId,parent_page,1);
            parent = (BTIndexPage *)parent_page;

        }
    }
    else
    {
        if(headerPage->numLevels>0)
        {
            Page *pg;
            char *childEntry = NULL;
           // MINIBASE_BM->pinPage(pageId,pg,0,fileName.c_str());
            insert(pageId,key,rid,INDEX,childEntry);
            if(childEntry!=NULL)//split occurred
            {

            }
        }
        else
        {
            char *childEntry = NULL;
            insert(pageId,key,rid,LEAF,childEntry);
            if(childEntry!=NULL) //split occurred
            {

            }
        }

    }
}

void BTreeFile::insert(PageId pageId, const void *key, RID rid,nodetype type,char *child) {
    PageId pageNum;
    void *k;
    RID r;
    int prevCompare = 0, compare = 0;
    Page *pg;

    if(type == INDEX)
    {
        Status status = MINIBASE_BM->pinPage(pageId,pg,0,fileName.c_str());
        BTIndexPage *nodepointer;
        nodepointer = (BTIndexPage *)pg;
         status = nodepointer->get_first(r, k, pageNum);
        PageId foundPageId;
        char *matchingKey;
        int matchingLen;
        while (status == OK) {
            prevCompare = compare;
            compare = keyCompare(k, key, headerPage->keyType);
            if (compare == 0) {
                foundPageId = pageNum;
                matchingKey = create_key_index_record(k,foundPageId,matchingLen);
                break;
            }
            if (compare * prevCompare < 0) {
                foundPageId = pageNum;
                matchingKey = create_key_index_record(k,foundPageId,matchingLen);
                break;
            }
            status = nodepointer->get_next(r, k, pageNum);
        }


        insert(foundPageId,key,rid,INDEX,child);
        if(child==NULL) {
            MINIBASE_BM->unpinPage(nodepointer->page_no(), false, fileName.c_str());
            return;
        }
        else
        {
            int len;
          //  child = create_key_data_record(key,rid,len);
            RID recordID;
            if(nodepointer->available_space()>len)
            {
                nodepointer->insertRecord(headerPage->keyType,child,len,recordID);
                return;
            }
            else //split
            {
                BTIndexPage *secondPage;
                Page *pg;
                PageId pgId;
                MINIBASE_BM->newPage(pgId,pg,1);
                secondPage = (BTIndexPage *)pg;
                split_page(nodepointer,secondPage);
                RID smallestRID;
                PageId temp;
                void *tempKey;
                int recLen;
                secondPage->get_first(smallestRID,tempKey,temp);
                child = create_key_index_record(tempKey,temp,recLen);

                if(nodepointer->page_no() == headerPage->pageId)
                {
                    BTIndexPage *newRoot;
                    headerPage->numLevels++;
                    PageId newPgId;
                    Page *newPage;
                    MINIBASE_BM->newPage(newPgId,newPage,1);
                    newRoot = (BTIndexPage *)newPage;
                    RID tempId;
                    newRoot->insertRecord(headerPage->keyType,matchingKey,matchingLen,tempId);
                    newRoot->insertRecord(headerPage->keyType,child,recLen,tempId);
                    headerPage->pageId = newRoot->page_no();
                    MINIBASE_BM->unpinPage(newPgId,true,fileName.c_str());

                }
                MINIBASE_BM->unpinPage(pgId,true,fileName.c_str());
            }
        }
    }
    else // a leaf node
    {
        int recLen;
        char *record = create_key_data_record(key,rid,recLen);
        BTLeafPage *leaf;
        Page *lPage;
        RID r;
        Status status = MINIBASE_BM->pinPage(pageId,lPage,0,fileName.c_str());
        leaf = (BTLeafPage *)lPage;

        if(leaf->available_space()>recLen)
        {
            leaf->insertRec(key,headerPage->keyType,rid,r);
            child = NULL;
           status = MINIBASE_BM->unpinPage(pageId,true,fileName.c_str());
            return;
        }
        else //split leaf
        {
            BTLeafPage *newLeaf;
            BTIndexPage *newParent;
            Page *newPage,*newPt;
            PageId newPageId,newParentId;
            MINIBASE_BM->newPage(newPageId,newPage,1);
        //    MINIBASE_BM->newPage(newParentId,newPt,1);
         //   newParent = (BTIndexPage *)newPt;
            newLeaf = (BTLeafPage *)newPage;
            newLeaf->init(newPageId);
            Status status =split_page(leaf,newLeaf);
            if(status!=OK)
                return;
            leaf->setNextPage(newLeaf->page_no());
            newLeaf->setPrevPage(leaf->page_no());
            RID temprid,tempDataRID;
            void *tempKey;
            int rLen;
            if(headerPage->keyType == attrInteger){
                tempKey = (void *) new int[1];
            }
            newLeaf->get_first(temprid,tempKey,tempDataRID);
            child = create_key_data_record(tempKey,tempDataRID,rLen);
          //  newParent->insertKey(tempKey,headerPage->keyType,leaf->page_no(),temprid);
           // newParent->insertKey(tempKey,headerPage->keyType,newLeaf->page_no(),temprid);
            //if(leaf->page_no()==headerPage->pageId)
           // {
             //   headerPage->pageId = newParent->page_no();
            //}
            MINIBASE_BM->unpinPage(newPageId,true,fileName.c_str());
            MINIBASE_BM->unpinPage(pageId,true,fileName.c_str());
            //MINIBASE_BM->unpinPage(newParentId,true,fileName.c_str());
            return;
        }
    }

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

Status BTreeFile::split_page(BTIndexPage *page, BTIndexPage *first)
{
    int recordCount = page->numberOfRecords();
    int firstCount,secondCount;
    if(recordCount%2!=0)
    {
        firstCount = recordCount/2 +1;
        secondCount = recordCount/2;
    }
    else
    {
        firstCount = secondCount = recordCount/2;
    }
    RID rid;
    void *key;
    int i=0;
    PageId pageId;
    page->get_first(rid,key,pageId);
    while(i<firstCount)
    {
        if(headerPage->keyType == attrInteger){
            int *k = (int *)key;
            k=new int[1];
        }
        page->get_next(rid,key,pageId);
        i++;
        delete(key);
    }
    i=0;
    vector<RID> deleted;
    while(i<secondCount)
    {
        int recLen;
        RID recId;
        if(headerPage->keyType == attrInteger){
            int *k = (int *)key;
            k=new int[1];
        }
        Status status = page->get_next(rid,key,pageId);
        if(status!=OK)
            return status;
        deleted.push_back(rid);
        char *rec = create_key_index_record(key,pageId,recLen);
        first->insertRecord(headerPage->keyType,rec,recLen,recId);
        i++;
        delete(key);
    }
    for(int i=0;i<deleted.size();i++)
        page->deleteRecord(deleted[i]);
    return OK;

}

char* BTreeFile::create_key_data_record(const void *key, RID dataRId,int& recLen)
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

char* BTreeFile::create_key_index_record(const void *key, PageId pageNum, int &recLen)
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

Status BTreeFile::split_page(BTLeafPage *page, BTLeafPage *other) {
    int numRecords = page->numberOfRecords();
    int firstCount = 0,secondCount = 0;
    secondCount = numRecords/2;
    firstCount = numRecords - secondCount;
    vector<RID> deleted;
    int i=0;
    void *key;
    if(headerPage->keyType == attrInteger){
        key = (void *)(new int[1]);
    }

    RID rid,dataRID;
    Status status =page->get_first(rid,key,dataRID);
    while(i < firstCount)
    {
        int *k;
        if(headerPage->keyType == attrInteger){
            k = (int *)key;
            k=new int[1];
        }

        status = page->get_next(rid,key,dataRID);
        i++;
    }
    i=0;

    while(i<secondCount)
    {


        if(status!=OK)
        {

            break;

        }
        int recLen;
        //char *rec = create_key_data_record(key,dataRID,recLen);
        RID temp;
        status = other->insertRec(key,headerPage->keyType,dataRID,temp);
        if(status!=OK)
            return status;
        deleted.push_back(rid);
        status = page->get_next(rid,key,dataRID);
        i++;
    }
    int n = other->numberOfRecords();
    for(int i=0;i<deleted.size();i++) {
        status = page->deleteRecord(deleted[i]);
        if(status!=OK)
            return status;
    }
    return OK;
}
