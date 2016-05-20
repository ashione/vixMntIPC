#include <iostream>
#include <cstring>
#include <cstdio>
#include <unistd.h>
#include <mqueue.h>
#include <fcntl.h>
#include <errno.h>
#include <cstdlib>
#include <vixMntMsgQue.h>

using namespace std;
#define MAXTIME 1000

int
testfunc(){
    mqd_t mqID;
    mqID = mq_open("/anonymQueue2",O_RDWR | O_CREAT | O_EXCL , 0664,NULL);
    cout<<"mqId : "<<mqID<<endl;
    if(mqID < 0 ){
        if(errno == EEXIST){
            mq_unlink("/anonymQueue2");
            mqID = mq_open("/anonymQueue2",O_RDWR | O_CREAT | O_EXCL, 0664,NULL);
            cout<<"reopen mqId : "<<mqID<<endl;
        }
        else{
            cout<<" open mesage queue error ... "<<strerror(errno)<<endl;
        }
        return -1;
    }

    if ( fork() == 0  ){

        mq_attr mqAttr;
        mq_getattr(mqID,&mqAttr);

        char *buf = new char[mqAttr.mq_msgsize];

        for ( int i = 1; i<=5 ; ++i ){
            if(mq_receive(mqID,buf,mqAttr.mq_msgsize,NULL)< 0){
                cout<<"receive message failed."<<endl;
                cout<<"error info:"<<strerror(errno)<<endl;
                continue;

            }
            cout<<"receive message "<<i<<" : "<<buf<<endl;
        }

        mq_unlink("/anonymQueue2");
        exit(0);
    }

    char msg[] = "test";

    for(int i= 1; i<=5 ; ++i ){
        if (mq_send(mqID,msg,sizeof(msg),i)< 0){

            cout<<"send message "<<i<<" failed."<<endl;
            cout<<"error info: "<<strerror(errno)<<endl;
        }
        cout<<"send message "<<i<<" sucess "<<endl;
    }

    return 0;
}

void
testVixMntClass(){


//VixMntMsgQue::releaseMsgQueInstance();
    //VixMntMsgQue* vixMntMsgQue::vixMntMsgInstance = NULL;
    VixMntMsgQue* myque= VixMntMsgQue::getMsgQueInstance();
    if( myque ){
         cout<<"INIT OK"<<endl;
    }
    else
        cout<<"INIT FAILED"<<endl;

    if ( fork() == 0  ){

        mq_attr mqAttr;
        myque->getattr(&mqAttr);

        char *buf = new char[mqAttr.mq_msgsize];
        //char buf[0xff];

        for ( int i = 1; i<=MAXTIME ; ++i ){
            if(myque->receive(buf,mqAttr.mq_msgsize,NULL)< 0){
                cout<<"receive message failed."<<endl;
                cout<<"error info:"<<strerror(errno)<<endl;
                continue;

            }
            cout<<"message len : "<<strlen(buf)<<endl;
            cout<<"receive message "<<i<<" : "<<buf<<endl;
        }

        VixMntMsgQue::releaseMsgQueInstance();
        exit(0);
    }

    char msg[] = "vixMntMsgQue";

    for(int i= 1; i<=MAXTIME ; ++i ){
        if (myque->send(msg,sizeof(msg),i%10)< 0){

            cout<<"send message "<<i<<" failed."<<endl;
            cout<<"error info: "<<strerror(errno)<<endl;
        }
        cout<<"send message "<<i<<" sucess "<<endl;
    }

}

void
testsleep(unsigned microseconds ){
     usleep(microseconds);
}

struct testMsgInfo{
    int offsize;
    char buff[0xff];
};

int
main(){

        //VixMntMsgQue* myque= VixMntMsgQue::getMsgQueInstance();
    if( fork() == 0 ){

        VixMntMsgQue* myque= new VixMntMsgQue("/operation",true);

        VixMntMsgData* msgre_data = new VixMntMsgData();
        myque->receiveMsg(msgre_data);

        cout<<"message 1 : "<<getOpValue(msgre_data->msg_op)<<endl;
        testMsgInfo result;
        cout<<"msg_data size : "<<msgre_data->msg_datasize<<endl;
        memcpy(&result,msgre_data->msg_buff,sizeof(testMsgInfo));
        result.buff[result.offsize] = '\0';
        cout<<" receiver : "<<result.offsize<<" "<<result.buff<<endl;

        myque->receiveMsg(msgre_data);

        cout<<"message 2 : "<<getOpValue(msgre_data->msg_op)<<endl;

        //VixMntMsgQue::releaseMsgQueInstance();
        //delete myque;
        delete msgre_data;
        exit(0);
    }

    VixMntMsgQue* myque= new VixMntMsgQue("/operation");
    //VixMntMsgQue* myque= VixMntMsgQue::getMsgQueInstance();
    char pmsg[] = "fkfdf 2 3\n";
    char msg[0xff];
    testMsgInfo info;
    info.offsize=strlen(pmsg);

    memcpy(info.buff,pmsg,strlen(pmsg));

    VixMntMsgData* msgdata = new VixMntMsgData();

    msgdata->msg_op = VixMntMsgOp::MntWrite;
    msgdata->msg_datasize = sizeof(testMsgInfo);

    memcpy(msgdata->msg_buff,&info,sizeof(testMsgInfo));

    myque->sendMsg(msgdata);
    msgdata->msg_op = VixMntMsgOp::MntInit;
    myque->sendMsg(msgdata);
    cout<<"sender :  sizeof VixMntMsgData : "<<sizeof(VixMntMsgData)<<" msg_datasize : "<<msgdata->msg_datasize <<endl;
    delete msgdata;
    //delete myque;
    //myque->sendMsgOp(VixMntMsgOp::MntWrite);
    //cout<<getOpValue(VixMntMsgOp::MntInit)<<endl;
    return 0;
}
