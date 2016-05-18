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

int
main(int argc,char** args){

    srand((unsigned) time(NULL));

    struct timeval startTime,endTime;

    const size_t msg_len = 1<<26;
    char *msg = new char[msg_len];
    memset(msg,random_str[rand()%MMAP_MAX_RANDOM],msg_len);
    //time(&startTime);
    gettimeofday(&startTime,NULL);

    VixMntMsgQue* myque= VixMntMsgQue::getMsgQueInstance();
    //VixMntMsgQue* resultMsgQ = new VixMntMsgQue("/result");

    //printf("%s\n",ctime(&startTime));

    VixMntMmap* testmap = new VixMntMmap(msg_len);
    cout<<testmap->getMmapFileName()<<endl;
    int status;
    pid_t pid = fork();
    if(pid == 0 ){
    //if( argc >= 2  ){
        //sleep(2);
        //VixMntMsgOp *receivedOp = new VixMntMsgOp();
        //myque->receiveMsgOp(receivedOp,NULL);
        VixMntMsgData *receiveMsg = new VixMntMsgData();
        myque->receiveMsg(receiveMsg);
        //printf("op %s\n",getOpValue(*receivedOp));
        printf("op %s\n",getOpValue(receiveMsg->msg_op));
        char* buff = new char[msg_len];
        testmap->mntReadMmap(buff);
        gettimeofday(&endTime,NULL);
        //cout<<"receive : "<<buff<<endl;
        struct timeval carried_time;
        char msg_name[10];

        memcpy(&carried_time,receiveMsg->msg_buff,sizeof(struct timeval));
        memcpy(msg_name,receiveMsg->msg_buff+sizeof(struct timeval),7);
        cout<<msg_name<<endl;

        printf("timeval size %d ; msg_datasize %d\n",sizeof(struct timeval),receiveMsg->msg_datasize);

        long long m1 = endTime.tv_sec*1000LL + endTime.tv_usec/1000;
        long long m2 = carried_time.tv_sec*1000LL - carried_time.tv_usec/1000 ;
        long long milliseconds = m1 - m2;
        printf("elaped milliseconds : %lld\n",milliseconds);
        VixMntMsgQue* resultMsgQSender  = new VixMntMsgQue(msg_name);
        if(resultMsgQSender->sendMsgOp(VixMntMsgOp::MntWriteDone))
        {
            printf("send msgOp writedone OK\n");
        }
        else{
            printf("%d |  send msgOp writedone failed\n",msg_name);
        }
        delete resultMsgQSender;
        delete buff;
        //delete testmap;
        return 0;
    }

    testmap->mntWriteMmap(msg);

    const char msg_name[10] = "/result";
    char* buf = new char[sizeof(struct timeval)+strlen(msg_name)];
    cout<<"msg_name len : "<<sizeof(msg_name)<<endl;
    VixMntMsgQue* resultMsgQ = new VixMntMsgQue(msg_name,true);
    //cout<<"buf size: "<<sizeof(buf)<<" "<<sizeof(*buf2)<<endl;
    memcpy(buf,&startTime,sizeof(struct timeval));
    mqd_t resultMsgQId = resultMsgQ->getVixMntMsgID();
    memcpy(buf+sizeof(struct timeval),msg_name,strlen(msg_name));
    VixMntMsgData* timeMsg = new VixMntMsgData(VixMntMsgOp::MntWrite,sizeof(struct timeval)+strlen(msg_name),buf);
    myque->sendMsg(timeMsg);

    //waitpid(pid,&status,0);

    VixMntMsgOp* resultOp = new VixMntMsgOp();
    resultMsgQ->receiveMsgOp(resultOp);

    if(*resultOp != VixMntMsgOp::ERROR){
        printf("op %s\n",getOpValue(*resultOp));
    }
    else{
         printf("vixMntMsgOp : Error\n");
    }
    myque->releaseMsgQueInstance();
    delete buf;
    delete resultOp;
    delete testmap;
    delete resultMsgQ;

    return 0;
}
