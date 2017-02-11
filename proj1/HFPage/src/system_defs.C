//
// filename system_defs.C 
//
// Ranjani Ramamurthy, Dec 3, 1995

#include <new>
#include <stdio.h>
#include "minirel.h"
#include "db.h"
#include "buf.h"

using namespace std;

SystemDefs* minibase_globals;
extern int MINIBASE_RESTART_FLAG;

ostream& operator<< (ostream& out, const struct RID rid)
{
    out << "[" << rid.pageNo << "/" << rid.slotNo << "]";
    return(out);
};

// constructor to start the system.

SystemDefs::SystemDefs( Status& status, const char* dbname, const char* logname,
                        unsigned num_pgs, unsigned logsize,
                        unsigned bufpoolsize, const char* replacement_policy )
{
    char real_logname[ strlen(logname) + 20 ];
    char real_dbname[ strlen(dbname) + 20 ];

    strcpy(real_dbname,dbname);
    strcpy(real_logname,logname);


    init( status, real_dbname,real_logname, num_pgs, logsize,
          bufpoolsize? bufpoolsize : NUMBUF, replacement_policy? replacement_policy : "Clock" );
}

SystemDefs::SystemDefs( Status& status, const char* dbname, unsigned num_pgs,
                        unsigned bufpoolsize, const char* replacement_policy )
{
    char logname[ strlen(dbname) + 20 ];
    char real_dbname[ strlen(dbname) + 20 ];

    sprintf(logname, "%s-log", dbname);
    strcpy(real_dbname,dbname);


    init( status, real_dbname, logname, num_pgs, num_pgs? 3*num_pgs : 500,
          bufpoolsize? bufpoolsize : NUMBUF,
          replacement_policy? replacement_policy : "Clock" );
}

void SystemDefs::init( Status& status, const char* dbname, const char* logname,
                       unsigned num_pgs, unsigned ,
                       unsigned bufpoolsize, const char* )
{
    status = OK;
    char* BufMgrAddress;

    GlobalBufMgr = 0;
    GlobalDB = 0;
    GlobalCatalogPtr = 0;       // Kill any users---they must use ExtSysDefs.
    GlobalDBName = 0;
    GlobalLogName = 0;
#define GlobalShMemMgr this

    minibase_globals = this;


          // create the buffer manager in shared memory
          // this needs to be changed later to merely the buffer pool.

        BufMgrAddress = GlobalShMemMgr->malloc(sizeof(BufMgr));
        GlobalBufMgr = new(BufMgrAddress) BufMgr(bufpoolsize);

        GlobalDBName = GlobalShMemMgr->malloc(strlen(dbname)+1);
        strcpy(GlobalDBName,dbname);

        GlobalLogName = GlobalShMemMgr->malloc(strlen(logname)+1);
        strcpy(GlobalLogName,logname);


      // create or open the DB 
    if ((MINIBASE_RESTART_FLAG) || (num_pgs == 0)){// open an existing database
        GlobalDB = new DB(dbname,status);
        if (status != OK) {
            cerr << "Error opening Database " << dbname << endl;
            minibase_errors.show_errors();
            return;
        }
    } else {
        GlobalDB = new DB(dbname,num_pgs,status);
        if (status != OK) {
            cerr << "Error creating Database " << dbname << endl;
            minibase_errors.show_errors();
            return;
        }
        
        status = GlobalBufMgr->flushAllPages();
        if (status != OK) {
            cerr << "Error flushing buffer pool pages\n" << endl;
            minibase_errors.show_errors();
            return;
        }
    }


}


SystemDefs::~SystemDefs()
{
  
      /* The buffer manager needs the GlobalDb to still exist when it is
         deleted. */

    delete GlobalBufMgr;   GlobalBufMgr = NULL;
    delete GlobalDBName; GlobalDBName = NULL;
    delete GlobalLogName; GlobalLogName = NULL;

  delete GlobalDB; GlobalDB = NULL; // no dependency
  minibase_globals = 0; 
}

