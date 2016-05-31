CCSTD= -g -std=c++0x -DVIXIPCTEST -fPIC
CFLAGS = -c -Wall $(CCSTD)
CC = g++
EXEC = test
INCLUDE=-I../include
LIBS = -L../lib -lrt -lfuseipc
EXES = testop $(EXEC) testmmap testst testutil

all :  $(EXES)

testmmap : testvixMntMmap.cpp 
	$(CC)  $(CCSTD) -o $@  $? $(INCLUDE) $(LIBS)


$(EXEC) : testMesgQue.cpp  
	  $(CC) -o $(EXEC) $(CCSTD)  $? $(INCLUDE) $(LIBS)

testop : testVixMntMsgOp.cpp 
	$(CC)  $(CCSTD) -o $@  $? $(INCLUDE) $(LIBS)


testst : testMntOperation.cpp 
	$(CC)  $(CCSTD) -o $@  $? $(INCLUDE) $(LIBS)

testutil : testMntUtility.cpp 
	$(CC) $(CCSTD) -DVIXIPCTEST -o $@ $? $(INCLUDE) $(LIBS)


clean : 
	rm -rf $(OBJECTS) $(EXES) *.o *.gch  core.* $(TARGET)
