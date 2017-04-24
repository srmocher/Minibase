#include "new_error.h"
#include "string.h"
#include "stdio.h"
#include "stdlib.h"


global_errors minibase_errors;
const char** error_string_table::table[NUM_STATUS_CODES];

const char* error_string_table::get_message( Status subsystem, int index )
{
    const char** messages = table[subsystem];
    if ( messages != 0 && index >= 0 )
        return messages[index];
    else
        return 0;
}


error_node::error_node( Status subsys, Status prior, int err_index,
                        const char* extra_msg )
    : next_node(0),
      subsystem(subsys),
      prior_status(prior),
      msg( extra_msg? strcpy(new char[strlen(extra_msg)+1],extra_msg)
                    : (char*) NULL),
      error_index(err_index)
{
}


error_node::~error_node()
{
    //free(msg);
    delete[] msg;
}


void error_node::show_error( ostream& to ) const
{
    if ( prior_status == OK )
        to << team_name(subsystem);
    else 
      {
        to << "--> " << team_name(subsystem);
        to << "[from the " << team_name(prior_status) << "]";
      }

    const char* index_msg = get_message();
    if ( index_msg )
        to << ": " << index_msg;
    if ( msg )
        to << ": " << msg;
    to << endl;
}

const char* error_node::team_name(Status T1)
{
  switch (T1) {

  case BUFMGR:
    return "Buffer Manager";
    
  case BTREE:
    return "BTree";
    
  case SORTEDPAGE:
    return "Sorted Page";

  case BTINDEXPAGE:
    return "BTree Index Page";

  case BTLEAFPAGE:
    return "BTree Leaf Page";

  case JOINS:
    return "Joins";
    
  case PLANNER:
    return "Planner";
    
  case PARSER:
    return "Parser";
    
  case OPTIMIZER:
    return "Optimizer";
    
  case FRONTEND: 
    return "Front End";
    
  case CATALOG:
    return "Catalog";

  case HEAPFILE:
    return "Heap File";
    
  case DBMGR:
    return "DB Manager";
   
  default:
    return "<<Unknown>>";
  }

  return NULL;
}

/*************************************
  GLOBAL_ERRORS::GLOBAL_ERRORS
*************************************/
global_errors::global_errors()
{  
  first = NULL;
  last = NULL;
}


Status global_errors::add_error( error_node* next )
{
    if (last)
        last->set_next(next);
    else
        first = next;
    last = next;
    return next->get_status();
}


Status global_errors::add_error( Status subsystem, Status priorStatus,
                                 int lineno, const char *file, int error_index )
{
    char extra[strlen(file) + 10];
    sprintf( extra, "%s:%d", file, lineno );
    return add_error( new error_node(subsystem,priorStatus,error_index,extra) );
}


void global_errors::show_errors( ostream& to )
{
    if ( first )
        to << "First error occurred: ";
    for ( const error_node* err = first; err; err = err->get_next() )
        err->show_error(to);
}

void global_errors::show_errors()
{
    show_errors( cerr );
}

void global_errors::clear_errors()
{
    for ( error_node* err = first; err; )
      {
        error_node* prev = err;
        err = (error_node*)err->get_next();
        delete prev;
      }

    first = last = NULL;
}



global_errors::~global_errors()
{
    clear_errors();
}
