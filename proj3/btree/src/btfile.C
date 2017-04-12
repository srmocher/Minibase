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

   if(headerPage->pageId ==-1)
   {
       PageId  rootId;
       Page *pg;
       MINIBASE_BM->newPage(rootId,pg,1);
       BTLeafPage *root = (BTLeafPage *)pg;
       root->init(rootId);
       headerPage->pageId = rootId;
       MINIBASE_BM->unpinPage(rootId,true,fileName.c_str());
       KeyDataEntry *child=NULL;
       PageId splitPageId;
       insert(rootId,key,rid,child,splitPageId);
       if(child!=NULL)
       {
            cout<<"test"<<endl;
       }
   }
    else
   {
       PageId pgId = headerPage->pageId;
       KeyDataEntry *entry=NULL;
       PageId splitPageId=-1;
       insert(pgId,key,rid,entry,splitPageId);
       if(entry!=NULL && splitPageId==headerPage->pageId)
       {
           Page *rootPage;
           PageId rootId;
           MINIBASE_BM->newPage(rootId,rootPage,1);
           BTIndexPage *root = (BTIndexPage *)rootPage;
           root->init(rootId);
           RID temp;
           cout<<"New Root PageId is "<<rootId<<endl;
           root->setLeftLink(headerPage->pageId);
          Status st = root->insertKey(&entry->key.intkey,headerPage->keyType,splitPageId,temp);
           headerPage->pageId = rootId;
           MINIBASE_BM->unpinPage(rootId,true,fileName.c_str());
       }
       else if(entry!=NULL && splitPageId!=headerPage->pageId)
       {
           Page *pg;
           RID t;
           MINIBASE_BM->pinPage(headerPage->pageId,pg,0,fileName.c_str());
           BTIndexPage *root = (BTIndexPage *)pg;
          Status status = root->insertKey(&entry->key.intkey,headerPage->keyType,entry->data.pageNo,t);
           MINIBASE_BM->unpinPage(headerPage->pageId,true,fileName.c_str());
       }
   }
    return OK;
}

