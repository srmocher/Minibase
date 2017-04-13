/*
 * btindex_page.C - implementation of class BTIndexPage
 *
 * Johannes Gehrke & Gideon Glass  951016  CS564  UW-Madison
 * Edited by Young-K. Suh (yksuh@cs.arizona.edu) 03/27/14 CS560 Database Systems Implementation 
 */

#include <cstdio>
#include <cstring>
#include "btindex_page.h"

// Define your Error Messge here
const char* BTIndexErrorMsgs[] = {
  //Possbile error messages,
  //OK,
  //Record Insertion Failure,
};

static error_string_table btree_table(BTINDEXPAGE, BTIndexErrorMsgs);

Status BTIndexPage::insertKey (const void *key,
                               AttrType key_type,
                               PageId pageNo,
                               RID& rid)
{
  Status status;
  if(key_type == attrString){
    char *k = (char *)key;
    int length  = get_key_length(key,key_type);
      char *record = new char[length+sizeof(PageId)];
      memcpy(record,k,length);
      memcpy(record+length,&pageNo,sizeof(PageId));
    status = SortedPage::insertRecord(key_type,record,length+sizeof(PageId),rid);

  }
  else if(key_type == attrInteger){

    int keyDataLength = sizeof(int) + sizeof(PageId);
    int *k = (int *)key;
    char *record = new char[keyDataLength];
    memcpy(record,key,sizeof(int));
    memcpy(record+sizeof(int),(void *)&pageNo,sizeof(PageId));

    status = SortedPage::insertRecord(key_type,record,keyDataLength,rid);
  } else
    return DONE;
  // put your code here
  return status;
}

Status BTIndexPage::deleteKey (const void *key, AttrType key_type, RID& curRid)
{
  Status status;
   int *givenkey = (int *)key;
  RID r;
  void *k;
  PageId p;

  if(key_type == attrInteger){
    status = get_first(r,k,p);
      if(status!=OK)
          return status;

      int *keyval = (int *)k;
      if(*keyval == *givenkey)
      {
          curRid = r;
          status = SortedPage::deleteRecord(curRid);
          return status;
      }
      status = get_next(r,k,p);
      while(status == OK)
      {
          int *keyval = (int *)k;
          if(*keyval == *givenkey)
          {
              curRid = r;
              status = SortedPage::deleteRecord(curRid);
              return status;
          }
          status = get_next(r,k,p);
      }
  }
  else if(key_type == attrString){
      char *givenkey = (char *)key;
      RID r;
      char *k;
      PageId p;
      Status  status;
      status = get_first(r,k,p);
      if(status!=OK)
          return status;
      if(strcmp(k,givenkey)==0)
      {
          curRid = r;
          status = SortedPage::deleteRecord(curRid);
          return status;
      }
      status = get_next(r,k,p);
      while(status == OK)
      {
          if(strcmp(k,givenkey)==0){
              curRid = r;
              status = SortedPage::deleteRecord(curRid);
              return status;
          }
          status = get_next(r,k,p);
      }
  }
  // put your code here
  return DONE;
}

Status BTIndexPage::get_page_no(const void *key,
                                AttrType key_type,
                                PageId & pageNo)
{

  if(key_type == attrInteger){
    int *k = (int *)key;

    RID rid,nextRid;
    void *key_val;
    PageId pageNum;
    Status status = get_first(rid,key_val,pageNum);
    if(status!=OK)
      return status;
    int *key_cast_val = (int *)key_val;
    if(*key_cast_val == *k)
    {
        pageNo =pageNum;
        return OK;
    }
    status = get_next(rid,key_val,pageNum);
    while(status == OK)
    {
        key_cast_val = (int *)key_val;
        if(*key_cast_val == *k)
        {
          pageNo =pageNum;
          return OK;
        }
        status = get_next(rid,key_val,pageNum);
    }

  } else if(key_type == attrString){

    char *k = (char *)key;
    RID rid;
    char *key_val;
    PageId pageNum;
    Status  status = get_first(rid,key_val,pageNum);
    if(status!=OK)
      return status;
    if(strcmp(key_val,k)==0)
    {
      pageNo=pageNum;
      return OK;
    }
    status = get_next(rid,key_val,pageNum);
    while(status == OK)
    {
       if(strcmp(key_val,k)==0){
         pageNo = pageNum;
         return OK;
       }
      status = get_next(rid,key_val,pageNum);
    }
  }
  // put your code here
  return DONE;
}

    
Status BTIndexPage::get_first(RID& rid,
                              void *key,
                              PageId & pageNo)
{

    slot_t *currSlot = this->slot;
    int i=0;
    while(currSlot->offset==-1)
    {
        currSlot = (slot_t *)(data+i*sizeof(slot_t));
        i++;
    }
    rid.slotNo = i;
    rid.pageNo = curPage;

    int offset = currSlot->offset;
    int length = currSlot->length;

    char *record = new char[length];
    memcpy(record,data+offset,length);
    int keySize = length - sizeof(PageId);
    if(keySize == sizeof(int))
    {
       // int *k = new int[1];
        memcpy(key,data+offset,sizeof(int));
     //   cout<<(*(int *)key)<<" is first key"<<endl;
      //  key = (void *)k;
        memcpy(&pageNo,data+offset+keySize,sizeof(PageId));
    }
    else
    {
      //  char *k = new char[keySize];
        memcpy(key,data+offset,keySize);
        //key = (void *)k;
        memcpy(&pageNo,data+offset+keySize,sizeof(PageId));
    }
  // put your code here
  return OK;
}

Status BTIndexPage::get_next(RID& rid, void *key, PageId & pageNo)
{
  // put your code here

    RID nextRid;
    int currSlot = rid.slotNo;
    if(currSlot+1 > this->slotCnt)
    {
        return NOMORERECS;
    }
    nextRid.pageNo = rid.pageNo;
    nextRid.slotNo = rid.slotNo+1;
    int nextSlot = currSlot+1;
    slot_t *next = (slot_t *)(data+currSlot*sizeof(slot_t));
    int offset = next->offset;
    int length = next->length;
    char *record = new char[length];
    memcpy(record,data+offset,length);

    int keySize = length - sizeof(PageId);
    if(keySize == sizeof(int))
    {
      //  int *k = new int[1];
        memcpy(key,data+offset,sizeof(int));
      //  key = (void *)k;
        memcpy(&pageNo,data+offset+keySize,sizeof(PageId));
    }
    else
    {
    //    char *k = new char[keySize];
        memcpy(key,data+offset,keySize);
     //   key = (void *)k;
        memcpy(&pageNo,data+offset+keySize,sizeof(PageId));
    }
    rid = nextRid;
  return OK;
}
