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
    numBuffers = numbuf;
    int id;
    Status status;

    hashTable = new HashEntry*[HTSIZE];
//    minibase_globals->malloc(sizeof(Page));

   bufPool = (Page *)malloc(numbuf* sizeof(Page));
    memset(bufPool, 0, numbuf * sizeof(Page));
   this->descriptors = new Descriptor[numbuf];
    for(int i=0;i<numbuf;i++) {
        descriptors[i].pageNumber = -1;
        descriptors[i].pin_count = 0;
        descriptors[i].dirty = false;
    }
    for(int i=0;i<HTSIZE;i++){
      hashTable[i]=NULL;
    }

   this->replacer = new PageReplacer();
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
       //  cout<<descriptors[i].pageNumber<<",";
          if(descriptors[i].pageNumber == PageId_in_a_DB)
          {
              descriptors[i].pin_count+=1;
              int frameIndex = hash(PageId_in_a_DB);
              HashEntry *entry = hashTable[frameIndex];
              while(entry!= NULL)
              {
                  if(entry->pageNumber == PageId_in_a_DB)
                      break;
                  entry = entry->next;
              }
              if(entry==NULL)
                  return MINIBASE_FIRST_ERROR(BUFMGR,BUFFERPAGENOTFOUND);
              page = bufPool + entry->frameNumber;
              int frameNumber = entry->frameNumber;
            //  memcpy(page,bufPool+frameNumber,sizeof(Page));
              return OK;
          }
      }
  //  cout<<endl;

    for(int i=0;i<numBuffers;i++)
    {
        if(descriptors[i].pageNumber==-1)
        {
            int frameIndex = hash(PageId_in_a_DB);
            HashEntry *entry = new HashEntry();
            entry->pageNumber = PageId_in_a_DB;
            entry->frameNumber = i;
            HashEntry *current = hashTable[frameIndex];
            entry->next = NULL;
            if(current!=NULL)
            {
                         while (current->next!=NULL)
                             current=current->next;
                        current->next = entry;
            }
            else
            {
                hashTable[frameIndex] = entry;
            }
            page = bufPool +i;
            if(emptyPage!=TRUE)
            {
                Status readStatus = MINIBASE_DB->read_page(PageId_in_a_DB,page);
                if(readStatus!=OK)
                    return MINIBASE_CHAIN_ERROR(BUFMGR,readStatus);

                  memcpy(bufPool + i,page,sizeof(Page));
            }
            descriptors[i].pin_count += 1;
            descriptors[i].pageNumber = PageId_in_a_DB;
            return OK;
        }
    }
    PageId victim = replacer->getVictim();
    int frameIndex = hash(victim);
    HashEntry *entry = hashTable[frameIndex];
    while(entry)
    {
        if(entry->pageNumber == victim)
            break;
        entry = entry->next;
    }

   Status flushStatus =  flushPage(victim);

    descriptors[entry->frameNumber].pageNumber= PageId_in_a_DB;
    page = bufPool + entry->frameNumber;
    Status readStatus = MINIBASE_DB->read_page(PageId_in_a_DB,page);
  //  page = pg;
  //  cout<<"Data read - "<<(char *)pg<<endl;
    descriptors[entry->frameNumber].pin_count+=1;
    int victimFrame = entry->frameNumber;
    if(readStatus==OK && emptyPage!=true)
       memcpy(bufPool+entry->frameNumber,page,sizeof(Page));
    if(flushStatus!=OK)
        return MINIBASE_FIRST_ERROR(BUFMGR,BUFMGRMEMORYERROR);
//    if(entry->next==NULL)
//    {
//           delete(entry);
//    } else
//    {
//        HashEntry *next = entry->next;
//        delete(entry);
//        entry = next;
//    }
    int newFrameIndex = hash(PageId_in_a_DB);
    HashEntry *newEntry = hashTable[newFrameIndex];
    if(newEntry==NULL)
    {
        newEntry = new HashEntry();
        newEntry->next = NULL;
        newEntry->frameNumber =  victimFrame;
        newEntry->pageNumber = PageId_in_a_DB;
    }
    else
    {
        while(newEntry->next)
            newEntry= newEntry->next;
        newEntry->next = new HashEntry();
        newEntry->next->next = NULL;
        newEntry->next->pageNumber = PageId_in_a_DB;
        newEntry->next->frameNumber = victimFrame;
    }


    //no frame available choose a replacement
  return OK;
}//end pinPage

