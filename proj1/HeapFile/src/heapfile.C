#include "heapfile.h"
#include <vector>

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

static error_string_table hfTable( HEAPFILE, hfErrMsgs );


enum {CLEAN,DIRTY};
int recCount = 0;


// ********************************************************
// Constructor
HeapFile::HeapFile( const char *name, Status& returnStatus )
{
    this->fileName = new char[strlen(name)];
    for(auto i=0;i<strlen(name);i++)
        this->fileName[i]=name[i];
    this->file_deleted = 0;
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
    // fill in the body
    for(int i=0;i<directoryPages.size();i++) {
        HFPage *hfpage = directoryPages[i];
        Page *page = (Page *) &hfpage;
        Status status = MINIBASE_BM->pinPage(hfpage->page_no(), page, hfpage->empty(), this->fileName);
        if (status != OK) {
            return status;
        }
        RID rid, currId;
        status = hfpage->firstRecord(rid);
        if (status != OK)
            return status;
        char *record;
        int len;
        while (status == OK) {
            Status returnStatus = hfpage->returnRecord(rid, record, len);
            if (returnStatus != OK)
                return returnStatus;
            DataPageInfo *info = (DataPageInfo *) record;
            if (info->availspace >= recLen) {
                outRid.pageNo = info->pageId;
                Page *dataPage;
                auto pinStatus = MINIBASE_BM->pinPage(info->pageId, dataPage, 0, this->fileName);
                if (pinStatus != OK)
                    return pinStatus;
                HFPage *hfdatapage = (HFPage *) dataPage;
                Status insertStatus = hfdatapage->insertRecord(recPtr, recLen, outRid);
                info->availspace = hfdatapage->available_space();
                info->recct += 1;

                pinStatus = MINIBASE_BM->unpinPage(info->pageId, DIRTY, this->fileName);
                if (pinStatus != OK)
                    return pinStatus;
                pinStatus = MINIBASE_BM->unpinPage(hfpage->page_no(), DIRTY, this->fileName);
                if (pinStatus != OK)
                    return pinStatus;
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
    info->availspace = info->availspace-recLen;
    info->recct += 1;
    PageId dirId,dataId;
    RID dirRid;
   // Status unpinStatus = MINIBASE_BM->unpinPage(info->pageId,1,this->fileName);
    //create directory entry for new page
    Status allocStatus = allocateDirSpace(info,dirId,dirRid);

    if(allocStatus!=OK)
        return allocStatus;

    //pin datapage for insertion


    Status pinStatus = MINIBASE_BM->pinPage(info->pageId,page,0,this->fileName);
    HFPage *hfpage = (HFPage *)page;
    Status insertStatus = hfpage->insertRecord(recPtr,recLen,outRid);
    //unpin to save datapage
    Status unPinStatus = MINIBASE_BM->unpinPage(info->pageId,DIRTY,this->fileName);
    if(unPinStatus!=OK)
    {

        return unPinStatus;
    }
    int num = MINIBASE_BM->getNumUnpinnedBuffers();
    int num2 = MINIBASE_BM->getNumBuffers();
   // recCount++;
   // cout<<"New page Record count is "<<recCount<<endl;
    return insertStatus;
}




// ***********************
// delete record from file
Status HeapFile::deleteRecord (const RID& rid)
{
  // fill in the body
    for(int i=0;i<directoryPages.size();i++)
    {
        HFPage *hfPage = directoryPages[i];
        Page *dirPage = (Page *)dirPage;
        MINIBASE_BM->pinPage(hfPage->page_no(),dirPage,0,this->fileName);
        RID currId,temp;
        char *record;
        int recLen;
        Status status = hfPage->firstRecord(currId);
        DataPageInfo *info;
        while(status == OK)
        {
            Status status = hfPage->returnRecord(currId,record,recLen);
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
    for(int i=0;i<directoryPages.size();i++)
    {
        HFPage *hfpage = directoryPages[i];
        Page *page = (Page *)hfpage;
        MINIBASE_BM->pinPage(hfpage->page_no(),page,0,this->fileName);
        RID currId,temp;
        Status status = hfpage->firstRecord(currId);
        char *record;
        int recLen;
        while(status==OK)
        {
            Status returnStatus = hfpage->returnRecord(currId,record,recLen);
            if(returnStatus!=OK)
                return returnStatus;
            DataPageInfo *info = (DataPageInfo *)record;
            if(info->pageId==rid.pageNo)
            {
                Page *dataPage;
                MINIBASE_BM->pinPage(rid.pageNo,dataPage,0,this->fileName);
                HFPage *hfDataPage = (HFPage *)dataPage;
                char *originalRecord;
                int len;
                Status status = hfDataPage->returnRecord(rid,originalRecord,len);
                if(status!=OK)
                    return status;
                memcpy(originalRecord,recPtr,recLen);
                MINIBASE_BM->unpinPage(rid.pageNo,DIRTY,this->fileName);
                MINIBASE_BM->unpinPage(hfpage->page_no(),CLEAN,this->fileName);
                return status;
            }
            temp = currId;
            status = hfpage->nextRecord(temp,currId);
        }
        MINIBASE_BM->unpinPage(hfpage->page_no(),CLEAN,this->fileName);
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
      MINIBASE_BM->pinPage(hfpage->page_no(),page,0,this->fileName);
      RID curRid,temp;
      Status status = hfpage->firstRecord(curRid);
      char *record;
      int recLength;
      while(status == OK)
      {
          Status ret = hfpage->returnRecord(curRid,record,recLength);
          if(ret!=OK)
              return ret;
          DataPageInfo *info = (DataPageInfo *)record;
          if(info->pageId == rid.pageNo)
          {
              Page *dataPage;
              MINIBASE_BM->pinPage(rid.pageNo,dataPage,0,this->fileName);
              HFPage *dp = (HFPage *)dataPage;

              Status returnStatus = dp->returnRecord(rid,recPtr,recLen);
              if(returnStatus!=OK)
                  return returnStatus;
              MINIBASE_BM->unpinPage(rid.pageNo,CLEAN,this->fileName);
              MINIBASE_BM->unpinPage(hfpage->page_no(),CLEAN,this->fileName);
              return returnStatus;
          }
          temp = curRid;
          status = hfpage->nextRecord(temp,curRid);
      }
      MINIBASE_BM->unpinPage(hfpage->page_no(),CLEAN,this->fileName);
  }
  return DONE;
}

// **************************
// initiate a sequential scan
Scan *HeapFile::openScan(Status& status)
{
    Scan scan(this,status);

  // fill in the body 
  return NULL;
}

// ****************************************************
// Wipes out the heapfile from the database permanently. 
Status HeapFile::deleteFile()
{
    // fill in the body
    return OK;
}

// ****************************************************************
// Get a new datapage from the buffer manager and initialize dpinfo
// (Allocate pages in the db file via buffer manager)
Status HeapFile::newDataPage(DataPageInfo *dpinfop)
{
//    if(directoryPages.size()==0)
//    {
//        HFPage headerPage,dataPage;
//        Page *page,*dpage;
//        int pageId,dpageId;
//        MINIBASE_BM->newPage(pageId,page,1);
//        headerPage.init(pageId);
//        MINIBASE_BM->newPage(dpageId,dpage,1);
//        dataPage.init(dpageId);
//        dpinfop = new DataPageInfo();
//        dpinfop->availspace = dataPage.available_space();
//        dpinfop->recct = 0;
//        dpinfop->pageId = dpageId;
//        RID rid;
//      //  Status status = headerPage.insertRecord((char *)dpinfop,sizeof(DataPageInfo),rid);
//        if(status == OK)
//        {
//            //directoryPages.push_back(headerPage);
//            return OK;
//        }
//    }
//    else
//    {
//        int spaceRequired = sizeof(DataPageInfo);
//        HFPage dataPage;
//        int dataPageId,dpId;
//        Page *dataPageVar,*dirVar;
//        RID rid;
//        for(int i=0;i<directoryPages.size();i++)
//        {
//            HFPage dirPage = directoryPages[i];
//            if(dirPage.available_space()>=spaceRequired)
//            {
//                dpinfop = new DataPageInfo();
//                MINIBASE_BM->newPage(dataPageId,dataPageVar,1);
//                dataPage.init(dataPageId);
//                dpinfop->availspace = dataPage.available_space();
//                dpinfop->pageId = dataPageId;
//                dpinfop->recct = 0;
//                Status status = dirPage.insertRecord((char *)dpinfop,spaceRequired,rid);
//                if(status == OK)
//                {
//                    return OK;
//                }
//            }
//        }
        //HFPage dirPage;
      //  MINIBASE_BM->newPage(dpId,dirVar,1);
      //  dirPage.init(dpId);
    int dataPageId;
    Page *dataPageVar;
    HFPage *dataPage;
        MINIBASE_BM->newPage(dataPageId,dataPageVar,1);
       dataPage = (HFPage *)dataPageVar;
        dataPage->init(dataPageId);

      //  dpinfop = new DataPageInfo();
        dpinfop->availspace = dataPage->available_space();
        dpinfop->pageId = dataPageId;
        dpinfop->recct = 0;
        Status status = MINIBASE_BM->unpinPage(dataPageId,DIRTY,this->fileName);
        return OK;
       // Status status = dirPage.insertRecord((char *)dpinfop,spaceRequired,rid);
//        if(status ==OK)
//        {
//            directoryPages.push_back(dirPage);
//            return OK;
//        }


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
            p->getRecord(dirEntryId,record,val);
            info = (DataPageInfo *)record;
            if(info->pageId==rid.pageNo)
            {
                cout<<"Pin directory page "<<endl;
                MINIBASE_BM->pinPage(pageNumber,page,0,this->fileName);
                cout<<"Pin data page"<<endl;
                MINIBASE_BM->pinPage(rid.pageNo,dp,0,this->fileName);
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

    //check existing pages
    for(int i=0;i<directoryPages.size();i++)
    {
        HFPage *page = directoryPages[i];

        if(page->available_space()>=sizeof(DataPageInfo))
        {
            pg = (Page *)page;
            int num = MINIBASE_BM->getNumUnpinnedBuffers();
            allocDirPageId = page->page_no();
            Status pinStatus = MINIBASE_BM->pinPage(allocDirPageId,pg,page->empty(),this->fileName);

            Status status = page->insertRecord((char *)dpinfop,sizeof(DataPageInfo),allocDataPageRid);
            Status unpinStatus = MINIBASE_BM->unpinPage(allocDirPageId,DIRTY,this->fileName);
            num = MINIBASE_BM->getNumUnpinnedBuffers();
            if(status==OK)
            {
                return OK;
            }
        }
    }

    //no space, so create new directory page
    HFPage *dirPage;

    MINIBASE_BM->newPage(pageId,pg,1);
    dirPage = (HFPage *)pg;
    dirPage->init(pageId);
    allocDirPageId = dirPage->page_no();
 //   pg = (Page *)&dirPage;
   // MINIBASE_BM->pinPage(allocDirPageId,pg);

    Status status = dirPage->insertRecord((char *)dpinfop,sizeof(DataPageInfo),allocDataPageRid);
    if(status!=OK)
        return status;
    status = MINIBASE_BM->unpinPage(allocDirPageId,DIRTY,this->fileName);
    directoryPages.push_back(dirPage);
    return status;
}

// *******************************************
