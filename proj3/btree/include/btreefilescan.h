/*
 * btreefilescan.h
 *
 * sample header file
 * Edited by Young-K. Suh (yksuh@cs.arizona.edu) 03/27/14 CS560 Database Systems Implementation 
 *
 */
 
#ifndef _BTREEFILESCAN_H
#define _BTREEFILESCAN_H

#include "btfile.h"

// errors from this class should be defined in btfile.h
class BTreeFile;

class BTreeFileScan : public IndexFileScan {
public:
    friend class BTreeFile;

    // get the next record
    Status get_next(RID & rid, void* keyptr);

    // delete the record currently scanned
    Status delete_current();

    int keysize(); // size of the key

    // destructor
    ~BTreeFileScan();
    BTreeFileScan(BTreeFile *f)
    {
        this->file=f;
        fullIndexScan =false;
        firstRecordScanned = false;
        exactMatchSearch = false;
        invalidRange = false;
    }
private:


    const void *lowVal;
    const void *highVal;
    bool firstRecordScanned;
    int keySize;
    AttrType type;
    PageId  rootId;
    BTreeFile *file;
    void traverseToLowValLeaf();
    bool fullIndexScan;
    bool exactMatchSearch;
    BTLeafPage *currentPage;
    RID currentRID,currentDataRID;
    void *currentKey;
    bool invalidRange;

};

#endif
