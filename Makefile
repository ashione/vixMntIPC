CCSTD= -std=c++0x
CFLAGS = -c -Wall $(CCSTD)
CC = g++
EXEC = test
INCLUDE=-I./
OBJECTS = testop $(EXEC) testmmap

all :  $(OBJECTS)

testmmap : testvixMntMmap.cpp vixMntMmap.o vixMntMsgQue.o vixMntMsgOp.o
	$(CC)  $(CCSTD) -g -o $@  $? $(INCLUDE) -lrt

vixMntMmap.o : vixMntMmap.cpp vixMntMmap.h
	$(CC) $(CFLAGS) $? $(INCLUDE)

$(EXEC) : testMesgQue.cpp  vixMntMsgQue.o vixMntMsgOp.o
	  $(CC) -o $(EXEC) $(CCSTD)  $? -lrt $(INCLUDE)

vixMntMsgQue.o : vixMntMsgOp.o vixMntMsgQue.cpp vixMntMsgQue.h
	$(CC) $(CFLAGS) $? $(INCLUDE)

testop : testVixMntMsgOp.cpp vixMntMsgOp.o
	$(CC)  $(CCSTD) -o $@  $? $(INCLUDE)

vixMntMsgOp.o : vixMntMsgOp.cpp  vixMntMsgOp.h
	$(CC) $(CFLAGS) $? $(INCLUDE)

clean : 
	rm -rf $(OBJECTS) *.o *.gch
	rm -rf /tmp/file
