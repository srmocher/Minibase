/* 
 * bt.h - global declarations for the B+ Tree.
 *
 * Gideon Glass & Johannes Gehrke  951012  CS564  UW-Madison
 * Edited by Young-K. Suh (yksuh@cs.arizona.edu) 03/27/14 CS560 Database Systems Implementation 
 */ 

#ifndef BT_H
#define BT_H

/*
 * This file contains, among other things, the interface to our
 * key (void*) abstraction.  The BTLeafPage and BTIndexPage code
 * know very little about the various key types.  In fact, only 
 * a very small amount of work would be required to make those 
 * classes share almost all of their code (e.g., by creating a class
 * `BTPage' derived from SortedPage and from which BTLeafPage and 
 * BTIndexPage themselves drive).  
 *
 * Essentially we provide a way for those classes to package and 
 * unpackage <key,data> pairs in a space-efficient manner.  That is, the 
 * packaged result contains the key followed immediately by the data value; no 
 * padding bytes are used (not even for alignment). 
 *
 * Furthermore, the BT<*>Page classes need
 * not know anything about the possible AttrType values, since all 
 * their processing of <key,data> pairs is done by the functions 
 * declared here.
 *
 * In addition to packaging/unpacking of <key,value> pairs, we 
 * provide a keyCompare function for the (obvious) purpose of
 * comparing keys of arbitrary type (as long as they are defined 
 * here).
 */
 
#include "minirel.h"


typedef enum {
    INDEX,
    LEAF
} nodetype;


/*
 * A bunch of macros for handling and returning errors.
 *
 * Note that some of these macros effect immediate returns from the current
 * function; some return the specified error; the *_CTOR ones assign
 * the error to an argument (these are for use in returnvalue-less 
 * constructors).
 */

/*
 * MAX_KEY_SIZE1: Maximum key size in bytes
 */

#define MAX_KEY_SIZE1        220


/*
 * Keytype union: used to discover the max keysize, and for minimal 
 * static type checking in key package/unpackage functions (see below).
 */

union Keytype
{
  int intkey;
  char charkey[MAX_KEY_SIZE1];
};


/*
 * Datatype union: used to discover the max data size, and for minimal 
 * static type checking in key package/unpackage functions (see below).
 */

union Datatype
{
  PageId      pageNo;  // for index page entries
  RID         rid;     // for leaf page entries    
};


/*
 * struct KeyDataEntry:
 *
 * The BT*Page code only knows about the following type (by name).
 * It never refers to the listed members; rather, it simply instantiates
 * an object so that it has enough storage to copy into and out of a record
 * the necessary number of <key, data> bytes (for whatever key and data
 * types it is interested in -- for key types, it doesn't care).  
 */
 
struct KeyDataEntry
{
  Keytype    key;
  Datatype   data;
};

/*
 * Finally, here is the interface to our <key,data> abstraction.
 * 
 * keyCompare simply compares keys (types must be the same); return 
 * value is < 0, 0, or > 0.
 *
 * make_entry packages a key and a data value into a chunk of memory 
 * large enough to hold it (the first parameter).  Note that the 
 * resultant KeyDataEntry cannot be accessed by its members because
 * the Datatype member may start after the actual beginning of the
 * data value stored here.  The real length of the resulting <key,data>
 * pair is returned in *pentry_len.
 *
 * get_key_data takes a KeyDataEntry chunk and its real length and 
 * unpacks the <key,data> values from it; those are written to *targetkey
 * and *targetdata, respectively.
 *   - key1  < key2 : negative
 *   - key1 == key2 : 0
 *   - key1  > key2 : positive
 *
 * Finally, get_key_length and get_key_data_length determine the 
 * storage required for given key and key+data. 
 */

int keyCompare(const void* key1, const void* key2, AttrType t);
                   
/*
 * make_entry: write a <key,data> pair to a blob of memory (*target) big
 * enough to hold it.  Return length of data in the blob via *pentry_len.
 *
 * Ensures that <data> part begins at an offset which is an even 
 * multiple of sizeof(PageNo) for alignment purposes.
 */

void make_entry(KeyDataEntry *target,
                AttrType key_type, const void *key,
                nodetype ndtype, Datatype data, 
                int *pentry_len);

/*
 * get_key_data: unpack a <key,data> pair into pointers to respective parts.
 * Needs a) memory chunk holding the pair (*psource) and, b) the length
 * of the data chunk (to calculate data start of the <data> part).
 */

void get_key_data(void *targetkey, Datatype *targetdata,
                  KeyDataEntry *psource, int entry_len, 
                  nodetype ndtype);

/*
 * get_key_length: return key length in given key_type
 */
int get_key_length(const void *key, const AttrType key_type);

/*
 * get_key_data_length: return (key+data) length in given key_type
 */   
int get_key_data_length(const void *key, const AttrType key_type, 
                        const nodetype ndtype);
                   
#endif
