#include <vixMntUtility.h>

#ifndef VIXIPCTEST
extern "C" {
    #include "str.h"
#endif
/*
 * Init mmap instance
 */

/*
 * TODO:
 *  Multithread safe
 */

void
vixMntLog(short level,
        pid_t pid,
        int line,
        const char* func,
        const char* fileName,
        const char* format,
        ...)
{
    const char* levelStr[] = {"I","W","E","F"};
    char buffLog[] = "[%s%05d %s %s %s:%d] ";
    va_list args;
    va_start(args,format);
    char buffer[0x100];
    char timebuf[80];

    getnow(timebuf);

#ifndef VIXIPCTEST
    Str_Sprintf(buffer,
            0x100,
            buffLog,
            levelStr[level],pid,timebuf,fileName,func,line);
    Str_Vsnprintf(buffer,strlen(buffer),format,args);
#else
    sprintf(buffer,
            buffLog,
            levelStr[level],pid,timebuf,fileName,func,line);
    vsprintf(buffer+strlen(buffer),format,args);
#endif

    va_end(args);

    printf("%s\n",buffer);
}

void getnow(char* buffer){

    time_t rawtime;
    struct tm* timeinfo;
    time(&rawtime);
    timeinfo = localtime(&rawtime);
    strftime(buffer,80,"%X|%F",timeinfo);

}

int vixMntIPC_InitMmap(
        size_t mmap_datasize,
        int isRoot)
{
    if (!mmap_instance)
        mmap_instance = new VixMntMmap(mmap_datasize,isRoot!=0);
    else
        WLog("map instance already init");

    return 0;
}

int vixMntIPC_CleanMmap(){

    if(mmap_instance)
        delete mmap_instance;
        mmap_instance = NULL;
        ILog("unmap instance ok");
        return 0;

    WLog("no instance set up");
    return -1;
}


void
vixMntIPC_WriteMmap(
        const char* buf,
        size_t write_pos ,
        size_t write_size )
{
    if(!mmap_instance){
        FLog("mmap instance never init");
        return;
    }
    mmap_instance->mntWriteMmap(buf,write_pos,write_size);
}
void
vixMntIPC_ReadMmap(
        char* buf,
        size_t read_pos,
        size_t read_size)
{
    if(!mmap_instance) {
        FLog("mmap instance never init");
        return;
    }

    mmap_instance->mntReadMmap(buf,read_pos,read_size);
}


#ifndef VIXIPCTEST
}
#endif
