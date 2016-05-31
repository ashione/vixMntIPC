CCSTD= -g -std=c++0x -DVIXIPCTEST -fPIC
CFLAGS = -c -Wall $(CCSTD)
CC = g++
#EXEC = test
INCLUDE=-I./include
SRC = src/*
#EXES = testop $(EXEC) testmmap testst testutil
TARGET = ./lib/libfuseipc.so
LDFLAGS = -shared

all : 
	$(CC) $(CCSTD) -o $(TARGET) $(SRC)  $(LDFLAGS) $(INCLUDE)
	@cp $(TARGET) ~/lib/

#all :  $(EXES)
#
#testmmap : testvixMntMmap.cpp vixMntMmap.o vixMntMsgQue.o vixMntMsgOp.o vixMntUtility.o
#	$(CC)  $(CCSTD) -o $@  $? $(INCLUDE) -lrt
#
#vixMntMmap.o : vixMntMmap.cpp vixMntMmap.h
#	$(CC) $(CFLAGS) $? $(INCLUDE)
#
#$(EXEC) : testMesgQue.cpp  vixMntMsgQue.o vixMntMsgOp.o vixMntUtility.o vixMntMmap.o
#	  $(CC) -o $(EXEC) $(CCSTD)  $? $(INCLUDE) -lrt
#
#vixMntMsgQue.o : vixMntMsgQue.cpp vixMntMsgQue.h
#	$(CC) $(CFLAGS) $? $(INCLUDE)
#
#testop : testVixMntMsgOp.cpp vixMntMsgOp.o
#	$(CC)  $(CCSTD) -o $@  $? $(INCLUDE)
#
#vixMntMsgOp.o : vixMntMsgOp.cpp  vixMntMsgOp.h
#	$(CC) $(CFLAGS) $? $(INCLUDE)
#
#testst : testMntOperation.cpp vixMntOperation.o vixMntMsgQue.o vixMntMsgOp.o vixMntUtility.o vixMntMmap.o
#	$(CC)  $(CCSTD) -o $@  $? $(INCLUDE) -lrt
#
#vixMntOperation.o : vixMntOperation.cpp vixMntOperation.h
#	$(CC) $(CFLAGS) $? $(INCLUDE)
#
#vixMntUtility.o : vixMntUtility.cpp vixMntUtility.h 
#	$(CC) $(CFLAGS) $? $(INCLUDE)
#
#testutil : testMntUtility.cpp vixMntUtility.o vixMntMmap.o vixMntMsgQue.o vixMntMsgOp.o
#	$(CC) $(CCSTD) -DVIXIPCTEST -o $@ $? $(INCLUDE) -lrt
#
#share : $(OBJECTS)
#	$(CC) $(CCSTD) -o $(TARGET)  $(OBJECTS) $(LDFLAGS)

#clean : 
#	rm -rf $(OBJECTS) $(EXES) *.o *.gch  core.* $(TARGET)
