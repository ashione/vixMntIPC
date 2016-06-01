#include <vixMntUtility.h>
#include <vixMntMsgQue.h>
#include <vixMntMsgOp.h>

#include <cstring>
#include <cstdio>
#include <cstdlib>

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

    pid_t pid = fork();

    if(!pid){
        vixMntIPC_WriteMmap(msg,10,msg_len);

        exit(0);
    }

    sleep(1);
    vixMntIPC_ReadMmap(buf,10,msg_len);

    vixMntIPC_CleanMmap();

    ILog("buf len %d  -- (%s)",msg_len,buf);
    printf("%s\n",buf);

    delete buf;

    listening();

    VixMntMsgQue* msgque = VixMntMsgQue::getMsgQueInstance();

    /*
     * TODO :
     *  will received ERROR when send HALT
     */

    msgque->sendMsgOp(VixMntMsgOp::MntInit);
    msgque->sendMsgOp(VixMntMsgOp::MntWrite);
    msgque->sendMsgOp(VixMntMsgOp::MntRead);

    msgque->sendMsgOp(VixMntMsgOp::HALT);

    sleep(4);
    msgque->unlink();
    delete msgque;
    ILog("end all");
    return 0;
}
