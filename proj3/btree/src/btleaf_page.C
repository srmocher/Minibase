/*
 * btleaf_page.C - implementation of class BTLeafPage
 *
 * Johannes Gehrke & Gideon Glass  951016  CS564  UW-Madison
 * Edited by Young-K. Suh (yksuh@cs.arizona.edu) 03/27/14 CS560 Database Systems Implementation 
 */

#include "btleaf_page.h"
#include <cstdio>
#include <cstring>

const char* BTLeafErrorMsgs[] = {
// OK,
// Insert Record Failed,
};
static error_string_table btree_table(BTLEAFPAGE, BTLeafErrorMsgs);
   
/*
 * Status BTLeafPage::insertRec(const void *key,
 *                             AttrType key_type,
 *                             RID dataRid,
 *                             RID& rid)
 *
 * Inserts a key, rid value into the leaf node. This is
 * accomplished by a call to SortedPage::insertRecord()
 * The function also sets up the recPtr field for the call
 * to SortedPage::insertRecord() 
 * 
 * Parameters:
 *   o key - the key value of the data record.
 *
 *   o key_type - the type of the key.
 * 
 *   o dataRid - the rid of the data record. This is
 *               stored on the leaf page along with the
 *               corresponding key value.
 *
 *   o rid - the rid of the inserted leaf record data entry.
 */

Status BTLeafPage::insertRec(const void *key,
                              AttrType key_type,
                              RID dataRid,
                              RID& rid)
{
  Status status;
  if(key_type == attrString){
    char *k = (char *)key;
      int len = get_key_length(key,key_type);
    char *record = new char[len+sizeof(RID)];
      memcpy(record,k,len);
      memcpy(record+len,&dataRid,sizeof(RID));
    status = SortedPage::insertRecord(key_type,record,len+sizeof(RID),rid);

  }
  else if(key_type == attrInteger){

    int keyDataLength = sizeof(int) + sizeof(RID);
    int *k = (int *)key;
    char *record = new char[keyDataLength];
    memcpy(record,key,sizeof(int));
    memcpy(record+sizeof(int),&dataRid,sizeof(RID));

    status = SortedPage::insertRecord(key_type,record,keyDataLength,rid);
  } else
    return DONE;
  // put your code here
  return status;
}

/*
 *
 * Status BTLeafPage::get_data_rid(const void *key,
 *                                 AttrType key_type,
 *                                 RID & dataRid)
 *
 * This function performs a binary search to look for the
 * rid of the data record. (dataRid contains the RID of
 * the DATA record, NOT the rid of the data entry!)
 */

Status BTLeafPage::get_data_rid(void *key,
                                AttrType key_type,
                                RID & dataRid)
{
    Status status;
    if(key_type == attrInteger){
        int *k = (int *)key;

        RID rid,nextRid;
        int *key_val;
        status = get_first(rid,key_val,nextRid);
        if(status!=OK)
            return status;
        int *key_cast_val = (int *)key_val;
        if(*key_cast_val == *k)
        {
            dataRid = nextRid;
            return OK;
        }
        status = get_next(rid,key_val,nextRid);
        while(status == OK)
        {
            key_cast_val = (int *)key_val;
            if(*key_cast_val == *k)
            {
                dataRid = nextRid;
                return OK;
            }
            status = get_next(rid,key_val,nextRid);
        }

    } else if(key_type == attrString){

        char *k = (char *)key;
        RID rid,nextRid;
        char *key_val;
        status = get_first(rid,key_val,nextRid);
        if(status!=OK)
            return status;
        if(strcmp(key_val,k)==0)
        {
            dataRid = nextRid;
            return OK;
        }
        status = get_next(rid,key_val,nextRid);
        while(status == OK)
        {
            if(strcmp(key_val,k)==0){
                dataRid = nextRid;
                return OK;
            }
            status = get_next(rid,key_val,nextRid);
        }
    }
    // put your code here
    return DONE;
}

/* 
 * Status BTLeafPage::get_first (const void *key, RID & dataRid)
 * Status BTLeafPage::get_next (const void *key, RID & dataRid)
 * 
 * These functions provide an
 * iterator interface to the records on a BTLeafPage.
 * get_first returns the first key, RID from the page,
 * while get_next returns the next key on the page.
 * These functions make calls to RecordPage::get_first() and
 * RecordPage::get_next(), and break the flat record into its
 * two components: namely, the key and datarid. 
 */
Status BTLeafPage::get_first (RID& rid,
                              void *key,
                              RID & dataRid)
{
    slot_t *currSlot = this->slot;
    int i=0;
    while(currSlot->offset==-1)
    {
        currSlot = (slot_t*)(data+i*sizeof(slot_t));
        i++;
    }
    rid.slotNo = i;
    rid.pageNo = curPage;

    int offset = currSlot->offset;
    int length = currSlot->length;

    char *record = new char[length];
    memcpy(record,data+offset,length);

    int keySize = length - sizeof(RID);
    if(keySize == sizeof(int))
    {
     //   int *k = new int[1];
        memcpy(key,data+offset,sizeof(int));
      //  key = (void *)k;
        memcpy(&dataRid,data+offset+keySize,sizeof(RID));
    }
    else
    {

        memcpy(key,data+offset,20);
       // key = (void *)k;
        memcpy(&dataRid,data+offset+20,sizeof(RID));
    }  // put your code here
  return OK;
}

Status BTLeafPage::get_next (RID& rid,
                             void *key,
                             RID & dataRid)
{
    RID nextRid;
    int currSlotNo = rid.slotNo;
    int i=0;
    slot_t *temp = this->slot;
    if(currSlotNo+1 > this->slotCnt)
        return NOMORERECS;
    slot_t *nextSlot = (slot_t*)(data + currSlotNo*sizeof(slot_t));
    if(nextSlot->offset==-1) {
     //  while (nextSlot->offset == -1 && currSlotNo <= this->slotCnt) {

               nextSlot = (slot_t*)(data + (currSlotNo+1)*sizeof(slot_t));
                 currSlotNo++;
       //  }
        //if(currSlotNo > this->slotCnt)
          //  return NOMORERECS;
        //currSlotNo--;
    }
    nextRid.pageNo = rid.pageNo;
    nextRid.slotNo = currSlotNo+1;
    rid = nextRid;
    int offset = nextSlot->offset;
    int length = nextSlot->length;


    int keyLength = length - sizeof(RID);
    if(keyLength == sizeof(int))
    {
      //  key = (void *)(new int[1]);
       memcpy(key,data+offset,sizeof(int));
       memcpy(&dataRid,data+offset+sizeof(int),sizeof(RID));
    } else
    {

        memcpy(key,data+offset,20);
        //key = (void *)k;
        memcpy(&dataRid,data+offset+20,sizeof(RID));
    }
    return OK;
}
