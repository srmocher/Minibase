// -*- C++ -*-
#ifndef _SYSTEM_DEFS_H
#define _SYSTEM_DEFS_H
/////////////////////////////////////////////////////////////////
//
// filename : system_defs.h    
//
// This defines the class for system startup and all macros
// to define the global variables.
//
//
/////////////////////////////////////////////////////////////////


class BufMgr;
class DB;
class Catalog;

#define MINIBASE_MAXARRSIZE 50

class SystemDefs
{

public:
    SystemDefs( Status& status, const char* dbname, unsigned dbpages =0,
                unsigned bufpoolsize =0, const char* replacement_policy =0 );
      /* This constructor uses a default log name and size, for multi-user
         Minibase.  For single-user Minibase, this is the designated
         constructor.  If "dbpages" is 0, the database is opened; if it is
         greater than 0, the database is created with that number of pages. */


    SystemDefs( Status& status, const char* dbname, const char* logname,
                unsigned dbpages, unsigned maxlogsize,
                unsigned bufpoolsize =0, const char* replacement_policy =0 );
      /* This constructor lets you specify all aspects of the system. */


    virtual ~SystemDefs();


    BufMgr*             GlobalBufMgr;

      /* We fake shared memory in single-user Minibase to simplify the
         maintenance of the two versions. */
    char* malloc( unsigned size )
        { return new char[size]; }

#define  MINIBASE_SHMEM minibase_globals

      // These are not in shared memory.
    DB*                 GlobalDB;
    Catalog*            GlobalCatalogPtr;
      /* The global catalog object is declared here, but not allocated by the
         SystemDefs constructor.  If you need to use the catalog, use the
         ExtendedSystemDefs constructor, declared in ext_sys_defs.h. */

    char*               GlobalDBName;
    char*               GlobalLogName;

protected:
    void init( Status& status, const char* dbname, const char* logname,
               unsigned dbpages, unsigned maxlogsize,
               unsigned bufpoolsize, const char* replacement_policy );
};

extern SystemDefs* minibase_globals;

#define  MINIBASE_DB                    (minibase_globals->GlobalDB)
#define  MINIBASE_BM                    (minibase_globals->GlobalBufMgr)


#define  MINIBASE_DBNAME                (minibase_globals->GlobalDBName)

#endif // _SYSTEM_DEFS_H
