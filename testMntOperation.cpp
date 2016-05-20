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
        printf("buf addr : %x %s\n",buf,buf);
        VixMntMsgQue* msgQue = new VixMntMsgQue("/operationabcde");
        //char* msgbuf = new char[sizeof(t)];
        //memcpy(msgbuf,&t,sizeof(t));
        VixMntMsgData* msgdata = new VixMntMsgData(VixMntMsgOp::MntInit,sizeof(t),(char *)&t);
        msgQue->sendMsg(msgdata);
        printf("send ok\n");

        exit(0);

    }

    //VixMntMsgQue* msgQue = VixMntMsgQue::getMsgQueInstance();
    VixMntMsgQue* msgQue = new VixMntMsgQue("/operationabcde",true);

    VixMntMsgData* msgdata = new VixMntMsgData();
    msgQue->receiveMsg(msgdata);
    printf("%s\n",getOpValue(msgdata->msg_op));

    VixMntOpRead* rt = new VixMntOpRead();
    rt->convertFromBytes(msgdata->msg_buff);
    printf("%s %x %s\n",rt->fileName,rt->buf,rt->buf);

    return 0;
}
