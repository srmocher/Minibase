
#ifndef _BUF_H
#define _BUF_H

#include "db.h"
#include "page.h"

#define NUMBUF 50   // Default number of frames, small number for debugging.

// **************** ALL BELOW are purely local to buffer Manager ********
// class for maintaining information about buffer pool frame
class   BufMgr;

#define REPLACER Clock  // This is the default replacement policy.  You
                        // may specify a different policy when you create
                        // the buffer manager.

// *****************************************************

// Create enums for internal errors.
enum bufErrCodes  {
    HASH_TBL_ERROR,
    HASH_NOT_FOUND,
    BUFFER_EXCEEDED,
    PAGE_NOT_PINNED,
    BAD_BUFFER,
    PAGE_PINNED,
    REPLACER_ERROR,
    BAD_BUF_FRAMENO,
    PAGE_NOT_FOUND,
    FRAME_EMPTY,
};


// *****************************************************
class FrameDesc {

  friend class BufMgr;

  private:
    int    pageNo;     // the page within file, or INVALID_PAGE if
                       // the frame is empty.

    unsigned int pin_cnt;  // The pin count for the page in this frame



    FrameDesc() {
        pageNo  = INVALID_PAGE;
        pin_cnt = 0;
    }

   ~FrameDesc() {}

  public:
    int pin_count() { return(pin_cnt); }
    int pin() { return(++pin_cnt); }
    int unpin() {
        pin_cnt = (pin_cnt <= 0) ? 0 : pin_cnt - 1;
        return(pin_cnt);
    }
};

// *****************************************************
class Replacer {

  public:
    virtual int pin( int frameNo );
    virtual int unpin( int frameNo );
    virtual int free( int frameNo );
    virtual int pick_victim() = 0;     // Must pin the returned frame.
    virtual const char *name() = 0;
    virtual void info();

    unsigned getNumUnpinnedBuffers();

  protected:
    Replacer();
    virtual ~Replacer();

    enum STATE {Available, Referenced, Pinned};

    BufMgr *mgr;
    friend class BufMgr;
    virtual void setBufferManager( BufMgr *mgr );

    // These variables are required for the clock algorithm.

    int         head;           // Clock hand.
    STATE      *state_bit;      // [numBuffers]
};

// *****************************************************
class Clock : public Replacer {

  public:

    Clock();
   ~Clock();

    int   pick_victim();
    const char *name() { return "Clock"; }
    void  info();
};

// *****************************************************
class BufMgr {
  friend class HPTester;

  private:
    unsigned int    numBuffers;
    Page           *bufPool;    // [numBuffers]; physical buffer pool

                                // An array of Descriptors one per frame.
    FrameDesc      *frmeTable;  // [numBuffers]

    Replacer       *replacer;

    // Factor out the common code for the two versions of Flush
    Status privFlushPages(int pageid, int all_pages=0);

  public:

                // If you provide a replacer, the BufMgr will free it.
    BufMgr( int bufsize, Replacer *replacer=0 );

                // flushs all valid dirty pages to disk.
   ~BufMgr();      

        // Check if this page is in buffer pool, otherwise
        // find a frame for this page, read in and pin it.
        // Also write out the old page if it's dirty before reading
        // if emptyPage==TRUE, then actually no read is done to bring
        // the page in.
    Status pinPage(int PageId_in_a_DB, Page*& page,
                   int emptyPage=0, const char *filename=NULL);

        // if pincount > 0, decrement it and if it becomes zero,
        // put it in a group of replacement candidates.
        // if pincount=0 before this call, return error.
    Status unpinPage(int globalPageId_in_a_DB,
                     int dirty=FALSE, const char *filename=NULL);

        // Call DB object to allocate a run of new pages and 
        // find a frame in the buffer pool for the first page
        // and pin it. If buffer is full, ask DB to deallocate 
        // all these pages and return error
    Status newPage(int& firstPageId, Page*& firstpage,int howmany=1); 

        // User should call this method if she needs to delete a page
        // this routine will call DB to deallocate the page .
    Status freePage(int globalPageId); 


        // Added to flush a particular page of the buffer pool to disk
    Status flushPage(int pageid);

        // Flushes all pages of the buffer pool to disk
    Status flushAllPages();


    unsigned int getNumBuffers() const { return numBuffers; }
    unsigned int getNumUnpinnedBuffers();

      // A few routines currently need direct access to the FrameTable.
    FrameDesc *frameTable() { return frmeTable; }
};

// *****************************************************

#endif // _BUF_H
