
#include <stdlib.h>
#include <iostream>

#include "BMTester.h"

int MINIBASE_RESTART_FLAG = 0;

int main(int argc, char **argv)
{
   BMTester bmt;
   Status dbstatus;

   dbstatus = bmt.runTests();

   if (dbstatus != OK) {       
      cout << "Error encountered during buffer manager tests: " << endl;
      minibase_errors.show_errors();      
      return(1);
   }
   
   return(0);
}

