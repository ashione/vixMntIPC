CCSTD= -g -std=c++0x -fPIC
CFLAGS = -c -Wall $(CCSTD)
CC = g++

INCLUDE=-I./include
SRC = $(wildcard src/*.cpp)

TARGET = ./lib/libfuseipc.so
LDFLAGS = -shared
LIBS = -L./lib -lfuseipc -lpthread -lrt

BINDIR = bin
TESTDIR= src/test
TESTSRC=$(wildcard $(TESTDIR)/*.cpp)
TESTOBJ=$(patsubst $(TESTDIR)/%.cpp,$(BINDIR)/%.bin,$(TESTSRC))


all : $(TARGET) 

$(TARGET) :
	test -d lib || mkdir lib
	$(CC) $(CCSTD) -o $(TARGET) $(SRC)  $(LDFLAGS) $(INCLUDE)
	@cp $(TARGET) ~/lib/
	@echo generate share library

ctest :
	test -d bin || mkdir bin

test : ctest $(TESTOBJ)

$(TESTOBJ): 
	$(CC) $(CCSTD) -o $@ $(patsubst $(BINDIR)/%.bin,$(TESTDIR)/%.cpp,$@) $(INCLUDE) $(LIBS)

.PHONY : clean

cleantest : 
	rm -rf bin

clean :  cleantest
	rm -rf lib  $(TARGET) *.o


