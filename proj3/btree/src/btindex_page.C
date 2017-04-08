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
    int length = get_key_length(key,key_type);
    char keyValue[MAX_KEY_SIZE1];


    for(int i=0;i<length;i++)
      keyValue[i]=k[i];

    for(int i=length;i<MAX_KEY_SIZE1;i++)
      keyValue[i]=' ';

    int dataLength = sizeof(PageId)+MAX_KEY_SIZE1;
    char *record = new char[dataLength];
    for(int i=0;i<MAX_KEY_SIZE1;i++)
      record[i]=keyValue[i];

    memcpy(record+MAX_KEY_SIZE1,(void *)pageNo,sizeof(PageId));
    status = SortedPage::insertRecord(key_type,record,dataLength,rid);

  }
  else if(key_type == attrInteger){

    int keyDataLength = sizeof(int) + sizeof(PageId);
    int k = (int)key;
    char *record = new char[keyDataLength];
    memcpy(record,key,sizeof(int));
    memcpy(record+sizeof(int),(void *)pageNo,sizeof(PageId));

    status = SortedPage::insertRecord(key_type,record,keyDataLength,rid);
  } else
    return DONE;
  // put your code here
  return status;
}

Status BTIndexPage::deleteKey (const void *key, AttrType key_type, RID& curRid)
{


  // put your code here
  return OK;
}

Status BTIndexPage::get_page_no(const void *key,
                                AttrType key_type,
                                PageId & pageNo)
{
  // put your code here
  return OK;
}

    
Status BTIndexPage::get_first(RID& rid,
                              void *key,
                              PageId & pageNo)
{

    slot_t *currSlot = this->slot;
    int i=0;
    while(currSlot->offset==-1)
    {
        currSlot = (slot_t *)data+i*sizeof(slot_t);
        i++;
    }
    rid.slotNo = i;
    rid.pageNo = curPage;

    int offset = currSlot->offset;
    int length = currSlot->length;

    char *record = new char[length];

    for(int i=0;i<length;i++)
      record[i] = data[offset+i];

    if(type == attrInteger){
      int *num = new int[1];
      memcpy(num,record,sizeof(int));
      PageId *pageNum = new PageId[1];
      key = (void *)num;
      memcpy(pageNum,record+sizeof(int),sizeof(PageId));
      pageNo = pageNum[0];
    }
    else if(type == attrString){
      char *k = new char[MAX_KEY_SIZE1];
      memcpy(k,record,MAX_KEY_SIZE1);
      PageId *pageNum = new PageId[1];
      key = (void *)k;
      memcpy(pageNum,record+MAX_KEY_SIZE1,length-MAX_KEY_SIZE1);
      pageNo = pageNum[0];
    }
  // put your code here
  return OK;
}

Status BTIndexPage::get_next(RID& rid, void *key, PageId & pageNo)
{
  // put your code here

  RID nextRid;
  Status status = HFPage::nextRecord(rid,nextRid);
  if(status!=OK)
    return NOMORERECS;
  int slotNo = nextRid.slotNo;
  int length,offset;
  if(slotNo == 0)
  {
      slot_t *current = this->slot;
      length = current->length;
      offset = current->offset;
  }
  else
  {
      int i=0;
      slot_t *current = this->slot;
      while(true)
      {
          if(i==slotNo)
            break;
          current = (slot_t *)data + i*sizeof(slot_t);
        i++;
      }
      length = current->length;
      offset = current->offset;
  }

  char *record = new char[length];
  memcpy(record,data+offset,length);

  char *k = new char[MAX_KEY_SIZE1];
  memcpy(k,record,MAX_KEY_SIZE1);

  key = (void *)k;
  PageId *page =new PageId[1];
  memcpy(page,record+MAX_KEY_SIZE1,sizeof(PageId));
  pageNo = page[0];
  return OK;
}
