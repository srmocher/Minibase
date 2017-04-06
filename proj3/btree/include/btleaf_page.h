/* 
 * btleaf_page.h - definition of class BTLeafPage for Minibase project.
 * Edited by Young-K. Suh (yksuh@cs.arizona.edu) 03/27/14 CS560 Database Systems Implementation 
 *
 */

#ifndef BTLEAF_PAGE_H
#define BTLEAF_PAGE_H

#include "minirel.h"
#include "page.h"
#include "sorted_page.h"
#include "bt.h"
#include "btindex_page.h"



class BTLeafPage : public SortedPage {
  
 private:
   // No private variables should be declared.

 public:


// In addition to initializing the  slot directory and internal structure
// of the HFPage, this function sets up the type of the record page.

   void init(PageId pageNo)
   { HFPage::init(pageNo); set_type(LEAF); }

// ------------------- insertRec ------------------------
   // READ THIS DESCRIPTION CAREFULLY. THERE ARE TWO RIDs
   // WHICH MEAN TWO DIFFERENT THINGS.
// Inserts a <key, dataRid> value into the leaf node. This is
// accomplished by a call to SortedPage::insertRecord()
// This function sets up the recPtr field for the call
// to SortedPage::insertRecord() 
//
// Parameters:
//   o key - the key value of the data record.
//
//   o key_type - the type of the key.
//
//   o dataRid - the rid of the data record. This is
//               stored on the leaf page along with the
//               corresponding key value.
//
//   o rid - the rid of the inserted leaf record data entry, i.e., the
//     <key, dataRid> pair
   
   Status insertRec(const void *key, AttrType key_type, RID dataRid, RID& rid);

// ------------------- Iterators ------------------------
// The two functions get_first and get_next provide an
// iterator interface to the records on a BTLeafPage.
// get_first returns the first <key, dataRid> pair from the page,
// while get_next returns the next pair on the page.
// These functions make calls to HFPage::firstRecord() and
// HFPage::nextRecord(), and then split the flat record into its
// two components: namely, the key and dataRid.
// Should return NOMORERECS when there are no more pairs

   Status get_first(RID& rid, void *key, RID & dataRid);
   Status get_next (RID& rid, void *key, RID & dataRid);


// ------------------ get_data_rid -----------------------
// This function performs a sequential search (or a
// binary search if you are ambitious) to find a data entry
// of the form <key, dataRid>, where key is given in the call.
// It returns the dataRid component of the pair; note that this
// is the rid of the DATA record, and NOT the rid of the data entry!

   Status get_data_rid(void *key, AttrType attrtype, RID & dataRid);

};

#endif
