#include "heapfile.h"
#include <vector>
#include "buf.h"

// ******************************************************
// Error messages for the heapfile layer

static const char *hfErrMsgs[] = {
    "bad record id",
    "bad record pointer", 
    "end of file encountered",
    "invalid update operation",
    "no space on page for record", 
    "page is empty - no records",
    "last record on page",
    "invalid slot number",
    "file has already been deleted",
};

typedef struct{
    int headerPageId;
    vector<HFPage*> pages;
}directory;          //mapping of page IDs to directory pages

vector<directory> dirs;    //list of all directories for all files

vector<HFPage*> directoryPages;   //holds the Directory pages for currently open file
string FileName;
static error_string_table hfTable( HEAPFILE, hfErrMsgs );


enum {CLEAN,DIRTY};
int recCount = 0;


// ********************************************************
// Constructor
HeapFile::HeapFile( const char *name, Status& returnStatus )
{
    PageId pg;
    if(strlen(name)>MAX_NAME)     //file name has a limit
    {
        returnStatus=FAIL;
        return;
    }
    Status status = MINIBASE_DB->get_file_entry(name,pg);   //check for file name entry in DB

    if(status==OK)         //already existing file
    {
        for(int i=0;i<dirs.size();i++)
        {
            if(dirs[i].headerPageId == pg)       //check for matching header page ID from list of directories and retrieve the directory pages
            {
                directoryPages = dirs[i].pages;
                break;
            }
        }
    } else
    {
        directoryPages.clear();   //otherwise clear the currently selected directory pages
    }
    this->fileName = new char[strlen(name)];
    for(int i=0;i<strlen(name);i++)
        this->fileName[i]=name[i];
    this->file_deleted = 0;



    FileName = name;
    // fill in the body
    returnStatus = OK;
   
}

// ******************
// Destructor
HeapFile::~HeapFile()
{
   // fill in the body 

}

// *************************************
// Return number of records in heap file
int HeapFile::getRecCnt()
{
   // fill in the body
    int count =0;
    for(int i=0;i<directoryPages.size();i++)
    {
        HFPage *hfpage = directoryPages[i];
        Page *page = (Page *)hfpage;
        Status status = MINIBASE_BM->pinPage(hfpage->page_no(),page,hfpage->empty(),this->fileName);
        if(status!=OK)
            return  status;
        RID rid,tempRid;
        status = hfpage->firstRecord(rid);
        DataPageInfo *info;
        char *record;
        int recLen;
        while(status==OK)
        {
            hfpage->returnRecord(rid,record,recLen);
            info = (DataPageInfo *)record;
            count += info->recct;

            tempRid=rid;
            status = hfpage->nextRecord(tempRid,rid);
        }
        status = MINIBASE_BM->unpinPage(hfpage->page_no(),CLEAN,this->fileName);
        if(status!=OK)
            return status;
    }
   return count;
}

