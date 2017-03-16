/*****************************************************************************/
/*************** Implementation of the Buffer Manager Layer ******************/
/*****************************************************************************/


#include <buf.h>
#include "buf.h"


// Define buffer manager error messages here
//enum bufErrCodes  {...};

// Define error message here
static const char* bufErrMsgs[] = { 
  // error message strings go here
  "Not enough memory to allocate hash entry",
  "Inserting a duplicate entry in the hash table",
  "Removing a non-existing entry from the hash table",
  "Page not in hash table",
  "Not enough memory to allocate queue node",
  "Poping an empty queue",
  "OOOOOOPS, something is wrong",
  "Buffer pool full",
  "Not enough memory in buffer manager",
  "Page not in buffer pool",
  "Unpinning an unpinned page",
  "Freeing a pinned page"
};

// Create a static "error_string_table" object and register the error messages
// with minibase system 
static error_string_table bufTable(BUFMGR,bufErrMsgs);


//*************************************************************
//** This is the implementation of BufMgr
//************************************************************

BufMgr::BufMgr (int numbuf, Replacer *replacer) {
  // put your code here
    this->numBuffers = numbuf;
    this->hashTable = new HashEntry[HTSIZE];
   this->bufPool = new Page[numbuf];
   this->descriptors = new Descriptor[numbuf];
    for(int i=0;i<numbuf;i++) {
        descriptors[i].pageNumber = -1;
        descriptors[i].pin_count = 0;
        descriptors[i].dirty = false;
    }

}

//*************************************************************
//** This is the implementation of ~BufMgr
//************************************************************
BufMgr::~BufMgr(){
  // put your code here
}

//*************************************************************
//** This is the implementation of pinPage
//************************************************************
Status BufMgr::pinPage(PageId PageId_in_a_DB, Page*& page, int emptyPage) {
  // put your code here


    for(int i=0;i<numBuffers;i++)
    {
        if(descriptors[i].pageNumber==-1)
        {
            int frameIndex = hash(PageId_in_a_DB);
            HashEntry *entry = new HashEntry();
            entry->pageNumber = PageId_in_a_DB;
            entry->frameNumber = i;
            HashEntry *current = hashTable+frameIndex;
            entry->next = NULL;
            if(current!=NULL)
            {
                         while (current->next!=NULL)
                             current=current->next;
                        current->next = entry;
            }
            else
            {
                current = entry;
            }
            if(emptyPage!=TRUE)
            {
                MINIBASE_DB->read_page(PageId_in_a_DB,page);
                memcpy(bufPool + i,page,sizeof(Page));
            }
            descriptors[i].pin_count += 1;
            descriptors[i].pageNumber = PageId_in_a_DB;
            return OK;
        }
    }
  return OK;
}//end pinPage

//*************************************************************
//** This is the implementation of unpinPage
//************************************************************
Status BufMgr::unpinPage(PageId page_num, int dirty=FALSE, int hate = FALSE){
  // put your code here
  return OK;
}

//*************************************************************
//** This is the implementation of newPage
//************************************************************
Status BufMgr::newPage(PageId& firstPageId, Page*& firstpage, int howmany) {
  // put your code here
  return OK;
}

//*************************************************************
//** This is the implementation of freePage
//************************************************************
Status BufMgr::freePage(PageId globalPageId){
  // put your code here
  return OK;
}

//*************************************************************
//** This is the implementation of flushPage
//************************************************************
Status BufMgr::flushPage(PageId pageid) {
  // put your code here
  return OK;
}
    
//*************************************************************
//** This is the implementation of flushAllPages
//************************************************************
Status BufMgr::flushAllPages(){
  //put your code here
  return OK;
}


/*** Methods for compatibility with project 1 ***/
//*************************************************************
//** This is the implementation of pinPage
//************************************************************
Status BufMgr::pinPage(PageId PageId_in_a_DB, Page*& page, int emptyPage, const char *filename){
  //put your code here
  return OK;
}

//*************************************************************
//** This is the implementation of unpinPage
//************************************************************
Status unpinPage(PageId globalPageId_in_a_DB, int dirty, const char *filename){
  //put your code here
  return OK;
}

//*************************************************************
//** This is the implementation of getNumUnpinnedBuffers
//************************************************************
unsigned int BufMgr::getNumUnpinnedBuffers(){
    int unpinned = 0;
    for(int i=0;i<numBuffers;i++)
    {
        if(descriptors[i].pageNumber!=-1 && descriptors[i].pin_count>0)
            unpinned++;
    }
    return unpinned;
}

int BufMgr::hash(int pageNumber)
{
    return (2*pageNumber + 1)%HTSIZE;
}