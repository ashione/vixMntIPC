CCSTD= -g -std=c++0x
CFLAGS = -c -Wall $(CCSTD)
CC = g++
EXEC = test
INCLUDE= -I./ -I$(BORA_ROOT)/lib/public
LIBSPATH= -L${BORA_ROOT}/build/obj-x64/vddk/lib
LIBS = -l-fuseMountIPC-obj

#OBJECTS = testop $(EXEC) testmmap testst
OBJECTS = $(EXEC) 

all :  $(OBJECTS)

test : testMesgQue.cpp
	$(CC) $(CCSTD) -o $(EXEC)  $? $(INCLUDE) $(LIBSPATH) $(LIBS)

clean : 
	rm -rf $(OBJECTS) *.o *.gch  core.*
