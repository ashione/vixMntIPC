#include <vixMntOperation.h>
#include <vixMntMsgOp.h>
#include <vixMntMsgQue.h>
#include <vixMntUtility.h>

#include <iostream>
#include <cstdio>
#include <cstring>
#include <cstdlib>
#include <memory>

using namespace std;

int
main(int argc,char** argv){
    //mq_unlink("/operation");
    pid_t pid = fork();
    if( pid == 0  ){

        char buf[] = "ashione";
        VixMntOpRead t("/tmp",buf,3,2);
        ILog("OpRead struct size : %ld %d %ld",sizeof(VixMntOpRead),t.size(),sizeof(t));
        ILog("buf addr : %x %s\n",( long )buf,buf);
        VixMntMsgQue* msgQue = new VixMntMsgQue("/op");
#if defined(__cplusplus) && __cplusplus >= 201103L
        VixMntMsgData* msgdata = new VixMntMsgData(VixMntMsgOp::MntInit,sizeof(t),(char *)&t);
#else
        VixMntMsgData* msgdata = new VixMntMsgData(MntInit,sizeof(t),(char *)&t);
#endif
        msgQue->sendMsg(msgdata);
        ILog("send ok\n");
        delete msgQue;
        exit(0);

    }

    //VixMntMsgQue* msgQue = VixMntMsgQue::getMsgQueInstance();
    VixMntMsgQue* msgQue = new VixMntMsgQue("/op",true);

    VixMntMsgData* msgdata = new VixMntMsgData();
    msgQue->receiveMsg(msgdata);
    ILog("%s",getOpValue(msgdata->msg_op));

    VixMntOpRead* rt = new VixMntOpRead();
    rt->convertFromBytes(msgdata->msg_buff);
    ILog("%s %x %s\n",rt->fileName,rt->buf,rt->buf);
    delete msgQue;
    VixMntMsgQue::unlink();

    return 0;
}
