SHELL = /bin/bash
# Uncomment for debugging
#~ FLAGS = -Wall -O3 -std=c++11 -std=gnu++11 -fpic
#FLAGS = -Wall -O3 -std=c++11 -std=gnu++11
LDFLAGS = 
CC = gcc 
STRIP = strip
FLAGS = -O3 -Wall 
LDFLAGS = 

EDIT = vi
ARC = ../wdsat_solver

SRC_SAT = 
SRC_GEN = main.c wdsat_utils.c dimacs.c cnf.c xorset.c xorgauss.c wdsat.c

OBJ_SAT = $(SRC_SAT:.c=.o)
LIB_SAT = $(SRC_SAT:.c=.h)
OBJ_GEN = $(SRC_GEN:.c=.o)
LIB_GEN = $(SRC_GEN:.c=.h)

### --- All Binaries

all: ../wdsat_solver clean

../wdsat_solver: $(OBJ_GEN)
	$(CC) $^ -o $@ $(LDFLAGS) -lm

%.o: %.c %.h
	$(CC) $(FLAGS) -c $< 

### --- OTHERS

clean:
	rm -f *.o *~ .*~ > /dev/null
	
arc:
	echo "VERSION -- `date +'%y%m%d%H%M'`" > README
	vi ./README
	cd .. ; \
	mkdir $(ARC)--`date +'%y%m%d%H%M'` ; \
	cp $(ARC)/*.cpp $(ARC)/*.hpp $(ARC)/*.py $(ARC)/makefile $(ARC)/README $(ARC)/INFO $(ARC)/BEGINNING ./$(ARC)--`date +'%y%m%d%H%M'` > /dev/null 2>&1 ; \
	tar cvzf ./$(ARC)--`date +'%y%m%d%H%M'.tar.gz` $(ARC)--`date +'%y%m%d%H%M'` ; \
	rm -rf ./$(ARC)--`date +'%y%m%d%H%M'` ; \
	cd $(ARC)
	
staticlib: $(OBJ_SAT)
	ar rcs cryptonid.a $(OBJ_SAT)

#~ sharedlib: $(OBJ_SAT)
	#~ $(CC) $^ -o $@.o $(LDFLAGS) -lm
	#~ g++ -shared -o $@.so $@.o

doxygen: $(ARC).doxygen
	doxygen $(ARC).doxygen > /dev/null
