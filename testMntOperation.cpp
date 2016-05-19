#include <vixMntOperation.h>
#include <vixMntMsgOp.h>
#include <vixMntMsgQue.h>

#include <iostream>
#include <cstdio>
#include <cstring>
#include <cstdlib>
#include <memory>

using namespace std;

int
main(int argc,char** argv){

    pid_t pid = fork();
    if( pid == 0  ){
        char buf[] = "ashione";
        VixMntOpRead t("/tmp",buf,3,2);
        printf("OpRead struct size : %d %d %d\n",sizeof(VixMntOpRead),t.size(),sizeof(t));
        VixMntMsgQue* msgQue = new VixMntMsgQue("/operation");
        char* msgbuf = new char[sizeof(t)];
        memcpy(msgbuf,&t,sizeof(t));
        VixMntMsgData* msgdata = new VixMntMsgData(VixMntMsgOp::MntInit,sizeof(t),msgbuf);
        msgQue->sendMsg(msgdata);
        printf("send ok\n");
        return 0;
    }

    //VixMntMsgQue* msgQue = VixMntMsgQue::getMsgQueInstance();
    VixMntMsgQue* msgQue = new VixMntMsgQue("/operation");

    VixMntMsgData* msgdata = new VixMntMsgData();
    msgQue->receiveMsg(msgdata);
    printf("%s\n",getOpValue(msgdata->msg_op));


    return 0;
}
