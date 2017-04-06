#ifndef _INDEX_H
#define _INDEX_H

#include "minirel.h"

typedef enum {
  NewScan,
  ScanRunning,
  ScanComplete
} ScanState;

class IndexFileScan;

class IndexFile {
  friend class IndexFileScan;

  public:
    virtual ~IndexFile() {};      // will close file, not destroy it

    virtual Status insert(const void* data, const RID rid) = 0;
    virtual Status Delete(const void* data, const RID rid) = 0;

    // virtual IndexFileScan *new_scan() = 0;  // 

};


class IndexFileScan {
   public:
     virtual ~IndexFileScan() {}  // will close scan

     virtual Status get_next(RID &rid, void* keyptr) = 0;
     virtual Status delete_current() = 0;

     virtual int keysize() = 0;

   private:
     // IndexFile* fileptr;

};

#endif

