# ~gid/CS564/Makefile
#
# Makefile for CS564 Minibase project.  Needs GNU make.
#
# Define DEBUGREL for some kind of debugging output (not from us, from
# the original Minibase implementors.)
#
# Warning: make depend overwrites this file.

.PHONY: depend clean backup setup

MAIN=SortMerge

MINIBASE = ..

CC=g++

CFLAGS= -DUNIX -Wall -Wno-write-strings -g

INCLUDES = -I${MINIBASE}/include -I.

LFLAGS= -L$(MINIBASE)/lib \
        -ldb -lm

SRCS =test_driver.C SMJTester.C main.C sortMerge.C scan.C \
	hfpage.C new_error.C system_defs.C heapfile.C\
	page.C buf.C

OBJS = $(SRCS:.C=.o)

$(MAIN):  $(OBJS)
	 $(CC) $(CFLAGS) $(INCLUDES) $(OBJS) sort.o -o $(MAIN) $(LFLAGS)

# Not really "all", but this is useful for setting up the libraries.
all: $(OBJS)

.C.o:
	$(CC) $(CFLAGS) $(INCLUDES) -c $<

depend: $(SRCS)
	makedepend $(INCLUDES) $^

clean:
	mv sort.o sort.a 
	rm -f *.o *~ $(MAIN)
	mv sort.a sort.o

backup:
	mkdir bak
	cp Makefile *.[Ch] bak

# Grab the sources for a user who has only the makefile
setup:
	/bin/cp -i $(MINIBASE)/src/*.[Ch] .
	-/bin/cp -i $(MINIBASE)/src/README .

# DO NOT DELETE THIS LINE -- make depend needs it 