// *****************************
// Insert a record into the file
Status HeapFile::insertRecord(char *recPtr, int recLen, RID& outRid)
{
    if(recPtr==NULL)
        return MINIBASE_FIRST_ERROR(HEAPFILE,BAD_REC_PTR);

    if(recLen>=MINIBASE_PAGESIZE)
        return MINIBASE_FIRST_ERROR(HEAPFILE,NO_SPACE);
    // fill in the body
    for(int i=0;i<directoryPages.size();i++) {
        HFPage *hfpage = directoryPages[i];
        Page *page = (Page *) hfpage;
        Status status = MINIBASE_BM->pinPage(hfpage->page_no(), page, hfpage->empty(), this->fileName);
        if (status != OK) {
            return status;
        }
        RID rid, currId;
        status = hfpage->firstRecord(rid);
        if (status != OK)
        {
           return status;
        }
        char *record;
        int len;
        while (status == OK) {
            Status returnStatus = hfpage->returnRecord(rid, record, len);
            if (returnStatus != OK)
                return returnStatus;
            DataPageInfo *info = (DataPageInfo *) record;
            if (info->availspace > recLen) {
                outRid.pageNo = info->pageId;
                Page *dataPage;
                Status pinStatus = MINIBASE_BM->pinPage(info->pageId, dataPage, 0, this->fileName);
                if (pinStatus != OK)
                    return pinStatus;
                HFPage *hfdatapage = (HFPage *) dataPage;
                Status insertStatus = hfdatapage->insertRecord(recPtr, recLen, outRid);
                if(insertStatus!=OK)
                    return MINIBASE_CHAIN_ERROR(HEAPFILE,insertStatus);
                info->availspace = hfdatapage->available_space();
                info->recct += 1;
                recCount+=1;

                pinStatus = MINIBASE_BM->unpinPage(info->pageId, DIRTY, this->fileName);
                if (pinStatus != OK)
                    return MINIBASE_CHAIN_ERROR(BUFMGR,pinStatus);
                pinStatus = MINIBASE_BM->unpinPage(hfpage->page_no(), DIRTY, this->fileName);
                if (pinStatus != OK)
                    return MINIBASE_CHAIN_ERROR(BUFMGR,pinStatus);
             //   cout<<"Record count is "<<getRecCnt();
               // recCount++;
              //  cout<<"Record count is "<<recCount<<endl;
                return insertStatus;

            }
            currId = rid;
            status = hfpage->nextRecord(currId, rid);

        }
        Status unpinStatus = MINIBASE_BM->unpinPage(hfpage->page_no(),CLEAN,this->fileName);
        if(unpinStatus!=OK)
            return unpinStatus;

    }
    //No existing datapage has space left for record, create new data page
    DataPageInfo *info = new DataPageInfo();
    Page *page;
    Status newStatus = newDataPage(info);

    if(newStatus!=OK)
        return newStatus;

    info->recct += 1;
    recCount+=1;

    PageId dirId,dataId;
    RID dirRid;
    Page *pg;

    //create directory entry for new page
    Status allocStatus = allocateDirSpace(info,dirId,dirRid);
    MINIBASE_BM->pinPage(dirId,pg,0,this->fileName);

    HFPage *dirhfpage = (HFPage *)pg;
    char *rec;
    int reclength;
    Status returnStatus = dirhfpage->returnRecord(dirRid,rec,reclength);
    if(returnStatus !=OK)
        return MINIBASE_FIRST_ERROR(HEAPFILE,RECNOTFOUND);
    DataPageInfo *info1 = (DataPageInfo*)rec;
    if(allocStatus!=OK)
        return allocStatus;

    //pin datapage for insertion


    Status pinStatus = MINIBASE_BM->pinPage(info->pageId,page,0,this->fileName);
    if(pinStatus!=OK)
        return MINIBASE_CHAIN_ERROR(BUFMGR,pinStatus);
    HFPage *hfpage = (HFPage *)page;
    Status insertStatus = hfpage->insertRecord(recPtr,recLen,outRid);

    info1->availspace = hfpage->available_space();

    pinStatus = MINIBASE_BM->unpinPage(dirId,DIRTY,this->fileName);
    if(pinStatus!=OK)
        return MINIBASE_CHAIN_ERROR(BUFMGR,pinStatus);
    //unpin to save datapage
    Status unPinStatus = MINIBASE_BM->unpinPage(info->pageId,DIRTY,this->fileName);

    if(unPinStatus!=OK)
    {
          // return unPinStatus;
       return MINIBASE_CHAIN_ERROR(BUFMGR,unPinStatus);
    }

    return insertStatus;
}




// ***********************
// delete record from file
Status HeapFile::deleteRecord (const RID& rid)
{
    if(rid.slotNo < 0)
        return MINIBASE_FIRST_ERROR(HEAPFILE,INVALID_SLOTNO);
  // fill in the body
    for(int i=0;i<directoryPages.size();i++)
    {
        HFPage *hfPage = directoryPages[i];
        Page *dirPage = (Page *)dirPage;
        Status pinStatus = MINIBASE_BM->pinPage(hfPage->page_no(),dirPage,0,this->fileName);
        if(pinStatus!=OK)
            return MINIBASE_CHAIN_ERROR(BUFMGR,pinStatus);
        RID currId,temp;
        char *record;
        int recLen;
        Status status = hfPage->firstRecord(currId);
        DataPageInfo *info;
        while(status == OK)
        {
            status = hfPage->returnRecord(currId,record,recLen);
            if(status!=OK)
                return status;
            info = (DataPageInfo *)record;
            if(info->pageId==rid.pageNo)
            {
                break;
            }
            temp=currId;
            status = hfPage->nextRecord(temp,currId);
        }
        if(status==OK)//record found
        {
           Page *dataPage;
            MINIBASE_BM->pinPage(rid.pageNo,dataPage,0,this->fileName);
            HFPage *dp = (HFPage *)dataPage;
            Status deleteStatus = dp->deleteRecord(rid);
            if(deleteStatus!=OK)
                return deleteStatus;
            MINIBASE_BM->unpinPage(rid.pageNo,DIRTY,this->fileName);
            info->availspace = dp->available_space();
            info->recct -= 1;
            MINIBASE_BM->unpinPage(hfPage->page_no(),DIRTY,this->fileName);
            return deleteStatus;
        }
        Status unpinStatus = MINIBASE_BM->unpinPage(hfPage->page_no(),CLEAN,this->fileName);
        if(unpinStatus!=OK)
            return unpinStatus;
    }
    return DONE;

}

