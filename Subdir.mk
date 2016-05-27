SUBDIR_FILES  =  vixMntMmap.cpp vixMntMsgOp.cpp vixMntMsgQue.cpp vixMntUtility.cpp vixMntOperation.cpp

INCLUDE += -I$(SRCROOT)/lib/distribute

INCLUDE += -I$(KROOTLIBFUSE)/include

INCLUDE +=  -I$(SRCROOT)/lib/fuseMountIPC

INCLUDE +=  -I$(SRCROOT)/lib/public

LIBS += -lrt

