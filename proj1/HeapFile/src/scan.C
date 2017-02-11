/*
 * implementation of class Scan for HeapFile project.
 * $Id: scan.C,v 1.1 1997/01/02 12:46:42 flisakow Exp $
 */

#include <stdio.h>
#include <stdlib.h>

#include "heapfile.h"
#include "scan.h"
#include "hfpage.h"
#include "buf.h"
#include "db.h"

// *******************************************
// The constructor pins the first page in the file
// and initializes its private data members from the private data members from hf
Scan::Scan (HeapFile *hf, Status& status)
{
  // put your code here
  status = OK;
}

// *******************************************
// The deconstructor unpin all pages.
Scan::~Scan()
{
  // put your code here
}

// *******************************************
// Retrieve the next record in a sequential scan.
// Also returns the RID of the retrieved record.
Status Scan::getNext(RID& rid, char *recPtr, int& recLen)
{
  // put your code here
  return OK;
}

// *******************************************
// Do all the constructor work.
Status Scan::init(HeapFile *hf)
{
  // put your code here
  return OK;
}

// *******************************************
// Reset everything and unpin all pages.
Status Scan::reset()
{
  // put your code here
  return OK;
}

// *******************************************
// Copy data about first page in the file.
Status Scan::firstDataPage()
{
  // put your code here
  return OK;
}

// *******************************************
// Retrieve the next data page.
Status Scan::nextDataPage(){
  // put your code here
  return OK;
}

// *******************************************
// Retrieve the next directory page.
Status Scan::nextDirPage() {
  // put your code here
  return OK;
}
