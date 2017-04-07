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
    int intData;
    AttrType type;
    int slotNo;
    int length;
    int offset;
    SlotData();
    bool operator<(const SlotData &other) const{
         if(type == attrString)
          return strcmp(stringData,other.stringData) < 0;
         else
             return intData < other.intData;
    }
};

SlotData::SlotData() {}


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

  if(key_type !=type)
    return DONE;
  Status status = HFPage::insertRecord(recPtr,recLen,rid);

  if(status == OK && this->slotCnt==0)
    return OK;

  slot_t *current = this->slot;
  int i=0;
  vector<SlotData> slotsInfo;

  while(i <= this->slotCnt)
  {
      SlotData slotData;
      slotData.slotNo = i;

      int offset = current->offset;
      int length = current->length;
      slotData.length = length;
      slotData.offset = offset;
      if(key_type == attrString)
      {
        if(recLen > MAX_KEY_SIZE1)
        {
            char *rec = new char[MAX_KEY_SIZE1];
            for(int j=0;j<MAX_KEY_SIZE1;j++)
                rec[j] = data[offset+j];
            slotData.stringData = rec;
        }
        else
        {
            char *rec = new char[recLen];
            for(int j=0;j<recLen;j++)
                rec[j] = data[offset+j];
            slotData.stringData = rec;
        }
      }
      else
      {
          slotData.intData = data[offset];
      }
      slotsInfo.push_back(slotData);
      current = ((slot_t *)data+i*sizeof(slot_t));
      i++;
  }
  std::sort(slotsInfo.begin(),slotsInfo.end());
  current =this->slot;

  int j=0;
  for(int i=0;i<slotsInfo.size();i++)
  {
    current->offset = slotsInfo[i].offset;
    current->length = slotsInfo[i].length;
    j++;
    current = ((slot_t *)data+j*sizeof(slot_t));
  }
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
    Status status =HFPage::deleteRecord(rid);
    if(status!=OK)
        return DONE;


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
      current = (slot_t*)data + i*sizeof(slot_t);
      i++;
  }
  return numRecords;
}
