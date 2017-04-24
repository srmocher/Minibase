#ifndef __SORT__
#define __SORT__

#include "minirel.h"
#include "new_error.h"
#include "scan.h"

#define    PAGESIZE    MINIBASE_PAGESIZE

class Sort
{
 public:
   //constructor
   Sort(
    char*    	inFile,        	// Name of unsorted heapfile.
    char*   	outFile,    	// Name of sorted heapfile.
    int         len_in,         // Number of fields in input records.
    AttrType    in[],        	// Array containing field types of input records.
                   		// i.e. index of in[] ranges from 0 to (len_in - 1)
    short       str_sizes[],  	// Array containing field sizes of input records.
    int         fld_no,     	// The number of the field to sort on.
                            	// fld_no ranges from 0 to (len_in - 1).
    TupleOrder  sort_order,   	// ASCENDING, DESCENDING
    int         amt_of_buf,   	// Number of buffer pages available for sorting.
    Status&     s		// status 
  );
  
  // Tuple comparision function
  // Implemented in sort.o
  int   tupleCmp (const void* t1, const void* t2);
   //destructor
  ~Sort(){}

 private: 
   //*************************************************************************/
   // The following private methods are internally invoked in the Sort constructor.
   //*************************************************************************
    
   //*************************************************************************
   //   _pass_one does the quicksorting into runs pass.  
   //	Returns the number of temporary files created in num_temp_file.
   //*************************************************************************
   Status _pass_one(int& numtempfile);
   
   //*********************************************************************************
   //    _merge_many_to_one: given source heapfiles, the number of heapfiles and a destination file,
   //		   this merges the source heapfiles into the destination heapfile. 
   //		   The main workhorse for the merging.
   //*********************************************************************************
   Status _merge_many_to_one(unsigned int numtempfile, HeapFile **source, HeapFile* dest);

   //*********************************************************************************
   //     This goes through one later pass, merging the files from the previous pass into
   // a smaller number of larger files.  It needs to know how many files were in the previous
   // pass and what the pass number is (for naming purposes).  It returns the number of files
   // it created in "numberDest"
   //      This just sets cycles through setting up the heapfiles and then calls 
   // _merge_many_to_one.
   //*********************************************************************************       
   Status _one_later_pass(int numberTempFiles, int passNum, int &numDest);

   //*********************************************************************************************
   //      _merge repeats calling _one_later_pass until only one file is left
   //*********************************************************************************************
   Status _merge(int numFiles);
   
   //*********************************************************************************
   //    _temp_name: given an output file, a pass number, and a file number within the pass, 
   //		     this creates the unique name for that file.
   //    File '7' in pass '3' for output file 'FOO' will be named 
   //		=> "FOO.sort.temp.3.7"
   //********************************************************************************* 
   char* _temp_name(int pass, int run, char* out_file);

   //*********************************************************************************
   // Member variables for storing the values of the input parameters of the constructor
   //*********************************************************************************
   int 		_rec_length;  // Record length.
   int 		_amt_of_buf;  // Number of buffer pages available for sorting.
   char* 	_in_file;     // Name of unsorted input heapfile.
   char* 	_out_file;    // Name of sorted output heapfile
   int 		_fld_no;      // The number of the field to sort on. _fld_no ranges from 0 to (len_in - 1).
   TupleOrder   _sort_order;  // ASCENDING, DESCENDING
   short* 	_str_sizes;   // Array containing field sizes of input records.
};


#endif
