CFLAGS = -g -Wall -fPIC -D_FILE_OFFSET_BITS=64
CXX = g++ 
CC = gcc

#FUSEROOT=/mts/home4/nzuo/workspace/download/libfuse
FUSEROOT=/bldmnt/toolchain/lin64/fuse-2.7.4
INCLUDE=-I./include  -I$(FUSEROOT)/include -I${VDDK_HOME}/include
SRC = $(wildcard src/*.cpp)

TARGET = ./lib/libfuseipc.so
LDFLAGS = -shared
LIBS = -L./lib -L$(FUSEROOT)/lib/.libs  -L${VDDK_HOME}/lib64 -lfuseipc -lpthread -lrt -lfuse -lvixDiskLib -lstdc++

BINDIR  = bin
OBJDIR = bin/obj
TESTDIR = src/test

TESTSRC_CPP = $(wildcard $(TESTDIR)/*.cpp)
TESTSRC_C = $(wildcard $(TESTDIR)/*.c)
TESTSRC = $(TESTSRC_CPP) $(TESTSRC_C)

TESTOBJ_CPP = $(patsubst $(TESTDIR)/%.cpp,$(OBJDIR)/%.o,$(filter %.cpp,$(TESTSRC)))
TESTOBJ_C = $(patsubst $(TESTDIR)/%.c,$(OBJDIR)/%.o,$(filter %.c,$(TESTSRC)))

TESTOBJ = $(TESTOBJ_CPP) $(TESTOBJ_C)

TESTBIN_CPP = $(patsubst $(TESTDIR)/%.cpp,$(BINDIR)/%.bin,$(filter %.cpp,$(TESTSRC)))
TESTBIN_C   = $(patsubst $(TESTDIR)/%.c,$(BINDIR)/%.bin,$(filter %.c,$(TESTSRC)))

TESTEXE = $(TESTBIN_CPP) $(TESTBIN_C)


all : $(TARGET) 
	
$(TARGET) : pre
	$(CXX) $(CFLAGS) -o $(TARGET) $(SRC)  $(LDFLAGS) $(INCLUDE)
	@cp $(TARGET) ~/lib/
	@echo "generate share library"

pre :
	test -d lib ||  mkdir lib 
	test -d bin || ( mkdir -p bin/obj)

test : $(TESTEXE)

$(BINDIR)/%.bin : $(OBJDIR)/%.o
	$(CXX) $(CFLAGS) -o $@ $<  $(INCLUDE) $(LIBS)

$(OBJDIR)/%.o : $(TESTDIR)/%.cpp
	$(CXX) $(CFLAGS) -o $@ -c $^ $(INCLUDE) $(LIBS)

$(OBJDIR)/%.o : $(TESTDIR)/%.c
	$(CC) $(CFLAGS) -o $@ -c $^ $(INCLUDE) $(LIBS)


.PHONY : clean vddk vddkclean

vddk :
	@sh vixlink

vddkclean :
	@find ./ -name \".cpp\" -type l | xargs -n1 -I{} unlink {}

cleantest : 
	rm -rf bin

clean :  cleantest
	rm -rf lib  $(TARGET) 

