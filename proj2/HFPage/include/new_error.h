#ifndef _NEW_ERROR_H
#define _NEW_ERROR_H

/* Written by Bill Kimmel in May 1995 as part of 764 minirel project.
   Revised by Michael Lee in Oct 1995, and by Luke Blanshard in March 1996.


HISTORY OF THIS PROTOCOL:
  It was inherited from the error protocol in place from the Spring 1994
  version of minirel.  In this version, variables of type Status were used to
  return errors from one function call to another.  No global maintenance was
  done.  This protocol was designed to be easily compatible with the old
  protocol (all that is needed are 30+ more Status types added to this file).


CONCEPT BEHIND THIS ERROR PROTOCOL:
  In a multi-layered system, errors should propagate up through the system to
  the top level.  In a multi-user environment, however, there are "parallel"
  hierarchies that call one another.  There is the "query" side and the
  "concurrency" side.  The process will bounce back and forth between these
  two, and a simple layered error handling system is not enough.  What is
  needed is the _path_ that brought the error to the top level (where was it
  first detected, and what functions were called when it was detected).  In
  effect, we want to look at the stack.


OUR APPROACH:
  Every subsystem creates error messages that describe the possible errors that
  will result.  When an error is detected by a subsystem for the first time,
  that subsystem adds an error message to the global queue.  The subsystem that
  discovered the error then returns a status code that indicates what subsystem
  it is: it identifies itself to the caller by returning its own ID.  If the
  caller cannot recover from the error, the caller must then append a new error
  to the global queue.  In this case, however, the thing appended to the queue
  is not a new message, but simply a pair of subsystem IDs: the original
  subsystem's ID, and the calling subsystem's ID.

  Here is a possible (hypothetical) example of the errors that will be logged
  in the global error object:

    DBMGR       "File Not Open"
    BUFMGR      DBMGR
    BTREE       BUFMGR
    JOINS       BTREE
    PLANNER     JOINS
    FRONTEND    PLANNER

  Note that there is some redundancy here: the recipient of an error is the
  source of an error in the next level.  This helps ensure that the protocol is
  being followed properly.

  The current protocol has all errors entered into a given variable of type
  Status.  This variable is checked.  If OK, then proceed.  If !OK, then call
  global_error::add_error(...).  All errors must then be returned to the
  caller.  Problem: destructors are unable to set a non-global variable to a
  value.  They can call global_error::add_error, but there is no way for them
  to communicate failures.


ERROR NUMBERS:
  When a subsystem posts a first error, it provides an error number that is
  specific to that subsystem.  Each subsystem has its own set of error numbers,
  independent of the other subsystems.  These numbers may become part of the
  subsystem's public interface---it is permissible to advertise what error
  numbers will be used in what circumstances, so that callers can recognize
  different conditions and handle the errors accordingly.

  The simplest approach for declaring error numbers is to provide an
  enumeration.  For example, here is the start of the buffer manager's
  enumeration of errors:

     enum bufErrCodes { HASHTBLERROR, HASHNOTFOUND, BUFFEREXCEEDED, ... };


ERROR MESSAGES:
  Corresponding to its set of error numbers, each subsystem also must declare
  an array of error messages, and must make these messages available to the
  global error object.  The index into the array must match the number of the
  corresponding error.  Here is an excerpt from the buffer manager's array of
  error messages, corresponding to the above enumeration:

     static const char* bufErrMsgs[] = {
         "hash table error",
         "hash entry not found",
         "buffer pool full",
      ... };

  Note that this array of strings is declared "static."  So how can the buffer
  manager make these strings available to the global error object?  By creating
  a static "error_string_table" object.  The constructor of this object
  registers the error messages with the system when the program first starts.
  Here is the buffer manager's error_string_table declaration, in the same file
  as the above array of strings:

     static error_string_table bufTable( BUFMGR, bufErrMsgs );


POSTING ERRORS:
  There are three macros defined that make it easy to add errors when you
  discover them.  These macros also add the name of the file and the line
  number where the error happens, as a debugging aid.

  To add a "first" error, use the MINIBASE_FIRST_ERROR macro.  For example, if
  the buffer manager detects that the buffer pool is too full to complete an
  operation, it posts its BUFFEREXCEEDED error like this:

     MINIBASE_FIRST_ERROR( BUFMGR, BUFFEREXCEEDED );

  To add a "chained" error, use the MINIBASE_CHAIN_ERROR macro.  For example,
  if the buffer manager calls on the database manager to write a page to disk,
  and that operation fails, the buffer manager adds an error that records the
  fact that the execution path that failed went through it:

     status = MINIBASE_DB->write_page( ... );
     if ( status != OK )
         return MINIBASE_CHAIN_ERROR( BUFMGR, status );

  Sometimes, you wish to post a different error message, but still acknowledge
  that the error resulted from a prior error.  This is a combination of the
  above situations.  For this, use the MINIBASE_RESULTING_ERROR macro:

     status = MINIBASE_DB->write_page( ... );
     if ( status != OK )
         return MINIBASE_RESULTING_ERROR( BUFMGR, status, BUFFEREXCEEDED );


HANDLING ERRORS:
  There are a number of ways to find out what has gone wrong in the system.
  The most primitive way is to print the global error object:

     minibase_errors.show_errors();

  If you need your program to detect whether an error has happened, you may ask
  the error object if it has accumulated any errors:

     if ( minibase_errors.error() )
         ...;

  You may find out the subsystem that posted the first error:

     if ( minibase_errors.originator() == BUFMGR )
         ...;

  You may find out the error number of the first error:

     if ( minibase_errors.error_index() == BUFFEREXCEEDED )
         ...;

  If you wish to examine the entire set of errors that have been posted, you
  may use the error() method shown above to get a pointer to the first error
  record in the list; you may then use methods of the error_node class to get
  details of the error, and to traverse the entire list of errors.

*/

