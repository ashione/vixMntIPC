CFLAGS = -c -Wall -std=c++0x
CC = g++
EXEC = test

all : test

$(EXEC) : testMesgQue.cpp  vixMntMsgQue.o
	  $(CC) -o $(EXEC)  $? -lrt

vixMntMsgQue.o : vixMntMsgQue.h vixMntMsgQue.cpp
	$(CC) $(CFLAGS) $?

#.PTHON:
clean : 
	rm -rf test *.o *.gch
