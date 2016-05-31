CCSTD= -g -std=c++0x -DVIXIPCTEST -fPIC
CFLAGS = -c -Wall $(CCSTD)
CC = g++
#EXEC = test
INCLUDE=-I./include
SRC = $(wildcard src/*.cpp)
#EXES = testop $(EXEC) testmmap testst testutil
TARGET = ./lib/libfuseipc.so
LDFLAGS = -shared
LIBS = -lrt -L$(dir $(TARGET)) -lfuseipc

BINDIR = bin
TESTDIR= src/test
TESTSRC=$(wildcard $(TESTDIR)/*.cpp)
TESTOBJ=$(patsubst $(TESTDIR)/%.cpp,$(BINDIR)/%.bin,$(TESTSRC))


all : $(TARGET) test $(TESTOBJ)

$(TARGET) :
	test -d lib || mkdir lib
	$(CC) $(CCSTD) -o $(TARGET) $(SRC)  $(LDFLAGS) $(INCLUDE)
	@cp $(TARGET) ~/lib/
	@echo generate share library

ctest :
	test -d bin || mkdir bin

test : ctest $(TESTOBJ)

$(TESTOBJ): $(TESTSRC)
	$(CC) $(CCSTD) -o $@ $< $(INCLUDE) $(LIBS)



.PHONY : clean
clean : 
	rm -rf lib bin $(TARGET) *.o

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

