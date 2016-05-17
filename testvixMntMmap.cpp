#include <vixMntMmap.h>
#include <vixMntMsgQue.h>
#include <vixMntMsgOp.h>

#include <sys/time.h>
#include <iostream>
#include <cstring>
#include <ctime>
#include <string>
#include <time.h>

using namespace std;


extern const char* random_str;
int

main(int argc,char** args){

    struct timeval startTime,endTime;

    const size_t msg_len = 1<<20;
    char *msg = new char[msg_len];
    memset(msg,random_str[rand()%MMAP_MAX_RANDOM],msg_len);
    //time(&startTime);
    gettimeofday(&startTime,NULL);

    VixMntMsgQue* myque= VixMntMsgQue::getMsgQueInstance();

    //printf("%s\n",ctime(&startTime));

    VixMntMmap* testmap = new VixMntMmap(msg_len);
    cout<<testmap->getMmapFileName()<<endl;
    if( fork() ){
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
        memcpy(&carried_time,receiveMsg->msg_buff,sizeof(struct timeval));
        printf("timeval size %d ; msg_datasize %d\n",sizeof(struct timeval),receiveMsg->msg_datasize);
        //printf("%s\n%s\n",ctime(&endTime),ctime(&carried_time));
        //printf("%.6fs\n",difftime(endTime,carried_time));
        long long m1 = endTime.tv_sec*1000LL + endTime.tv_usec/1000;
        long long m2 = carried_time.tv_sec*1000LL - carried_time.tv_usec/1000 ;
        long long milliseconds = m1 - m2;
        printf("elaped milliseconds : %lld\n",milliseconds);
        delete buff;
        delete testmap;
        exit(0);
    }
    testmap->mntWriteMmap(msg);

    //sleep(1);
    char* buf = new char[sizeof(struct timeval)];
    //cout<<"buf size: "<<sizeof(buf)<<" "<<sizeof(*buf2)<<endl;
    memcpy(buf,&startTime,sizeof(struct timeval));
    VixMntMsgData* timeMsg = new VixMntMsgData(VixMntMsgOp::MntWrite,sizeof(struct timeval),buf);
    myque->sendMsg(timeMsg);
    //cout<<"send : "<<msg<<endl;
    //myque->sendMsgOp(VixMntMsgOp::MntWrite,0);

    delete testmap;
    return 0;
}
