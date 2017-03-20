#include <iostream>
#include <stdlib.h>
#include <vector>
#include <memory.h>
#include <heapfile.h>

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
    this->slot[0].offset = 0;
    this->freeSpace = MAX_SPACE - DPFIXED + sizeof(slot_t);
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
    bool negativeOffsetFound=false;
    if(recLen > available_space())
    {
        return DONE;
    }
    if(this->slotCnt==0 && recLen>this->freeSpace){
        return DONE;
    }
    rid.pageNo = this->curPage;
    if(this->usedPtr == MAX_SPACE - DPFIXED) //insert first record
    {
        int offset = MAX_SPACE - DPFIXED - recLen;
        this->slot->offset = offset;
        this->slot->length = recLen;
        rid.slotNo = 0;

        this->usedPtr = offset;
        this->freeSpace = this->freeSpace - recLen;
        memcpy(data+offset,recPtr,recLen);
        return OK;
    }
    else
    {
        slot_t *current = this->slot;
        int j=0;
        int minOffset=current->offset;
        while(j <= this->slotCnt)
        {

            if(current->offset<minOffset && current->offset!=-1 )
                minOffset = current->offset;

            current = (slot_t *)(data+j*sizeof(slot_t));
            j++;
        }

        current = this->slot;
        j=0;
        while(j<=this->slotCnt)
        {
            if(current->offset==-1)
            {
                rid.slotNo=j;
                //   negativeOffsetFound=true;
                break;
            }
            current = (slot_t*)(data+j*sizeof(slot_t));
            j++;
        }

        current->offset = minOffset - recLen;
        int offset = current->offset;

        if(j>this->slotCnt) {
            this->slotCnt += 1;
            // current = this->slot + (j-1)*sizeof(slot_t);
            this->freeSpace = this->freeSpace - recLen - sizeof(slot_t);
            rid.slotNo = j;
        }
        else{
            this->freeSpace = this->freeSpace - recLen;
        }
        current->length = recLen;
        //  cout<<"Offset 1:"<<offset1<<endl;

        memmove(data+offset,recPtr,recLen);


    }
    // cout<<available_space()<<endl;
    return OK;
}

// **********************************************************
// Delete a record from a page. Returns OK if everything went okay.
// Compacts remaining records but leaves a hole in the slot array.
// Use memmove() rather than memcpy() as space may overlap.
Status HFPage::deleteRecord(const RID& rid)
{
    // fill in the body
    int slot = rid.slotNo;
    if(rid.pageNo!=this->curPage)
        return FAIL;

    if(this->empty())
        return FAIL;
    RID nextRecord,currRecord = rid;
    Status status = this->nextRecord(rid,nextRecord);

    if(slot > this->slotCnt || this->curPage != rid.pageNo)
    {
        return  FAIL;
    }
    slot_t *current = this->slot;

    int j=0;
    while(j < this->slotCnt)

    {
        if(j==slot)
            break;

        current = (slot_t *)(data+j*sizeof(slot_t));
        j++;
    }


    int deletedOffset = current->offset;
    int deletedLength = current->length;
    current->offset=-1;
    this->freeSpace = this->freeSpace + deletedLength;


    while(status==OK){
        int nextSlot = nextRecord.slotNo;
        current = (slot_t *)(data +(nextSlot-1)*sizeof(slot_t));
        int offset = current->offset;

        memmove(data+offset+deletedLength,data+offset,current->length);
        currRecord = nextRecord;
        status = this->nextRecord(currRecord,nextRecord);
        current->offset = offset+deletedLength;

        //   deletedLength = current->length;
        //  deletedOffset = offset;

    }

    j=0;
    current = this->slot;
    int maxOffset = -1;
    while(j < this->slotCnt)
    {

        if(current->offset > maxOffset)
            maxOffset = current->offset;
        current = (slot_t *)(data+j*sizeof(slot_t));
        j++;
    }
    if(maxOffset!=-1)
        this->usedPtr = maxOffset;
    else
        this->usedPtr = MAX_SPACE - DPFIXED;
    if(maxOffset==-1) {
        this->slot->length = EMPTY_SLOT;
    }
    else{
        this->slot->length = MAX_SPACE - DPFIXED - maxOffset;
    }

    if(slot==this->slotCnt && this->slotCnt!=0)
    {
        //slot compaction
        this->slotCnt--;
        this->freeSpace = this->freeSpace + sizeof(slot_t);

    }
    return OK;
}