//*************************************************************
//** This is the implementation of unpinPage
//************************************************************
Status BufMgr::unpinPage(PageId page_num, int dirty=FALSE, int hate = FALSE){
  // put your code here

  	for(int i=0;i<numBuffers;i++)
	{
        if(descriptors[i].pageNumber==page_num)
        {
            int frameIndex = hash(page_num);
            HashEntry *entry = hashTable[frameIndex];
            Page *page = bufPool+entry->frameNumber;

    		if(descriptors[i].pin_count == 0)
    		{
    			return MINIBASE_FIRST_ERROR(BUFMGR,BUFFERPAGENOTPINNED);
    		}

            if(descriptors[i].pageNumber == page_num)
            {
            //	descriptors[i].pageNumber = -1;
        		descriptors[i].pin_count -= 1;
        		descriptors[i].dirty = dirty;

        		if(descriptors[i].pin_count == 0 && hate == FALSE)
        		{
                    //descriptors[i].pageNumber = -1;
        			replacer->addtoLRUQueue(page_num);
        		}
        		else if(descriptors[i].pin_count == 0 && hate == TRUE)
        		{
                    //descriptors[i].pageNumber=-1;
        			replacer->addtoMRUStack(page_num);
        		}
                if(dirty==TRUE && descriptors[i].pin_count==0)
                {
                   Status writeStatus = MINIBASE_DB->write_page(descriptors[i].pageNumber,page);
                    if(writeStatus!=OK)
                        return MINIBASE_CHAIN_ERROR(BUFMGR,writeStatus);
                }


        		return OK;
            }
        }
    }

  return DONE;
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


            descriptors[i].pageNumber = firstPageId;
            descriptors[i].pin_count = 1;
            descriptors[i].dirty = false;
            int frameIndex = hash(firstPageId);
            firstpage = bufPool + i;
            HashEntry *entry = new HashEntry();
            entry->pageNumber = firstPageId;
            entry->frameNumber = i;
            entry->next = NULL;
            HashEntry *current = hashTable[frameIndex];
            if(current!=NULL)
            {
                while(current->next!=NULL)
                    current=current->next;
                current->next = entry;
            }
            else
            {
                hashTable[frameIndex] = entry;
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
    for(int i=0;i<numBuffers;i++)
    {
        if(descriptors[i].pageNumber==globalPageId && descriptors[i].pin_count!=0)
            return MINIBASE_FIRST_ERROR(BUFMGR,BUFFERPAGEPINNED);
        if(descriptors[i].pageNumber==globalPageId)
        {
            descriptors[i].pageNumber=-1;
            descriptors[i].pin_count = 0;
            Status deallocateStatus =  MINIBASE_DB->deallocate_page(globalPageId,1);
            if(deallocateStatus!=OK)
                return MINIBASE_CHAIN_ERROR(BUFMGR,deallocateStatus);
            int frameIndex = hash(globalPageId);
            HashEntry *entry = hashTable[frameIndex];
            while(entry)
            {
                if(entry->pageNumber == globalPageId)
                    break;
                entry = entry->next;
            }
            if(entry->next==NULL)
                delete(entry);
            else
            {
                HashEntry *next = entry->next;
                delete(entry);
                entry = next;
            }
        }
    }
  return OK;
}

//*************************************************************
//** This is the implementation of flushPage
//************************************************************
Status BufMgr::flushPage(PageId pageid) {
  // put your code here
    for(int i=0;i<numBuffers;i++)
    {
        if(descriptors[i].pageNumber==pageid)
        {
//            if(descriptors[i].pin_count!=0)
//                return MINIBASE_FIRST_ERROR(BUFMGR,BUFFERPAGEPINNED);
            int frameIndex = hash(pageid);
            HashEntry *entry = hashTable[frameIndex];
            while(entry!=NULL)
            {
                   if(entry->pageNumber == pageid)
                       break;
                entry = entry->next;
            }
            if(entry==NULL)
                return MINIBASE_FIRST_ERROR(BUFMGR,HASHNOTFOUND);
            int frameNumber = entry->frameNumber;
            Page *page = &bufPool[frameNumber];
            Status writeStatus = MINIBASE_DB->write_page(pageid,page);
            if(writeStatus!=OK)
                return MINIBASE_CHAIN_ERROR(BUFMGR,writeStatus);


            return OK;
        }
    }
  return MINIBASE_FIRST_ERROR(BUFMGR,BUFFERPAGENOTFOUND);
}

//*************************************************************
//** This is the implementation of flushAllPages
//************************************************************
Status BufMgr::flushAllPages(){
  //put your code here
    for(int i=0;i<numBuffers;i++)
    {
        if(descriptors[i].pageNumber!=-1)
        {
            Status status = flushPage(descriptors[i].pageNumber);
            if(status!=OK)
                return MINIBASE_FIRST_ERROR(BUFMGR,INTERNALERROR);
        }
    }
  return OK;
}


/*** Methods for compatibility with project 1 ***/
//*************************************************************
//** This is the implementation of pinPage
//************************************************************
Status BufMgr::pinPage(PageId PageId_in_a_DB, Page*& page, int emptyPage, const char *filename){
  //put your code here
 return pinPage(PageId_in_a_DB,page,emptyPage);
}

//*************************************************************
//** This is the implementation of unpinPage
//************************************************************
Status BufMgr::unpinPage(PageId globalPageId_in_a_DB, int dirty, const char *filename){
  //put your code here

  return unpinPage(globalPageId_in_a_DB,dirty,FALSE);
}

//*************************************************************
//** This is the implementation of getNumUnpinnedBuffers
//************************************************************
unsigned int BufMgr::getNumUnpinnedBuffers(){
    int unpinned = 0;
    for(int i=0;i<numBuffers;i++)
    {
        if(descriptors[i].pin_count==0)
            unpinned++;
    }
    return unpinned;
}

int BufMgr::hash(int pageNumber)
{
    return (2*pageNumber + 3)%HTSIZE;
}

PageReplacer::PageReplacer() {}

void PageReplacer::addtoLRUQueue(PageId pageNumber) {
         LRUQueue.push(pageNumber);
}

PageId PageReplacer::getVictim() {
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

void PageReplacer::addtoMRUStack(PageId pageNumber) {
    MRUStack.push(pageNumber);
}