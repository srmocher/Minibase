#include <iostream>
#include <stdlib.h>
#include <memory.h>

#include "hfpage.h"
#include "buf.h"
#include "db.h"


// **********************************************************
// page class constructor

void HFPage::init(PageId pageNo)
{
    this->curPage = pageNo;
    this->nextPage = INVALID_PAGE;
    this->prevPage = INVALID_PAGE;
    this->slotCnt = 0;
    this->slot[0].length = EMPTY_SLOT;
    this->slot[1].offset = -1;
    this->freeSpace = MAX_SPACE - DPFIXED;
    this->usedPtr = MAX_SPACE - DPFIXED;
  // fill in the body
}

// **********************************************************
// dump page utlity
void HFPage::dumpPage()
{
    int i;

    cout << "dumpPage, this: " << this << endl;
    cout << "curPage= " << curPage << ", nextPage=" << nextPage << endl;
    cout << "usedPtr=" << usedPtr << ",  freeSpace=" << freeSpace
         << ", slotCnt=" << slotCnt << endl;
   
    for (i=0; i < slotCnt; i++) {
        cout << "slot["<< i <<"].offset=" << slot[i].offset
             << ", slot["<< i << "].length=" << slot[i].length << endl;
    }
}

// **********************************************************
PageId HFPage::getPrevPage()
{
    return this->prevPage;
}

// **********************************************************
void HFPage::setPrevPage(PageId pageNo)
{
    this->prevPage = pageNo;
}

// **********************************************************
PageId HFPage::getNextPage()
{
   return this->nextPage;
}

// **********************************************************
void HFPage::setNextPage(PageId pageNo)
{
  this->nextPage = pageNo;
}

// **********************************************************
// Add a new record to the page. Returns OK if everything went OK
// otherwise, returns DONE if sufficient space does not exist
// RID of the new record is returned via rid parameter.
Status HFPage::insertRecord(char* recPtr, int recLen, RID& rid)
{
    // fill in the body
    return OK;
}

// **********************************************************
// Delete a record from a page. Returns OK if everything went okay.
// Compacts remaining records but leaves a hole in the slot array.
// Use memmove() rather than memcpy() as space may overlap.
Status HFPage::deleteRecord(const RID& rid)
{
    // fill in the body
    return OK;
}

// **********************************************************
// returns RID of first record on page
Status HFPage::firstRecord(RID& firstRid)
{
    if(this->usedPtr == MAX_SPACE - DPFIXED)
    {
        return DONE;
    }
    firstRid.pageNo = this->curPage;
    slot_t currSlot = this->slot[0];
    int j=1;
    while(j<=this->slotCnt)
    {
        if(currSlot.offset == this->usedPtr)
        {
            firstRid.slotNo = j;
            return  OK;
        }
        short *temp = static_cast<short *>(&currSlot);
        temp = temp + sizeof(slot_t);
        currSlot.offset = *temp;
        currSlot.length = *(temp+sizeof(short));
        j++;
    }
    return DONE;
}

// **********************************************************
// returns RID of next record on the page
// returns DONE if no more records exist on the page; otherwise OK
Status HFPage::nextRecord (RID curRid, RID& nextRid)
{
    // fill in the body
    if(usedPtr == MAX_SPACE - DPFIXED)
    {
        return  FAIL;
    }
    int curSlot = curRid.slotNo;
    int currOffset;
    slot_t temp = this->slot[0];
    int j=1;

    while(j<=this->slotCnt)
    {
        if(j==curSlot)
        {
            currOffset = temp.offset;
            break;
        }
        short *currSlotPtr= static_cast<short *>(&temp);
        currSlotPtr = currSlotPtr + sizeof(slot_t);
        temp.offset = *currSlotPtr;
        temp.length = *(currSlotPtr+sizeof(short));
        j++;
    }

    j=1;
    temp = this->slot[0];
    while(j<=this->slotCnt)
    {
        int sum = temp.offset + temp.length;
        if(sum==currOffset)
        {
            nextRid.pageNo = this->curPage;
            nextRid.slotNo = j;
        }
        short *currSlotPtr= static_cast<short *>(&temp);
        currSlotPtr = currSlotPtr + sizeof(slot_t);
        temp.offset = *currSlotPtr;
        temp.length = *(currSlotPtr+sizeof(short));
        j++;
    }
    return OK;
}

// **********************************************************
// returns length and copies out record with RID rid
Status HFPage::getRecord(RID rid, char* recPtr, int& recLen)
{
    if(this->slotCnt)
        return FAIL;
    int slotNo = rid.slotNo;
    int j=1;
    slot_t currSlot = this->slot[0];
    int offset=-1;
    while(j<=this->slotCnt)
    {
        if(j==slotNo)
        {
            offset = currSlot.offset;
            recLen = currSlot.length;
            break;
        }
        short *currSlotPtr= static_cast<short *>(&currSlot);
        currSlotPtr = currSlotPtr + sizeof(slot_t);
        currSlot.offset = *currSlotPtr;
        currSlot.length = *(currSlotPtr+sizeof(short));
        j++;
    }
    if(offset == -1)
    {
        return DONE;
    }
    for(int i=0;i<recLen;i++){
        recPtr[i]=data[offset+i];
    }
    // fill in the body
    return OK;
}

// **********************************************************
// returns length and pointer to record with RID rid.  The difference
// between this and getRecord is that getRecord copies out the record
// into recPtr, while this function returns a pointer to the record
// in recPtr.
Status HFPage::returnRecord(RID rid, char*& recPtr, int& recLen)
{
    // fill in the body
    if(this->slotCnt==0)
        return FAIL;
    int slotNo = rid.slotNo;
    int j=1;
    slot_t currSlot = this->slot[0];
    int offset=-1;
    while(j<=this->slotCnt)
    {
        if(j==slotNo)
        {
            offset = currSlot.offset;
            recLen = currSlot.length;
            break;
        }
        short *currSlotPtr= static_cast<short *>(&currSlot);
        currSlotPtr = currSlotPtr + sizeof(slot_t);
        currSlot.offset = *currSlotPtr;
        currSlot.length = *(currSlotPtr+sizeof(short));
        j++;
    }
    if(offset == -1)
    {
        return DONE;
    }
    recPtr = (char *)data[offset];
    return OK;
}

// **********************************************************
// Returns the amount of available space on the heap file page
int HFPage::available_space(void)
{
    // fill in the body
    slot_t currSlot = this->slot[0];
    return 0;
}

// **********************************************************
// Returns 1 if the HFPage is empty, and 0 otherwise.
// It scans the slot directory looking for a non-empty slot.
bool HFPage::empty(void)
{
    // fill in the body
    return true;
}