// *******************************************
// updates the specified record in the heapfile.
Status HeapFile::updateRecord (const RID& rid, char *recPtr, int recLen)
{
    if(recPtr==NULL)
                  return MINIBASE_FIRST_ERROR(HEAPFILE,BAD_REC_PTR);
    if(rid.slotNo < 0)
    {
        return MINIBASE_FIRST_ERROR(HEAPFILE,INVALID_SLOTNO);
    }

    for(int i=0;i<directoryPages.size();i++)
    {
        HFPage *hfpage = directoryPages[i];
        Page *page = (Page *)hfpage;
        Status pinStatus = MINIBASE_BM->pinPage(hfpage->page_no(),page,0,this->fileName);
        if(pinStatus!=OK)
            return MINIBASE_CHAIN_ERROR(BUFMGR,pinStatus);
        RID currId,temp;
        Status status = hfpage->firstRecord(currId);
        char *record;
        int recLength;
        while(status==OK)
        {
            Status returnStatus = hfpage->returnRecord(currId,record,recLength);
            if(returnStatus!=OK)
                return MINIBASE_FIRST_ERROR(HEAPFILE,RECNOTFOUND);
            DataPageInfo *info = (DataPageInfo *)record;
            if(info->pageId==rid.pageNo)
            {
                Page *dataPage;
                Status pinStatus = MINIBASE_BM->pinPage(rid.pageNo,dataPage,0,this->fileName);
                if(pinStatus!=OK)
                    return MINIBASE_CHAIN_ERROR(BUFMGR,pinStatus);
                HFPage *hfDataPage = (HFPage *)dataPage;
                char *originalRecord;
                int len;
                Status status = hfDataPage->returnRecord(rid,originalRecord,len);
                if(status!=OK)
                    return MINIBASE_FIRST_ERROR(HEAPFILE,RECNOTFOUND);
                if(len!=recLen)
                    return MINIBASE_FIRST_ERROR(HEAPFILE,INVALID_UPDATE);
                memcpy(originalRecord,recPtr,recLen);
                pinStatus = MINIBASE_BM->unpinPage(rid.pageNo,DIRTY,this->fileName);
                if(pinStatus!=OK)
                    return MINIBASE_CHAIN_ERROR(BUFMGR,pinStatus);
                pinStatus = MINIBASE_BM->unpinPage(hfpage->page_no(),CLEAN,this->fileName);
                if(pinStatus!=OK)
                    return MINIBASE_CHAIN_ERROR(BUFMGR,pinStatus);
                return status;
            }
            temp = currId;
            status = hfpage->nextRecord(temp,currId);
        }
       Status unpinStatus= MINIBASE_BM->unpinPage(hfpage->page_no(),CLEAN,this->fileName);
        if(unpinStatus!=OK)
            return MINIBASE_CHAIN_ERROR(BUFMGR,unpinStatus);
    }

  // fill in the body
  return OK;
}

