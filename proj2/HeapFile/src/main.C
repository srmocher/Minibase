#include <stdlib.h>
#include <iostream>

#include "heap_driver.h"

int MINIBASE_RESTART_FLAG = 0;

int main(int argc, char **argv)
{
   HeapDriver hd;
   Status dbstatus;

   dbstatus = hd.runTests();

   // Check if the create database has succeeded
   if (dbstatus != OK) {       
      cout << "Errors encountered during heapfile tests: " << endl;
      minibase_errors.show_errors();
      return(1);
   }
   
   return(0);
}


