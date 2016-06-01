CCSTD= -g -std=c++0x -DVIXIPCTEST -fPIC
CFLAGS = -c -Wall $(CCSTD)
CC = g++

INCLUDE=-I./include
SRC = $(wildcard src/*.cpp)

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

$(TESTOBJ): 
	$(CC) $(CCSTD) -o $@ $(patsubst $(BINDIR)/%.bin,$(TESTDIR)/%.cpp,$@) $(INCLUDE) $(LIBS)


.PHONY : clean
clean : 
	rm -rf lib bin $(TARGET) *.o


