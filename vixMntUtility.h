#ifndef VIXMNTUTILITY_H
#define VIXMNTUTILITY_H


#include <stdio.h>
#include <stdarg.h>
#include <sys/types.h>
#include <string.h>
#include <unistd.h>
#include <time.h>
#include <vixMntMsgQue.h>
#include <vixMntMmap.h>


static VixMntMmap *mmap_instance;

#ifdef __cplusplus
extern "C" {
#endif

#define ILog(format,...) vixMntLog(0,getpid(),__LINE__,__func__,__FILE__,format,##__VA_ARGS__)

#define WLog(format,...) vixMntLog(1,getpid(),__LINE__,__func__,__FILE__,format,##__VA_ARGS__)

#define ELog(format,...) vixMntLog(2,getpid(),__LINE__,__func__,__FILE__,format,##__VA_ARGS__)

#define FLog(format,...) vixMntLog(3,getpid(),__LINE__,__func__,__FILE__,format,##__VA_ARGS__)



void
vixMntLog(short level,
        pid_t pid,
        int line,
        const char* func,
        const char* fileName,
        const char* format,
        ...);

void
getnow(char* buffer);


int
vixMntIPC_InitMmap(
        size_t mmap_datasize,
        int isRoot=0);

int
vixMntIPC_CleanMmap();

void
vixMntIPC_WriteMmap(
        const char* buf,
        size_t write_pos =0,
        size_t write_size = 0);
void
vixMntIPC_ReadMmap(
        char* buf,
        size_t read_pos = 0,
        size_t read_size = 0);

#ifdef __cplusplus
}
#endif

#endif //VIXMNTUTILITY_H
