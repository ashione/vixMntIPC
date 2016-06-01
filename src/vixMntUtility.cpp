#include <vixMntUtility.h>
#include <vixMntMsgQue.h>
#include <vixMntMmap.h>
#include <vixMntMsgOp.h>

#include <pthread.h>


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
    strftime(buffer,80,"%X|%F",timeinfo);

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

    while(true){

        VixMntMsgOp msg_op;
        vixmntmsg->receiveMsgOp(&msg_op);

        if(msg_op == VixMntMsgOp::ERROR){
            ILog("receive error, breaking");
        }
        else if(msg_op == VixMntMsgOp::HALT){
            ILog("stop listening, breaking");
            break;

        }
        else
            ILog("receive %s",getOpValue(msg_op));

    }

    delete vixmntmsg;

    return NULL;
}

void
listening(){

    pthread_t pt_id;
    int err = pthread_create(&pt_id,NULL,vixMntIPC_run,NULL);

    if(err){
      ELog("can't create thread");
      return ;
    }

    ILog("thread running");
}
