/*
 * sorted_page.C - implementation of class SortedPage
 *
 * Johannes Gehrke & Gideon Glass  951016  CS564  UW-Madison
 * Edited by Young-K. Suh (yksuh@cs.arizona.edu) 03/27/14 CS560 Database Systems Implementation 
 */

#include <vector>
#include <algorithm>
#include <cstring>

#include "sorted_page.h"
#include "btindex_page.h"
#include "btleaf_page.h"


class SlotData
{
   public:
    char *stringData;
    int *intData;
    AttrType type;
    int slotNo;
    int length;
    int offset;
    SlotData(){intData = new int[1];}
    bool operator<(const SlotData &other) const{
         if(type == attrString)
          return strcmp(stringData,other.stringData) < 0;
         else
             return *intData < *(other.intData);
    }
};



const char* SortedPage::Errors[SortedPage::NR_ERRORS] = {
  //OK,
  //Insert Record Failed (SortedPage::insertRecord),
  //Delete Record Failed (SortedPage::deleteRecord,
};


/*
 *  Status SortedPage::insertRecord(AttrType key_type, 
 *                                  char *recPtr,
 *                                    int recLen, RID& rid)
 *
 * Performs a sorted insertion of a record on an record page. The records are
 * sorted in increasing key order.
 * Only the  slot  directory is  rearranged.  The  data records remain in
 * the same positions on the  page.
 *  Parameters:
 *    o key_type - the type of the key.
 *    o recPtr points to the actual record that will be placed on the page
 *            (So, recPtr is the combination of the key and the other data
 *       value(s)).
 *    o recLen is the length of the record to be inserted.
 *    o rid is the record id of the record inserted.
 */

Status SortedPage::insertRecord (AttrType key_type,
                                 char * recPtr,
                                 int recLen,
                                 RID& rid)
{
  // put your code here

//  if(key_type !=type)
 //   return DONE;
  Status status = HFPage::insertRecord(recPtr,recLen,rid);

//  if(status == OK && this->slotCnt==0)
  //  return OK;

    if(status!=OK)
        return status;
  slot_t *current = this->slot;
  int i=0;

  vector<SlotData> slotsInfo;
  int j=0;
//    cout<<"Num records:"<<this->numberOfRecords()<<endl;
  while(i < this->numberOfRecords())
  {
      SlotData slotData;
      slotData.slotNo = i;


      int offset = current->offset;
      int length = current->length;
      slotData.length = length;
      slotData.offset = offset;
      if(key_type == attrString)
      {
          slotData.type = attrString;
        if(recLen > MAX_KEY_SIZE1)
        {
            char *rec = new char[MAX_KEY_SIZE1];
            memcpy(rec,data+offset,MAX_KEY_SIZE1);
            slotData.stringData = rec;
        }
        else
        {
            char *rec = new char[recLen];
            memcpy(rec,data+offset,recLen);
            slotData.stringData = rec;
        }
      }
      else
      {
          slotData.type = attrInteger;
         memcpy(slotData.intData,data+offset,sizeof(int));

      }
      slotsInfo.push_back(slotData);
      current = (slot_t*)(data+j*sizeof(slot_t));

      i++;
      j++;
  }
  rid.slotNo = this->slotCnt;
  std::sort(slotsInfo.begin(),slotsInfo.end());
   // if(*(int *)key)
  //  if(slotsInfo.size()>65){
       // cout<<slotsInfo.size()<<endl;
    //}
//    cout<<"after sorting"<<endl;
//    for(int i=0;i<slotsInfo.size();i++)
//    {
//        cout<<slotsInfo[i].offset<<endl;
//    }
  //printing sorted data for verification

  current =this->slot;
  i=0;
 j=0;
  while(i<slotsInfo.size())
  {
        if(current->offset==-1)
            break;
      current->offset = slotsInfo.at(i).offset;
     current->length = slotsInfo.at(i).length;
      i++;
      current = (slot_t*)(data+j*sizeof(slot_t));
      j++;
  }

//    current = this->slot;
//    cout<<"after insertion"<<endl;
//    i =0;
//    while(i <= this->slotCnt)
//    {
//        int offset = current->offset;
//        int *k = new int[1];
//        memcpy(k,data+offset,sizeof(int));
//        cout << *k<<endl;
//        current =  (slot_t*)(data+i*sizeof(slot_t));
//        i++;
//    }
  return OK;
}


/*
 * Status SortedPage::deleteRecord (const RID& rid)
 *
 * Deletes a record from a sorted record page. It just calls
 * HFPage::deleteRecord().
 */

Status SortedPage::deleteRecord (const RID& rid)
{
  // put your code here
  //   this->usedPtr = MAX_SPACE - DPFIXED;
//    this->slotCnt--;
//  return status;
   Status status = HFPage::deleteRecord(rid);
   // this->slotCnt--;
   // this->freeSpace = this->freeSpace+ sizeof(slot_t);

   // this->slotCnt--;

    return status;
}

int SortedPage::numberOfRecords()
{
  // put your code here
  int i=0;
  slot_t *current = this->slot;
  int numRecords = 0;
  while(i <= this->slotCnt)
  {
      if(current->offset!=-1)
          numRecords++;
      else
          break;
      current = (slot_t*)(data + i*sizeof(slot_t));
      i++;
  }
  return numRecords;
}
