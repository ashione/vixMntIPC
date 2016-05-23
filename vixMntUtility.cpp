#include <vixMntUtility.h>

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
    sprintf(buffer,
            buffLog,
            levelStr[level],pid,timebuf,fileName,func,line);
    vsprintf(buffer+strlen(buffer),format,args);
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
