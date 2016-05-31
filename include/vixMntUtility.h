#ifndef VIXMNTUTILITY_H
#define VIXMNTUTILITY_H


#include <stdio.h>
#include <stdarg.h>
#include <sys/types.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <sys/types.h>


//static VixMntMmap *mmap_instance = NULL;

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
        int isRoot);

int
vixMntIPC_CleanMmap();

void
vixMntIPC_WriteMmap(
        const char* buf,
        size_t write_pos,
        size_t write_size);
void
vixMntIPC_ReadMmap(
        char* buf,
        size_t read_pos,
        size_t read_size);

#ifdef __cplusplus
}
#endif

#endif //VIXMNTUTILITY_H
