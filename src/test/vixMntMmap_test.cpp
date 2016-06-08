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

const size_t msg_len = 1<<20;
VixMntMmap* testmap = new VixMntMmap(msg_len,true);

void
child_receiver(){

        ILog("shm c_addr : %x",testmap->getDataAddr());
        struct timeval endTime;
        VixMntMsgQue* myque= new VixMntMsgQue("/input",true);

        VixMntMsgData *receiveMsg = new VixMntMsgData();
        myque->receiveMsg(receiveMsg);
        //printf("op %s\n",getOpValue(*receivedOp));
        ILog("op %s",getOpValue(receiveMsg->msg_op));
        char* buff = new char[msg_len];
        testmap->mntReadMmap(buff);
        gettimeofday(&endTime,NULL);
        ILog("receive : %x , %c",buff,buff[0]);
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

        buff[0] = 'A'+milliseconds%10;
        ILog("receiver change buff[0] to %c",buff[0]);
        testmap->mntWriteMmap(buff);

#if defined(__cplusplus) && __cplusplus >= 201103L
        if(resultMsgQSender->sendMsgOp(VixMntMsgOp::MntWriteDone))
#else
        if(resultMsgQSender->sendMsgOp(MntWriteDone))
#endif
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

    if(mq_unlink("/input") < 0){
        ILog("unlink input error");
    }
    if(mq_unlink("/result") < 0 ){
        ILog("unlink result error");
    }

    const int testNum = 3;

    pid_t pid = fork();

    if(pid == 0 ){
        for(int i=0; i<testNum; ++i)
            child_receiver();
        exit(0);
    }

    //VixMntMmap* testmap = new VixMntMmap(msg_len,true);
    VixMntMsgQue* myque = new VixMntMsgQue("/input");

    for(int i=0 ; i <testNum ; ++i){
        ILog("shm p_addr : %x",testmap->getDataAddr());
        memset(msg,random_str[rand()%MMAP_MAX_RANDOM],msg_len);
        gettimeofday(&startTime,NULL);

        testmap->mntWriteMmap(msg);

        const char msg_name[10] = "/result";
        char* buf = new char[sizeof(struct timeval)+strlen(msg_name)];

        ILog("msg_name len : %d",sizeof(msg_name));

        VixMntMsgQue* resultMsgQ = new VixMntMsgQue(msg_name,true);

        memcpy(buf,&startTime,sizeof(struct timeval));
        mqd_t resultMsgQId = resultMsgQ->getVixMntMsgID();
        memcpy(buf+sizeof(struct timeval),msg_name,strlen(msg_name));
#if defined(__cplusplus) && __cplusplus >= 201103L
        VixMntMsgData* timeMsg = new VixMntMsgData(VixMntMsgOp::MntWrite,sizeof(struct timeval)+strlen(msg_name),buf);
#else
        VixMntMsgData* timeMsg = new VixMntMsgData(MntWrite,sizeof(struct timeval)+strlen(msg_name),buf);
#endif

        myque->sendMsg(timeMsg);


        VixMntMsgOp* resultOp = new VixMntMsgOp();
        resultMsgQ->receiveMsgOp(resultOp);

#if defined(__cplusplus) && __cplusplus >= 201103L
        if(*resultOp != VixMntMsgOp::ERROR){
#else
        if(*resultOp != ERROR){
#endif

            ILog("op %s\n",getOpValue(*resultOp));
            testmap->mntReadMmap(msg);
            ILog("share memory show changed bit %c",msg[0]);
        }
        else{
             ILog("vixMntMsgOp : Error\n");
        }

        delete[] buf;
        delete resultOp;
        delete resultMsgQ;
    }

    delete testmap;

    return 0;
}