// ***************************************************
// read record from file, returning pointer and length
Status HeapFile::getRecord (const RID& rid, char *recPtr, int& recLen)
{
  // fill in the body
  for(int i=0;i<directoryPages.size();i++)
  {
      HFPage *hfpage = directoryPages[i];
      Page *page = (Page *)hfpage;
      Status pinStatus = MINIBASE_BM->pinPage(hfpage->page_no(),page,0,this->fileName);
      if(pinStatus!=OK)
          return MINIBASE_CHAIN_ERROR(BUFMGR,pinStatus);
      RID curRid,temp;
      Status status = hfpage->firstRecord(curRid);
      char *record;
      int recLength;
      while(status == OK)
      {
          Status ret = hfpage->returnRecord(curRid,record,recLength);
          if(ret!=OK)
              return MINIBASE_FIRST_ERROR(HEAPFILE,RECNOTFOUND);
          DataPageInfo *info = (DataPageInfo *)record;
          if(info->pageId == rid.pageNo)
          {
              Page *dataPage;
              pinStatus = MINIBASE_BM->pinPage(rid.pageNo,dataPage,0,this->fileName);
              if(pinStatus!=OK)
                  return MINIBASE_CHAIN_ERROR(BUFMGR,pinStatus);
              HFPage *dp = (HFPage *)dataPage;
              char *record;
              Status returnStatus = dp->returnRecord(rid,record,recLen);
              if(returnStatus!=OK)
                  return MINIBASE_CHAIN_ERROR(BUFMGR,returnStatus);
              memcpy(recPtr,record,recLen);
              if(returnStatus!=OK)
                  return returnStatus;
              pinStatus = MINIBASE_BM->unpinPage(rid.pageNo,CLEAN,this->fileName);
              if(pinStatus!=OK)
                  return MINIBASE_CHAIN_ERROR(BUFMGR,pinStatus);
              pinStatus = MINIBASE_BM->unpinPage(hfpage->page_no(),CLEAN,this->fileName);
              if(pinStatus!=OK)
                  return MINIBASE_CHAIN_ERROR(BUFMGR,pinStatus);
              return returnStatus;
          }
          temp = curRid;
          status = hfpage->nextRecord(temp,curRid);
      }
      pinStatus = MINIBASE_BM->unpinPage(hfpage->page_no(),CLEAN,this->fileName);
      if(pinStatus!=OK)
          return MINIBASE_CHAIN_ERROR(BUFMGR,pinStatus);
  }
  return DONE;
}

// **************************
// initiate a sequential scan
Scan *HeapFile::openScan(Status& status)
{
    Scan *scan = new Scan(this,status);


  return scan;
}

// ****************************************************
// Wipes out the heapfile from the database permanently. 
Status HeapFile::deleteFile()
{
    Status status = MINIBASE_DB->delete_file_entry(this->fileName)  ;
    this->file_deleted=1;
    int index;
    for(int i=0;i<dirs.size();i++)
    {
        if(dirs[i].headerPageId == directoryPages[0]->page_no())
        {
            index=i;
            break;
        }
    }
    dirs.erase(dirs.begin()+index);
    return status;
    // fill in the body

}

// ****************************************************************
// Get a new datapage from the buffer manager and initialize dpinfo
// (Allocate pages in the db file via buffer manager)
Status HeapFile::newDataPage(DataPageInfo *dpinfop)
{

    if(dpinfop==NULL)
    {
        return FAIL;
    }
    int dataPageId;
    Page *dataPageVar;
    HFPage *dataPage;
        Status newStatus = MINIBASE_BM->newPage(dataPageId,dataPageVar,1);
    if(newStatus!=OK)
        return MINIBASE_CHAIN_ERROR(BUFMGR,newStatus);
       dataPage = (HFPage *)dataPageVar;
        dataPage->init(dataPageId);


        dpinfop->availspace = dataPage->available_space();
        dpinfop->pageId = dataPageId;
        dpinfop->recct = 0;

        Status status = MINIBASE_BM->unpinPage(dataPageId,DIRTY,this->fileName);
        if(status!=OK)
            return MINIBASE_CHAIN_ERROR(BUFMGR,status);
        return OK;



}

