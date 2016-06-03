SUBDIR_FILES += src/vixMntMsgOp.cpp src/vixMntOperation.cpp src/vixMntMmap.cpp src/vixMntMsgQue.cpp src/vixMntUtility.cpp

INCLUDE += -I$(SRCROOT)/lib/distribute

INCLUDE += -I$(KROOTLIBFUSE)/include

INCLUDE +=  -I$(SRCROOT)/lib/fuseMountIPC/include

INCLUDE +=  -I$(SRCROOT)/lib/public

INCLUDE +=  -I$(SRCROOT)/public

LIBS += -lrt