using namespace std;

#include <iostream>
#include <fstream>
#include <assert.h>


  /* Here is the Status enumeration.  There are four groups of status codes:

     1. OK.  This is the "normal return" status.  It is in a class by itself.

     2. The "layer," or subsystem ID, status codes---for example, BUFMGR
     (meaning the buffer manager subsystem).  A subsystem notifies its caller
     of an error by adding an error to the global error list and returning its
     subsystem ID status code.  The caller may then inspect the error and
     decide how to handle it.

     3. DONE and FAIL.  DONE is a special code for non-errors that are
     nonetheless not "OK": it generally means "finished" or "not found."  FAIL
     is for errors that happen outside the bounds of a subsystem.

     4. The last category is a set of deprecated status codes that refer to
     specific problems.  These are being replaced by the subsystem+error index
     mechanism as time permits. */

enum Status { OK,

                // The subsystem ID codes.
              BUFMGR, RECOVERYMGR, LOGMGR, SHAREDMEMORYMGR, BTREE,
	      SORTEDPAGE, BTINDEXPAGE, BTLEAFPAGE,
	      LINEARHASH, GRIDFILE, RTREE, JOINS, PLANNER, PARSER,
	      OPTIMIZER, FRONTEND, CATALOG, DBMGR, RAWFILE, LOCKMGR,
	      XACTMGR, HEAPFILE, SCAN,


                // Other, legitimate, codes.
              DONE, FAIL,


                // Deprecated status codes that indicate specific errors.
	      TUPLE_TOO_BIG, TUPLE_TYPE,
              FILEEOF, RECNOTFOUND, RELNOTFOUND, INDEXNOTFOUND,
	      ATTRNOTFOUND, INSUFMEM, NOMORERECS,


                // This is not a status code; it is the number of status codes.
              NUM_STATUS_CODES };



  /* This is how you register the error messages for a subsystem.  You declare
     a static error_string_table object and pass the constructor the subsystem
     ID (from the Status enumeration, above), and your table of error messages
     for the subsystem.  For example, the buffer manager subsystem has a
     declaration like this:

       static error_string_table errtable( BUFMGR, bufErrMsgs );

     Here "bufErrMsgs" is previously declared as a static array of strings. */

