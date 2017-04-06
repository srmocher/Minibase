#
# Makefile for CS560 Minibase project.  Needs GNU make.
#
# Warning: make depend overwrites this file.

.PHONY: depend clean backup setup

MAIN=btree

MINIBASE = ..

CC=g++

CFLAGS= -DUNIX -Wall -g

INCLUDES = -I${MINIBASE}/include

LFLAGS= -L${MINIBASE}/lib -ldb -lm
 
# you need to change this 

SRCS =  main.C btree_driver.C btfile.C btindex_page.C \
	btleaf_page.C buf.C new_error.C key.C \
	btreefilescan.C system_defs.C page.C sorted_page.C hfpage.C

OBJS = $(SRCS:.C=.o)

$(MAIN):  $(OBJS)
	 $(CC) $(CFLAGS) $(INCLUDES) $(OBJS) -o $(MAIN) $(LFLAGS)

.C.o:
	$(CC) $(CFLAGS) $(INCLUDES) -c $<

depend: $(SRCS)
	makedepend $(INCLUDES) $^

clean:
	rm -f *.o *~ $(MAIN)

backup:
	-mkdir bak
	cp Makefile *.[Ch] bak

# Grab the sources for a user who has only the makefile
setup:
	/bin/cp -i $(MINIBASE)/src/*.[Ch] .

# DO NOT DELETE THIS LINE -- make depend needs it 
