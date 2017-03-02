#ifndef PAGE_H
#define PAGE_H

#include "minirel.h"

const PageId INVALID_PAGE = -1;
const int MAX_SPACE = MINIBASE_PAGESIZE;

class Page
{
public:
    Page();
    ~Page();


private:
    char data[MAX_SPACE];
};

#endif
