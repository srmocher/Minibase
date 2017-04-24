
#include <stdlib.h>
#include <iostream>

#include "SMJTester.h"

int MINIBASE_RESTART_FLAG = 0;

int main()
{
   SMJTester smjt;
   Status dbstatus;

   dbstatus = smjt.runTests();

   if (dbstatus != OK) {       
      cout << "Error encountered during sort merge join tests: " << endl;
      minibase_errors.show_errors();      
      return(1);
   }
   
   return(0);
}