class error_string_table
{
public:
    error_string_table( Status subsystem, const char* messages[] )
        { table[subsystem] = messages; }

    static const char* get_message( Status subsystem, int index );
      /* Returns the message for the given index and subsystem, or NULL. */

private:
    static const char** table[];
};



  /* Every error that is logged by a call to global_error::add_error creates an
     error node.  The Status types are converted into strings in team_name().
     A node contains either a from (type Status) variable or a message (char*). */

class error_node
{
public:
    error_node( Status subsys, Status prior = OK, int err_index = -1,
                const char* extra_msg = 0 );
    ~error_node();
    void set_next(error_node* nxt)      { next_node = nxt; }
	
    void show_error( ostream& to=cerr ) const;
    static const char* team_name(Status T1);

    const error_node* get_next() const  { return next_node; }
    Status get_status() const           { return subsystem; }
    int get_error_index() const         { return error_index; }
    Status get_prior_status() const     { return prior_status; }
    const char* get_message() const
        { return error_string_table::get_message(subsystem,error_index); }
    const char* get_extra_message() const { return msg; }

private:
    error_node* next_node;
    Status subsystem;           // The subsystem that added the error.
    Status prior_status;        // The status that prompted the error, or OK.
    char* msg;                  // An extra error message.
    int error_index;            // Index into subsystem's error messages, or -1.
};



class global_errors
{
public:
    global_errors();
    ~global_errors();

    Status add_error( Status subsystem, const char* msg )
        { return add_error( new error_node(subsystem,OK,-1,msg) ); }
      /* Discouraged: use MINIBASE_FIRST_ERROR() instead (and use a table of
         messages instead of literal strings in-line). */

    Status add_error( Status subsystem, Status priorStatus,
                      int lineno, const char *file, int error_index );

    Status add_error( Status subsystem, int lineno,
                      const char *file, int error_index )
        { return add_error( subsystem, OK, lineno, file, error_index ); };


    void clear_errors();
      /* If you have dealt with the errors, you may call clear_errors() to
         throw away all traces of them. */

    void show_errors( ostream& to );
    void show_errors();         // Displays to cerr (out of line for debugger)
      /* If you would like to display the current set of errors, call
         show_errors.  If there are no errors, nothing is displayed. */


    const error_node* error()
        { return first; }
      /* The error() method returns the original error, or NULL if there are no
         errors yet.  You may walk the chain of errors by calling the
         get_next() method. */


    Status status()
        { return last? last->get_status() : OK; }
      /* The status() method tells you what the "current" status of the system
         is.  If errors are present, returns the ID of the subsystem that added
         the most recent error.  If there are no errors, returns OK. */


    Status originator()
        { return first? first->get_status() : OK; }
      /* Returns the ID of the subsystem that posted the first error, or OK if
         none. */


    int error_index()
        { return first? first->get_error_index() : -1; }
      /* Returns the subsystem-specific error index of the original error, or
         -1 if either there are no errors or the original error was posted with
         a string instead of a number. */


private:
    Status add_error( error_node* next );
	
    error_node* first;
    error_node* last;
};

 // This is the global object that holds errors.
extern global_errors minibase_errors;

#define MINIBASE_FIRST_ERROR( SUBSYS, INDEX ) \
        ( minibase_errors.add_error(SUBSYS,OK,__LINE__,__FILE__,INDEX) )

#define MINIBASE_CHAIN_ERROR( SUBSYS, PRIOR ) \
        ( minibase_errors.add_error(SUBSYS,PRIOR,__LINE__,__FILE__,-1) )

#define MINIBASE_RESULTING_ERROR( SUBSYS, PRIOR, INDEX ) \
        ( minibase_errors.add_error(SUBSYS,PRIOR,__LINE__,__FILE__,INDEX) )

#define MINIBASE_SHOW_ERRORS() \
        ( minibase_errors.show_errors() )

#endif
