# CS560 Makefile
#
# Makefile for CS560 Minibase project.  Needs GNU make.
#
# Warning: make depend overwrites this file.

.PHONY: depend clean backup setup

MAIN=heaptest

MINIBASE=..

CC=g++

CFLAGS= -DUNIX -Wall -g

INCLUDES = -I${MINIBASE}/include -I.

# LFLAGS= -L${MINIBASE}/lib -liberty -lheapfile -lm
LFLAGS= -L${MINIBASE}/lib -L${MINIBASE}/lib32 -lbm -ldb -lm

SRCS = main.C heapfile.C heap_driver.C test_driver.C \
	 	new_error.C page.C system_defs.C \
		scan.C hfpage.C

OBJS = $(SRCS:.C=.o)

$(MAIN):  $(OBJS)
	 $(CC) $(CFLAGS) $(INCLUDES) $(OBJS) -o $(MAIN) $(LFLAGS)

.C.o:
	$(CC) $(CFLAGS) $(INCLUDES) $(LFLAGS) -c $<

depend: $(SRCS)
	makedepend $(INCLUDES) $^

clean:
	rm -f *.o *~ $(MAIN) $(MAKECLEANGARBAGE) 

backup:
	mkdir bak
	cp Makefile *.[Ch] bak

# Grab the sources for a user who has only the makefile
setup:
	/bin/cp -i $(MINIBASE)/src/*.[Ch] .
	-/bin/cp -i $(MINIBASE)/src/README .

# DO NOT DELETE THIS LINE -- make depend needs it 
