#ifndef VIXMNTUTILITY_H
#define VIXMNTUTILITY_H

#include <stdio.h>
#include <stdarg.h>
#include <sys/types.h>
#include <string.h>
#include <unistd.h>
#include <string>
#include <time.h>

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

void getnow(char* buffer);

#endif //VIXMNTUTILITY_H