// **********************************************************
// returns RID of first record on page
Status HFPage::firstRecord(RID& firstRid)
{
    if(this->usedPtr == MAX_SPACE - DPFIXED || this->empty())
    {
        return DONE;
    }
    firstRid.pageNo = this->curPage;
    slot_t *currSlot = this->slot;
    int j=0;
    while(j<=this->slotCnt)
    {
        if(currSlot->offset == this->usedPtr)
        {
            firstRid.slotNo = j;
            return  OK;
        }
        currSlot = (slot_t *)(data+j*sizeof(slot_t));
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
    if(this->empty())
        return FAIL;
    if(curRid.pageNo!=this->curPage)
    {
        return FAIL;
    }
//    if(curRid.slotNo > this->slotCnt)
//    {
//        return DONE;
//    }
    int curSlot = curRid.slotNo;
    int currOffset;
    slot_t *temp = this->slot;
    int j=0;

    //move to current slot
    while(j<=this->slotCnt)
    {
        if(j==curSlot && temp->offset!=-1)
        {
            currOffset = temp->offset;
            break;
        }
        temp = (slot_t *)(data+j*sizeof(slot_t));
        j++;
    }

    j=0;
    temp = this->slot;
    bool recordFound = false;
    //find next record by summing offsets and lengths and equating to current record's offset
    while(j<=this->slotCnt)
    {
        int sum = temp->offset + temp->length;
        if(sum==currOffset)
        {
            nextRid.pageNo = this->curPage;
            nextRid.slotNo = j;
            recordFound = true;
            break;
        }
        temp = (slot_t *)(data+j*sizeof(slot_t));
        j++;
    }
    if(!recordFound)
        return DONE;
    return OK;
}

// **********************************************************
// returns length and copies out record with RID rid
Status HFPage::getRecord(RID rid, char* recPtr, int& recLen)
{
    if(recPtr==NULL || rid.pageNo!=this->curPage)
    {
        return FAIL;
    }
    if(this->empty())
        return FAIL;
    int slotNo = rid.slotNo;
    int j=0;
    slot_t *currSlot = this->slot;
    int offset=-1;
    while(j<=this->slotCnt)
    {
        if(j==slotNo)
        {
            offset = currSlot->offset;
            recLen = currSlot->length;
            break;
        }
        currSlot = (slot_t *)(data+j*sizeof(slot_t));
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
    if(this->empty())
    {
        return MINIBASE_FIRST_ERROR(HEAPFILE,NOMORERECS) ;
    }
    if(rid.pageNo!=this->curPage)
    {
        return FAIL;
    }


    int slotNo = rid.slotNo;
    if(slotNo==0)
    {
        int offset = this->slot->offset;
        int length = this->slot->length;
        recPtr = data + offset;
        recLen = length;
    }
    else{
        slot_t *slot = (slot_t *)(data+(slotNo-1)*sizeof(slot_t));
        int offset = slot->offset;
        int length = slot->length;
        recPtr = data + offset;
        recLen = length;
    }


    return OK;
}

// **********************************************************
// Returns the amount of available space on the heap file page
int HFPage::available_space(void)
{
    if(this->slotCnt==0)
    {
        if(this->slot->offset==0)
        {
            return MAX_SPACE - DPFIXED;
        }
        else
        {
            return freeSpace - sizeof(slot_t);
        }
    }
    else
    {
        int emptySlots = 0;
        int j=0;
        slot_t *curr = this->slot;
        while(j<this->slotCnt-1)
        {
            if(curr->offset==-1)
                emptySlots++;
            curr = (slot_t *)(data + j*sizeof(slot_t));
            j++;
        }
        if(emptySlots>0)
            return this->freeSpace + emptySlots*sizeof(slot_t);
        else
            return this->freeSpace - sizeof(slot_t);
    }
}

// **********************************************************
// Returns 1 if the HFPage is empty, and 0 otherwise.
// It scans the slot directory looking for a non-empty slot.
bool HFPage::empty(void)
{
    if(freeSpace == MAX_SPACE - DPFIXED + sizeof(slot_t))
        return true;
    return false;
}





