
#include <stdlib.h>
#include <iostream>

#include "btree_driver.h"

int MINIBASE_RESTART_FLAG = 0;

int main()
{
   BTreeTest btt;
   Status dbstatus;

   dbstatus = btt.runTests();

   if (dbstatus != OK) {       
      cout << "Error encountered during btree tests: " << endl;
      minibase_errors.show_errors();      
      return(1);
   }
   
   return(0);
}


// Get a simple integer choice, return 1 for valid, 0 if invalid.
int get_choice(istream& in, int& choice)
{
#define BUFSZE 80
    char *bf, buff[BUFSZE+1];

    char c;
    int tot = 0;

    bf = buff;

    do {

        in.get(c);

        if (in.eof())
            break;

        *bf++ = c;
        tot++;

    } while ((tot < BUFSZE) && (c != '\n'));

    *bf = '\0';

    choice = atoi(buff);

    return 1;
#undef BUFSZE
}