void BTreeFile::insert(PageId &pageId, const void *key, RID rid,KeyDataEntry *&childEntry,PageId& splitPageId) {
    PageId pageNum;
    void *k;

    RID r;
    int prevCompare = 0, compare = 0;
    SortedPage *pg;
    Page *currPage;
    MINIBASE_BM->pinPage(pageId,currPage,0,fileName.c_str());
    pg = (SortedPage *)currPage;
    short type = pg->get_type();
    if(type == INDEX)
    {
        BTIndexPage *page = (BTIndexPage *)pg;
        PageId insertionPageId = get_page_no(page,key,headerPage->keyType);
        KeyDataEntry *newEntry = new KeyDataEntry();
        MINIBASE_BM->unpinPage(pageId,true,fileName.c_str());
        insert(insertionPageId,key,rid,childEntry,splitPageId);
        if(childEntry==NULL) return;
        if(childEntry != NULL)
        {
            if(page->available_space()>sizeof(KeyDataEntry))
            {
                int entryLen = get_key_data_length(key,headerPage->keyType,INDEX);
                void *k;
                if(headerPage->keyType == attrInteger){
                    k = new int[1];
                }
                Datatype *dt = new Datatype();
                RID temp;
                get_key_data(k,dt,childEntry,entryLen,INDEX);
                Status st = page->insertKey(k,headerPage->keyType,dt->pageNo,temp);
                childEntry = NULL;
                return;
            }
            else
            {
                BTIndexPage *rightSibling;
           //     rightSibling->setLeftLink(1);
                PageId rightId;
                Page *rightPg;
                MINIBASE_BM->newPage(rightId,rightPg,1);
                rightSibling = (BTIndexPage *)rightSibling;
                rightSibling->init(rightId);
                split_page(page,rightSibling);
                void *k;
                RID tempRID;
                PageId rightSmallestId;
                rightSibling->get_first(tempRID,k,rightSmallestId);
                    childEntry = new KeyDataEntry();
                    memcpy(&(childEntry->key.intkey),k,sizeof(int));
                    PageId rightPageId = rightSibling->page_no();
                    memcpy(&childEntry->data.pageNo,&rightPageId,sizeof(PageId));
                rightSibling->deleteRecord(tempRID);
                if(page->page_no() == headerPage->pageId)//root split
                {
                    Page *newRoot;
                    PageId newId;
                    MINIBASE_BM->newPage(newId,newRoot,1);
                    BTIndexPage *root;
                    root = (BTIndexPage *)newRoot;
                    root->init(newId);
                    RID temp;
                    root->insertKey(&childEntry->key.intkey,headerPage->keyType,childEntry->data.pageNo,temp);
                    root->setLeftLink(page->page_no());
                    headerPage->pageId = root->page_no();
                    MINIBASE_BM->unpinPage(newId,true,fileName.c_str());
                }
                MINIBASE_BM->unpinPage(pageId,true,fileName.c_str());
             return;
            }
        }
    }
    else // a leaf node
    {
      int entryLen = get_key_data_length(key,headerPage->keyType,LEAF);
      BTLeafPage *leaf = (BTLeafPage *)pg;
        if(leaf->available_space() > entryLen)
        {
            RID tempId;
            leaf->insertRec(key,headerPage->keyType,rid,tempId);
            cout<<"Key,PageId - "<<(*(int *)key)<<","<<leaf->page_no()<<endl;
            childEntry = NULL;
            MINIBASE_BM->unpinPage(leaf->page_no(),true,fileName.c_str());
            return;
        }
        else
        {

            BTLeafPage *rightSibling;
            Page *rightPage;
            PageId rightId;
            MINIBASE_BM->newPage(rightId,rightPage,1);
            rightSibling = (BTLeafPage *)rightPage;
            rightSibling->init(rightId);
            split_page(leaf,rightSibling);
            childEntry = new KeyDataEntry();
            void *rightKey;
            if(headerPage->keyType==attrInteger){
                rightKey = new int[1];
            }
            RID rightRID,temp;
            splitPageId = leaf->page_no();
            rightSibling->get_first(temp,rightKey,rightRID);

                memcpy(&childEntry->key.intkey, rightKey, sizeof(int));
                PageId tempPageId = leaf->page_no();
                memcpy(&childEntry->data.rid, &tempPageId, sizeof(RID));
            //}
         //   cout<<"Splitting leaf and the key going to parent is "<<(*(int *)rightKey)<<endl;
            if(leaf->getNextPage()!=-1) //has siblings already
            {
                rightSibling->setNextPage(leaf->getNextPage());
            }
            leaf->setNextPage(rightSibling->page_no());
            rightSibling->setPrevPage(leaf->page_no());
            if(keyCompare(key,rightKey,headerPage->keyType)<0){
               Status st = leaf->insertRec(key,headerPage->keyType,rid,temp);
                cout<<"Key,PageId - "<<(*(int *)key)<<","<<leaf->page_no()<<endl;
                if(st!=OK)
                    cout<<"Error inserting"<<endl;
            }
            else{
               Status st = rightSibling->insertRec(key,headerPage->keyType,rid,temp);
                cout<<"Key,PageId - "<<(*(int *)key)<<","<<rightSibling->page_no()<<endl;
                if(st!=OK)
                    cout<<"Error inserting"<<endl;
            }
            MINIBASE_BM->unpinPage(rightId,true,fileName.c_str());
        }
        return;
    }

}



Status BTreeFile::Delete(const void *key, const RID rid) {
  // put your code here
  return OK;
}
    
IndexFileScan *BTreeFile::new_scan(const void *lo_key, const void *hi_key) {
  // put your code here
    BTreeFileScan *scan = new BTreeFileScan(this);
    scan->lowVal = lo_key;
    scan->highVal = hi_key;
    scan->keySize = headerPage->keyLength;
    scan->type = headerPage->keyType;
    scan->traverseToLowValLeaf();
     return scan;
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
    for(int i=deleted.size();i>=0;i--) {
       Status status = page->deleteRecord(deleted[i]);
        if(status!=OK)
        {
            cout<<"Error deleting"<<endl;
        }
    }
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
    Status status = page->get_first(rid,key,dataRID);
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
    for(int i=deleted.size()-1;i>=0;i--) {
        status = page->deleteRecord(deleted[i]);
        if(status!=OK)
            return status;
    }
    n = page->numberOfRecords();
    return OK;
}




PageId BTreeFile::get_page_no(BTIndexPage *page,const void *key,AttrType type){
    void *k;
    if(type == attrInteger){
        k=new int[1];
    }
    RID rid;
    PageId pageNum;
    Status status = page->get_first(rid,k,pageNum);
    if(keyCompare(key,k,type)<0) {
        return pageNum;
    }
    int prevCompare = 0,compare =0;
    PageId prevPageId;
    while(status == OK){
        prevCompare = compare;
        prevPageId = pageNum;
        compare = keyCompare(key,k,type);
        if(compare == 0){
            return pageNum;
        }
        if(prevCompare*compare<0){
            return prevPageId;
        }
        status = page->get_next(rid,k,pageNum);
    }
    return prevPageId;
}

