#include <vixMntUtility.h>
#include <vixMntMsgQue.h>
#include <vixMntMmap.h>
#include <vixMntMsgOp.h>

#include <stdio.h>
#include <string.h>
#include <stdarg.h>
#include <sys/stat.h>
#include <errno.h>
#include <time.h>
#include <sys/time.h>

const char* random_str =
    "0123456789abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ";

static VixMntMmap *mmap_instance = NULL;
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
    char timebuf[80]="NOW";

    getnow(timebuf);

    snprintf(buffer,
            0x100,
            buffLog,
            levelStr[level],pid,timebuf,fileName,func,line);
    vsnprintf(buffer+strlen(buffer),80,format,args);

    va_end(args);

    printf("%s\n",buffer);
}

void getnow(char* buffer){

    time_t rawtime;
    struct tm* timeinfo;
    time(&rawtime);
    timeinfo = localtime(&rawtime);
    strftime(buffer,40,"%F %H:%M:%S",timeinfo);
    struct timeval tp;
    gettimeofday(&tp, NULL);
    float us = tp.tv_usec;
    snprintf(buffer+strlen(buffer),10,":%06.f",us);

}

int vixMntIPC_InitMmap(
        size_t mmap_datasize ,
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

void*
vixMntIPC_run(void* arg)
{
    VixMntMsgQue* vixmntmsg = VixMntMsgQue::getMsgQueInstance();
    pthread_t run_tid = pthread_self();
    while(true){

        VixMntMsgOp msg_op;
        vixmntmsg->receiveMsgOp(&msg_op);

#if defined(__cplusplus) && __cplusplus >= 201103L
        if(msg_op == VixMntMsgOp::ERROR){
#else
        if(msg_op == ERROR){
#endif
            ILog("thread ID %u, receive error, breaking",run_tid);
        }
#if defined(__cplusplus) && __cplusplus >= 201103L
        else if(msg_op == VixMntMsgOp::HALT){
#else
        else if(msg_op == HALT) {
#endif

            ILog("thread ID %u, stop listening, breaking",run_tid);
            break;

        }
        else
            ILog("thread ID %u, receive %s",run_tid,getOpValue(msg_op));

    }
    // instance is not belong to this thread
    //VixMntMsgQue::releaseMsgQueInstance();

    return NULL;
}

pthread_t
listening(){

    pthread_t pt_id;
    int err = pthread_create(&pt_id,NULL,vixMntIPC_run,NULL);

    if(err){
      ELog("can't create thread");
      return 0;
    }

    ILog("thread running, %u",pt_id);
    return pt_id;
}

int
isDirectoryExist(const char* path){

   struct stat info;

   if( stat(path,&info) )
       return false;

   return ( info.st_mode & S_IFDIR ) !=0;

}

/*
 * check path was created.
 * If it's not, create whole path by hierarchy
 */

int makeDirectoryHierarchy( const char *path ){

     mode_t mode = 0666;
     std::string spath(path);

     int status = mkdir(spath.c_str(),mode);

     if (!status )
         return true;

     switch ( errno ){
         case ENOENT :
           {
               size_t pos = spath.find_last_of('/');
               if(pos == std::string::npos)
                   pos = spath.find_last_of('\\');

               if(pos == std::string::npos)
                    return false;

               if(!makeDirectoryHierarchy(spath.substr(0,pos).c_str()))
                   return false;

               int status = mkdir(spath.c_str(),mode);
               return !status;

           }
        case EEXIST :
           return isDirectoryExist(path);

        default :
           return false;
     }


}

const char*
getRandomFileName(const char* rootPath,size_t max_random_len){
    srand((unsigned) time(NULL));

    std::string rfile_name = rootPath;
    for(size_t i = 0 ; i < max_random_len - 1 ; ++i){
        rfile_name+= random_str[rand()%STR_RANDOM_NUM_LEN];
    }
    rfile_name += '\0';
    return rfile_name.c_str();
}
