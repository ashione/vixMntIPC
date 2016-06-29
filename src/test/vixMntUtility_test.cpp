#include <vixMntUtility.h>
#include <vixMntMsgQue.h>
#include <vixMntMsgOp.h>

#include <cstring>
#include <cstdio>
#include <cstdlib>
#include <semaphore.h>

int
main(){
    ILog("Info");

    vixMntIPC_InitMmap(0x100,0);

    const char msg[12] = "mmap test 1";
    size_t msg_len = strlen(msg);
    char *buf = new char[msg_len];

/*
 * test same process whether mmap is works
    vixMntIPC_WriteMmap(msg,0,msg_len);
    vixMntIPC_ReadMmap(buf,0,msg_len);

    vixMntIPC_CleanMmap();

    ILog("buf len %d  -- (%s)",msg_len,buf);
    printf("%s\n",buf);

*/

/*
 * test mmap between two processes
 */

    sem_t *semaphore  = sem_open("/sema",O_CREAT,0777,0);
    pid_t pid = fork();

    if(!pid){
        vixMntIPC_WriteMmap(msg,10,msg_len);
        sem_post(semaphore);
        sem_close(semaphore);
        exit(0);
    }
    sem_wait(semaphore);
    vixMntIPC_ReadMmap(buf,10,msg_len);
    sem_close(semaphore);
    sem_unlink("/sema");
    vixMntIPC_CleanMmap();

    ILog("buf len %d  -- (%s)",msg_len,buf);

    delete[] buf;


    pthread_t pid_t = listening();
    pthread_t pid_t2 = listening();

    VixMntMsgQue* msgque = VixMntMsgQue::getMsgQueInstance();

    ILog("size of msgque instance %u",sizeof(*msgque));
    VixMntMsgQue* msgque2 = new VixMntMsgQue("/test2");
    VixMntMsgQue* msgque3 = new VixMntMsgQue("/test3");

    ILog("size of msgque instance %u",sizeof(VixMntMsgQue));
    delete msgque3;
    delete msgque2;

    if(!pid_t){
        ELog("error goto clean");
        goto clean;
    }

    /*
     * TODO :
     *  will received ERROR when send HALT
     */

    msgque->sendMsgOp(VixMntOp( MntInit ));
    msgque->sendMsgOp(VixMntOp( MntWrite ));
    msgque->sendMsgOp(VixMntOp( MntReadDone ));
    msgque->sendMsgOp(VixMntOp( MntRead ));
    msgque->sendMsgOp(VixMntOp( HALT ));
    msgque->sendMsgOp(VixMntOp( HALT ));

    //sleep(4);
    pthread_join(pid_t,NULL);
    pthread_join(pid_t2,NULL);
    msgque->unlink();

    VixMntMsgQue::releaseMsgQueInstance();
clean:
    ILog("end all");
    return 0;
}
