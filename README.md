# Minibase
CSC560

A series of projects focused on building parts of a relational database system - Minibase fully in C++.

**Project 1** - Page abstraction and HeapFile CRUD operations, along with sequential scan. Supports variable length records using slot-directory structure.

**Project 2** - BufferManager to manage main memory frames for pages and caching pages.

**Project 3** - B+ Tree index for string/integer keys. Insertion, deletion, full scan, range queries supported.

**Project 4** - Joining two HeapFiles using Sort-Merge Join.

**Build Instructions**
Similar for each project
````
mkdir build
cd build
cmake ..
make

