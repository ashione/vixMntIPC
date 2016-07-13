#include <vixMntUtility.h>
#include <vixMntMsgQue.h>
#include <vixMntMmap.h>
#include <vixMntMsgOp.h>
#include <vixMntDisk.h>
#include <vixMntSocket.h>

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
static VixMntMsgQue *msgQ_instance = NULL;
static VixMntDiskHandle *diskHandle_instance = NULL;
static uint8 IPCTYPE_FLAG = 0;

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
    mmap_instance->mntWriteMmap((uint8*)buf,write_pos,write_size);
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

    mmap_instance->mntReadMmap((uint8*)buf,read_pos,read_size);
}

void
vixMntIPC_InitMsgQue() {

    if(!msgQ_instance){
        msgQ_instance = VixMntMsgQue::getMsgQueInstance();
        ILog("msgQ_instance init ok");
    }
    else
        ILog("msgQ_instance is already inited");
}

void
vixMntIPC_CleanMsgQue(){

    if(msgQ_instance){
        VixMntMsgQue::releaseMsgQueInstance();
        ILog("release msgQ_instance");
        msgQ_instance = NULL;
    }
}

void
vixMntIPC_InitDiskHandle(
        VixDiskLibConnection connection,
        const char* path,
        uint32 flag,
        uint8 IPCType)
{
    IPCTYPE_FLAG = IPCType;
    diskHandle_instance = new VixMntDiskHandle(connection,path,flag);

    if( IPCTYPE_FLAG == VIXMNTIPC_MMAP ){
        vixMntIPC_InitMmap(MMAP_MEMORY_SIZE,0);
        vixMntIPC_InitMsgQue();
        diskHandle_instance->prepare(msgQ_instance,mmap_instance);
    }
}

void
vixMntIPC_CleanDiskHandle(){
    ILog("Clean DiskHandle");
    delete diskHandle_instance;

    if(IPCTYPE_FLAG == VIXMNTIPC_MMAP){
        vixMntIPC_CleanMmap();
        msgQ_instance->unlink();
        vixMntIPC_CleanMsgQue();
    }
}

VixError
vixMntIPC_GetDiskInfo(VixDiskLibInfo **info){
    ILog("get vixdisklibinfo ");
   return diskHandle_instance->getDiskInfo(info);
};

void
vixMntIPC_FreeDiskInfo(VixDiskLibInfo *info){
    ILog("free vixdisklibinfo");
    diskHandle_instance->freeDiskInfo(info);
}


void*
vixMntIPC_run(void* arg)
{
    VixMntMsgQue* vixmntmsg = VixMntMsgQue::getMsgQueInstance();
    //VixDiskLibHandle vixHandle = (VixDiskLibHandle) arg;

    //assert(vixHandle);
    ILog("tranfer arg to vixHandle");

    pthread_t run_tid = pthread_self();
    while(true){

        VixMntMsgOp msg_op;
        //VixMntMsgData msg_data;
        //vixmntmsg->receiveMsg(&msg_data);
        //msg_op = msg_data.msg_op;

        vixmntmsg->receiveMsgOp(&msg_op);

        if(msg_op == VixMntOp(ERROR)){
            ILog("thread ID %u, receive error, breaking",run_tid);
        }
        else if(msg_op == VixMntOp(HALT)) {

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

void*
vixMntIPC_listen(void* args){
    if (IPCTYPE_FLAG == VIXMNTIPC_MMAP)
        return diskHandle_instance->listen(args);
    else{
        VixMntSocketServer* socketServer_instance = new VixMntSocketServer();
        socketServer_instance->serverListen(diskHandle_instance);
        return NULL;

    }
}

int
vixMntIPC_main(){

    pthread_t pt_id;
    //int err = pthread_create(&pt_id,NULL,vixMntIPC_run,(void *)vixHandle);
    int err = pthread_create(&pt_id,NULL,vixMntIPC_listen,NULL);

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

void
getRandomFileName(const char* rootPath,size_t max_random_len,char *destination){
    srand((unsigned) time(NULL));

    std::string rfile_name = rootPath;
    for(size_t i = 0 ; i < max_random_len - 1 ; ++i){
        rfile_name+= random_str[rand()%STR_RANDOM_NUM_LEN];
    }
    //rfile_name += '\0';
    //char *ptr = new char[rfile_name.size() +1 ];
    strncpy(destination,rfile_name.c_str(),rfile_name.size());
 //   return rfile_name.c_str();
}

uint8
getVixMntIPCType(){
    return IPCTYPE_FLAG;
}