// ************************************************************************
// Internal HeapFile function (used in getRecord and updateRecord): returns
// pinned directory page and pinned data page of the specified user record
// (rid).
//
// If the user record cannot be found, rpdirpage and rpdatapage are 
// returned as NULL pointers.
//
Status HeapFile::findDataPage(const RID& rid,
                    PageId &rpDirPageId, HFPage *&rpdirpage,
                    PageId &rpDataPageId,HFPage *&rpdatapage,
                    RID &rpDataPageRid)
{
    rpdirpage = NULL;
    rpdatapage = NULL;
    // fill in the body
    for(int i=0;i<directoryPages.size();i++)
    {
        HFPage *p = directoryPages[i];
        int pageNumber = p->page_no();
        RID dirEntryId;
        Page *page,*dp;
        Status status = p->firstRecord(dirEntryId);
        while(status==OK)
        {
            DataPageInfo *info;
            char *record;
            int val;
            Status getStatus = p->getRecord(dirEntryId,record,val);
            if(getStatus!=OK)
                return MINIBASE_CHAIN_ERROR(HEAPFILE,getStatus);
            info = (DataPageInfo *)record;
            if(info->pageId==rid.pageNo)
            {
              //  cout<<"Pin directory page "<<endl;
                Status pinStatus = MINIBASE_BM->pinPage(pageNumber,page,0,this->fileName);
                if(pinStatus!=OK)
                    return MINIBASE_CHAIN_ERROR(BUFMGR,pinStatus);
             //   cout<<"Pin data page"<<endl;
                pinStatus = MINIBASE_BM->pinPage(rid.pageNo,dp,0,this->fileName);
                if(pinStatus!=OK)
                    return MINIBASE_CHAIN_ERROR(BUFMGR,pinStatus);
                rpdatapage = (HFPage *)dp;
                rpdirpage = (HFPage *)page;
                rpDirPageId = pageNumber;
                rpDataPageId = rid.pageNo;
                rpDataPageRid = rid;
                return OK;
            }
            RID currId = dirEntryId;
            status = p->nextRecord(currId,dirEntryId);
        }

    }

    return DONE;
}

// *********************************************************************
// Allocate directory space for a heap file page 

Status HeapFile::allocateDirSpace(struct DataPageInfo * dpinfop,
                            PageId &allocDirPageId,
                            RID &allocDataPageRid)
{
    RID rid;
    int pageId;
    Page *pg;
    int i;
    //check existing pages
    for(int i=0;i<directoryPages.size();i++)
    {
        HFPage *page = directoryPages[i];

        if(page->available_space()>sizeof(DataPageInfo))
        {
            pg = (Page *)page;
          //  int num = MINIBASE_BM->getNumUnpinnedBuffers();
            allocDirPageId = page->page_no();
            Status pinStatus = MINIBASE_BM->pinPage(allocDirPageId,pg,page->empty(),this->fileName);
            if(pinStatus!=OK)
                return MINIBASE_CHAIN_ERROR(BUFMGR,pinStatus);

            Status status = page->insertRecord((char *)dpinfop,sizeof(DataPageInfo),allocDataPageRid);
            if(status!=OK)
                return MINIBASE_CHAIN_ERROR(HEAPFILE,status);
            Status unpinStatus = MINIBASE_BM->unpinPage(allocDirPageId,DIRTY,this->fileName);
            if(unpinStatus!=OK)
                return MINIBASE_CHAIN_ERROR(BUFMGR,unpinStatus);
          //  num = MINIBASE_BM->getNumUnpinnedBuffers();
            if(status==OK)
            {
                return OK;
            }
        }
    }


    //no space, so create new directory page
    HFPage *dirPage;

    Status newStatus = MINIBASE_BM->newPage(pageId,pg,1);
    if(newStatus!=OK)
        return MINIBASE_CHAIN_ERROR(BUFMGR,newStatus);
    dirPage = (HFPage *)pg;
    dirPage->init(pageId);
    allocDirPageId = dirPage->page_no();

    Status status = dirPage->insertRecord((char *)dpinfop,sizeof(DataPageInfo),allocDataPageRid);
    if(status!=OK)
        return MINIBASE_CHAIN_ERROR(HEAPFILE,status);
    status = MINIBASE_BM->unpinPage(allocDirPageId,DIRTY,this->fileName);
    if(status!=OK)
        return MINIBASE_CHAIN_ERROR(BUFMGR,status);
    directoryPages.push_back(dirPage);
    if(directoryPages.size()==1)
    {
        Status status = MINIBASE_DB->add_file_entry(this->fileName,dirPage->page_no());
        directory d;
        d.pages = directoryPages;
        d.headerPageId = dirPage->page_no();
        dirs.push_back(d);
    } else
    {
        HFPage *pg = directoryPages[0];
        for(int i=0;i<dirs.size();i++)
        {
            if(dirs[i].headerPageId==pg->page_no())
                dirs[i].pages = directoryPages;
        }
    }

    return status;
}

// *******************************************
