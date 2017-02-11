
#ifndef _HFPAGE_H
#define _HFPAGE_H

#include "minirel.h"
#include "page.h"


const int INVALID_SLOT =  -1;
const int EMPTY_SLOT   =  -1;

// Class definition for a minibase data page.   
// The design assumes that records are kept compacted when
// deletions are performed. Notice, however, that the slot
// array cannot be compacted.  Notice, this class does not keep
// the records aligned, relying instead on upper levels to take
// care of non-aligned attributes.

class HFPage {

  protected:
    struct slot_t {
        short   offset;  
        short   length;    // equals EMPTY_SLOT if slot is not in use
    };

    static const int DPFIXED =       sizeof(slot_t)
                           + 4 * sizeof(short)
                           + 3 * sizeof(PageId);

      // Warning:
      // These items must all pack tight, (no padding) for
      // the current implementation to work properly.
      // Be careful when modifying this class.

    short     slotCnt;     // number of slots in use
    short     usedPtr;     // offset of first used byte in data[]
    short     freeSpace;   // number of bytes free in data[]

    short     type;        // an arbitrary value used by subclasses as needed

    PageId    prevPage;    // backward pointer to data page
    PageId    nextPage;    // forward pointer to data page
    PageId    curPage;     // page number of this page

    slot_t    slot[1];     // first element of slot array.

    char      data[MAX_SPACE - DPFIXED]; 

  public:
    void init(PageId pageNo);   // initialize a new page
    void dumpPage();            // dump contents of a page

    PageId getNextPage();       // returns value of nextPage
    PageId getPrevPage();       // returns value of prevPage

    void setNextPage(PageId pageNo);    // sets value of nextPage to pageNo
    void setPrevPage(PageId pageNo);    // sets value of prevPage to pageNo

    PageId page_no() { return curPage;} // returns the page number

    // inserts a new record pointed to by recPtr with length recLen onto
    // the page, returns RID of record 
    Status insertRecord(char *recPtr, int recLen, RID& rid);

    // delete the record with the specified rid
    Status deleteRecord(const RID& rid);

      // returns RID of first record on page
      // returns DONE if page contains no records.  Otherwise, returns OK
    Status firstRecord(RID& firstRid);

      // returns RID of next record on the page 
      // returns DONE if no more records exist on the page
    Status nextRecord (RID curRid, RID& nextRid);

      // copies out record with RID rid into recPtr
    Status getRecord(RID rid, char *recPtr, int& recLen);

      // returns a pointer to the record with RID rid
    Status returnRecord(RID rid, char*& recPtr, int& recLen);

      // returns the amount of available space on the page
    int    available_space(void);

      // Returns true if the HFPage is has no records in it, false otherwise.
    bool empty(void);

};

#endif // _HFPAGE_H
