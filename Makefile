CFLAGS = -c -g -Wall fPIC 
CXX = g++ -std=c++0x
CC = gcc

INCLUDE=-I./include
SRC = $(wildcard src/*.cpp)

TARGET = ./lib/libfuseipc.so
LDFLAGS = -shared
LIBS = -L./lib -lfuseipc -lpthread -lrt

BINDIR  = bin
TESTDIR = src/test
TESTSRC = $(wildcard $(TESTDIR)/*.cpp)
TESTSRC += $(wildcard $(TESTDIR)/*.c)

TESTBIN_CPP = $(patsubst $(TESTDIR)/%.cpp,$(BINDIR)/%.bin,$(filter %.cpp,$(TESTSRC)))
TESTBIN_C   = $(patsubst $(TESTDIR)/%.c,$(BINDIR)/%.bin,$(filter %.c,$(TESTSRC)))


all : $(TARGET) 

$(TARGET) :
	test -d lib || ( mkdir lib && sh vixlink )
	$(CXX) $(CCSTD) -o $(TARGET) $(SRC)  $(LDFLAGS) $(INCLUDE)
	@cp $(TARGET) ~/lib/
	@echo generate share library

ctest :
	test -d bin || mkdir bin

test : ctest $(TESTBIN_CPP) $(TESTBIN_C)

$(TESTBIN_CPP) : 
	$(CXX) $(CCSTD) -o $@ $(patsubst $(BINDIR)/%.bin,$(TESTDIR)/%.cpp,$@)  $(INCLUDE) $(LIBS)

$(TESTBIN_C) :
	$(CC) $(CCSTD) -o $@ $(patsubst $(BINDIR)/%.bin,$(TESTDIR)/%.c,$@)  $(INCLUDE) $(LIBS)

.PHONY : clean

cleantest : 
	rm -rf bin

clean :  cleantest
	rm -rf lib  $(TARGET) *.o

