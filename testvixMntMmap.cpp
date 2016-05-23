#include <vixMntMmap.h>
#include <vixMntMsgQue.h>
#include <vixMntMsgOp.h>

#include <sys/time.h>
#include <sys/wait.h>
#include <iostream>
#include <cstring>
#include <ctime>
#include <string>
#include <time.h>

using namespace std;

extern const char* random_str;

const size_t msg_len = 1<<10;
VixMntMmap* testmap = new VixMntMmap(msg_len,true);

void
child_receiver(){

        struct timeval endTime;
        VixMntMsgQue* myque= new VixMntMsgQue("/input",true);

        VixMntMsgData *receiveMsg = new VixMntMsgData();
        myque->receiveMsg(receiveMsg);
        //printf("op %s\n",getOpValue(*receivedOp));
        ILog("op %s",getOpValue(receiveMsg->msg_op));
        char* buff = new char[msg_len];
        testmap->mntReadMmap(buff);
        gettimeofday(&endTime,NULL);
        //cout<<"receive : "<<buff<<endl;
        struct timeval carried_time;
        char msg_name[10];

        memcpy(&carried_time,receiveMsg->msg_buff,sizeof(struct timeval));
        memcpy(msg_name,receiveMsg->msg_buff+sizeof(struct timeval),7);

        ILog("timeval size %d ; msg_datasize %d",sizeof(struct timeval),receiveMsg->msg_datasize);

        long long m1 = endTime.tv_sec*1000LL + endTime.tv_usec/1000;
        long long m2 = carried_time.tv_sec*1000LL - carried_time.tv_usec/1000 ;
        long long milliseconds = m1 - m2;
        ILog("elaped milliseconds : %lld",milliseconds);
        ILog("received msg_name %s",msg_name);

        VixMntMsgQue* resultMsgQSender  = new VixMntMsgQue(msg_name);

        if(resultMsgQSender->sendMsgOp(VixMntMsgOp::MntWriteDone))
        {
            ILog("send msgOp writedone OK");
        }
        else{
            ILog("%d |  send msgOp writedone failed",msg_name);
        }
        delete resultMsgQSender;
        delete buff;
        //delete testmap;
}

int
main(int argc,char** args){

    srand((unsigned) time(NULL));

    struct timeval startTime;

    char *msg = new char[msg_len];
    memset(msg,random_str[rand()%MMAP_MAX_RANDOM],msg_len);

    mq_unlink("/input");
    mq_unlink("/result");

    VixMntMsgQue* myque = new VixMntMsgQue("/input");
    const int testNum = 1;

    pid_t pid = fork();

    if(pid == 0 ){
        for(int i=0; i<testNum; ++i)
            child_receiver();
        exit(0);
    }

    for(int i=0 ; i <testNum ; ++i){

        gettimeofday(&startTime,NULL);

        testmap->mntWriteMmap(msg);

        const char msg_name[10] = "/result";
        char* buf = new char[sizeof(struct timeval)+strlen(msg_name)];

        ILog("msg_name len : %d",sizeof(msg_name));

        VixMntMsgQue* resultMsgQ = new VixMntMsgQue(msg_name,true);

        memcpy(buf,&startTime,sizeof(struct timeval));
        mqd_t resultMsgQId = resultMsgQ->getVixMntMsgID();
        memcpy(buf+sizeof(struct timeval),msg_name,strlen(msg_name));
        VixMntMsgData* timeMsg = new VixMntMsgData(VixMntMsgOp::MntWrite,sizeof(struct timeval)+strlen(msg_name),buf);

        myque->sendMsg(timeMsg);


        VixMntMsgOp* resultOp = new VixMntMsgOp();
        resultMsgQ->receiveMsgOp(resultOp);

        if(*resultOp != VixMntMsgOp::ERROR){
            ILog("op %s\n",getOpValue(*resultOp));
        }
        else{
             ILog("vixMntMsgOp : Error\n");
        }
        delete buf;
        delete resultOp;
        delete resultMsgQ;
    }

    delete testmap;

    return 0;
}
