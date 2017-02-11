# Makefile for the Minibase project.  Needs GNU make.
#
# Warning: make depend overwrites this file.

.PHONY: depend clean backup setup

MAIN=hfpage

MINIBASE=..

CC=g++

CFLAGS= -DUNIX -Wall -g 

INCLUDES = -I${MINIBASE}/include -I.

#LFLAGS= -L${MINIBASE}/lib -liberty -lm
#LFLAGS= -L${MINIBASE}/lib32 -lbm -ldb -lm
LFLAGS= -L${MINIBASE}/lib32 -L${MINIBASE}/lib -lbm -ldb -lm

SRCS = main.C hfpage.C hfp_driver.C test_driver.C \
		new_error.C page.C system_defs.C

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
