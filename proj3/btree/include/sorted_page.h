/* 
 * sorted_page.h - definition of class SortedPage for Minibase project.
 *
 * Johannes Gehrke & Gideon Glass  951016  CS564  UW-Madison
 * Edited by Young-K. Suh (yksuh@cs.arizona.edu) 03/27/14 CS560 Database Systems Implementation 
 */

#ifndef SORTED_PAGE_H
#define SORTED_PAGE_H

/*
 * SortedPage just holds abstract records in sorted order, based 
 * on how they compare using the key interface from bt.h+key.C.
 */
 

#include "minirel.h"
#include "page.h"
#include "hfpage.h"
#include "bt.h"


class SortedPage : public HFPage {
 private:
   // No private variables should be declared.

 public:

/*
 * Error handling infrastructure, added by us:
 */

   enum ErrorTypes { 
    _OK = 0,   /* these are indices */
    INSERT_REC_FAILED,
    DELETE_REC_FAILED,
    NR_ERRORS              /* and this is the number of them */
  }; 

  static const char* Errors[NR_ERRORS];

// Performs a sorted insertion of a record on an record page. The records are
// sorted in increasing key order.
// Only the  slot  directory is  rearranged.  The  data records remain in
// the same positions on the  page.
//
// Parameters:
//
//   o key_type - the type of the key.
//
//   o recPtr points to the actual record that will be placed on the page
//            (So, recPtr is the combination of the key and the other data
//              value(s)).
//
//   o recLen is the length of the record to be inserted.
//
//   o rid is the record id of the record inserted.
//
   Status insertRecord(AttrType key_type,
                       char * recPtr, int recLen,
                       RID& rid);


// Deletes a record from a sorted record page. It just calls
// HFPage::deleteRecord()
   Status deleteRecord(const RID& rid);

  // The remaining functions of HFPage are still visible.
  // return number of records
  int     numberOfRecords();
  // return free spacce
  int     free_space() { return freeSpace;}
  // set node type
  void  set_type(short t) { type = t; }
  // get node type
  short get_type()         { return type; }
};

#endif
