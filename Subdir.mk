SUBDIR_FILES += $(addprefix ,vixMntMsgOp.cpp vixMntOperation.cpp \
				vixMntMmap.cpp vixMntMsgQue.cpp vixMntUtility.cpp \
				vixMntDisk.cpp vixMntFuse.cpp vixMntSocket.cpp)
##SUBDIR_FILES += $(wildcard *.cpp )
#$(info '*************************')
#$(info $(SUBDIR_FILES))
#$(info '*************************')
INCLUDE += -I$(SRCROOT)/lib/distribute

INCLUDE += -I$(KROOTLIBFUSE)/include

INCLUDE +=  -I$(SRCROOT)/lib/fuseMountIPC/include

INCLUDE +=  -I$(SRCROOT)/lib/public

INCLUDE +=  -I$(SRCROOT)/public

LIBS += -lrt

