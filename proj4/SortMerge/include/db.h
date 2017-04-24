#ifndef DB_H
#define DB_H

#include <string.h>
#include <stdlib.h>
#include "page.h"


// Each database is basically a UNIX file and consists of several relations
// (viewed as heapfiles and their indexes) within it.

// Class definition for DB which manages a database.


const unsigned MAX_NAME = 50;
  // This is the maximum length of the name of a "file" within a database.
  

class DB
{
public:
    // Constructors
    // Create a database with the specified number of pages where the page
    // size is the default page size.
    DB( const char* name, unsigned num_pages, Status& status );

    // Open the database with the given name.
    DB( const char* name, Status& status );

    // Destructor : closes the database
    ~DB();

    // Destroy the database.
    Status db_destroy();


    // Functions to return some characteristics of the database.
    const char* db_name() const;
    int db_num_pages() const;
    int db_page_size() const;


    // Allocate a set of pages where the run size is taken to be 1 by default.
    // Gives back the page number of the first page of the allocated run.
    Status allocate_page(PageId& start_page_num, int run_size = 1);

    // Deallocate a set of pages starting at the specified page number and
    // a run size can be specified.
    Status deallocate_page(PageId start_page_num, int run_size = 1);


    // Adds a file entry to the header page(s).
    Status add_file_entry(const char* fname, PageId start_page_num);

    // Delete the entry corresponding to a file from the header page(s).
    Status delete_file_entry(const char* fname);

    // Get the entry corresponding to the given file.
    Status get_file_entry(const char* name, PageId& start_pg);



    // Read the contents of the specified page into the given memory area.
    Status read_page(PageId pageno, Page* pageptr);

    // Write the contents of the specified page.
    Status write_page(PageId pageno, Page* pageptr);

    // Print out the space map of the database.
    Status dump_space_map();

    enum {
        DB_FULL,
        DUPLICATE_ENTRY,
        UNIX_ERROR,
        BAD_PAGE_NO,
        FILE_IO_ERROR,
        FILE_NOT_FOUND,
        FILE_NAME_TOO_LONG,
	NEG_RUN_SIZE
   };

private:
    int fd;
    unsigned num_pages;
    char* name;


    struct file_entry
    {
        PageId pagenum;         // INVALID_PAGE if no entry.
        char   fname[MAX_NAME];
    };

    struct directory_page
    {
        PageId     next_page;
        unsigned   num_entries;
        file_entry entries[0];  // Variable-sized struct
    };

      // A first_page structure appears on the first page of the database.
    struct first_page
    {
        unsigned num_db_pages;  // How big the database is.
        directory_page dir;     // The first page's directory starts here.
    };               


      /* Internal structure of a Minibase DB:

         The DB keeps two basic kinds of global information.  The first is a
         map of which database pages have been allocated.  The second is a
         directory of "files" created within the database.

         The first page of the database (page ID 0) is reserved for a special
         structure that holds global information about the database, like the
         number of pages in the database.  After this special first-page
         information comes the first (of possibly many) "directory page."  A
         directory page is where the DB keeps track of the files created within
         the database.

         The second page (page ID 1), and as many subsequent pages as needed,
         holds the "space map," which is a bit map representing pages allocated
         in the database.

       */


      // Set runsize bits starting from start to value specified
    Status set_bits( PageId start, unsigned runsize, int bit );

      // Initializes the given directory page.
    void init_dir_page( directory_page* dp, unsigned used_bytes );


};

#endif
