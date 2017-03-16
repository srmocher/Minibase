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
    this->replacer = new Replacer();
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
    PageId victim = replacer->getVictim();
    int frameIndex = hash(victim);
    HashEntry *entry = hashTable + frameIndex;
    if(entry->pageNumber == victim)
    {
        HashEntry *next = entry->next;
        delete(entry);
        entry = next;
    }

    //no frame available choose a replacement
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

    for(int i=0;i<numBuffers;i++)
    {
        if(descriptors[i].pageNumber == -1)
        {
            MINIBASE_DB->allocate_page(firstPageId,howmany);
            MINIBASE_DB->read_page(firstPageId,firstpage);
            descriptors[i].pageNumber = firstPageId;
            descriptors[i].pin_count = 0;
            descriptors[i].dirty = false;
            int frameIndex = hash(firstPageId);
            HashEntry *entry = new HashEntry();
            entry->pageNumber = firstPageId;
            entry->frameNumber = i;
            entry->next = NULL;
            HashEntry *current = hashTable+frameIndex;
            if(current!=NULL)
            {
                while(current->next!=NULL)
                    current=current->next;
                current->next = entry;
            }
            else
            {
                current = entry;
            }
            memcpy(bufPool+i,firstpage,sizeof(Page));
            return OK;
        }
    }
  return MINIBASE_FIRST_ERROR(BUFMGR,BUFFERFULL);
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

Replacer::Replacer() {}

void Replacer::addtoLRUQueue(PageId pageNumber) {
         LRUQueue.push(pageNumber);
}

PageId Replacer::getVictim() {
    if(MRUStack.empty() && LRUQueue.empty()){
        return -1;
    }
    if(!MRUStack.empty()){
        PageId pageNumber = MRUStack.top();
        if(LRUQueue.front()==pageNumber){
            MRUStack.pop();
            pageNumber = MRUStack.top();
        }
        MRUStack.pop();
        return pageNumber;
    }
    if(!LRUQueue.empty()){
        PageId  pageNumber = LRUQueue.front();
        LRUQueue.pop();
        return pageNumber;
    }
}

void Replacer::addtoMRUStack(PageId pageNumber) {
    MRUStack.push(pageNumber);
}