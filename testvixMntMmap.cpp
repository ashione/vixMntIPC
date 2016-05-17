#include <vixMntMmap.h>
#include <vixMntMsgQue.h>
#include <vixMntMsgOp.h>

#include <iostream>
#include <cstring>
#include <ctime>

using namespace std;

int main(int argc,char** args){
    time_t startTime,endTime;
    int fd;
    //mode_t mode = S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH;
    char filename[] = "/tmp/file";
    //char msg[]= "testmap";
    char *msg = new char[1<<25];
    memset(msg,'1',1<<25);
    time(&startTime);

    VixMntMsgQue* myque= VixMntMsgQue::getMsgQueInstance();

    printf("%s",ctime(&startTime));
    fd = open(filename, O_RDWR | O_CREAT | O_TRUNC,0666);
    lseek(fd,strlen(msg),SEEK_SET);
    write(fd,"",1);
    VixMntMmap* testmap = new VixMntMmap(strlen(msg),fd);
    if( fork() ){
    //if( argc >= 2  ){
        //sleep(2);
        VixMntMsgOp *receivedOp = new VixMntMsgOp();
        myque->receiveMsgOp(receivedOp,NULL);
        printf("op %s\n",getOpValue(*receivedOp));
        char* buff = new char[strlen(msg)];
        testmap->mntReadMmap(buff);
        time(&endTime);
        //cout<<buff<<endl;
        printf("%s",ctime(&endTime));
        printf("%.6fs\n",difftime(endTime,startTime));
        delete buff;
        delete testmap;
        exit(0);
    }
    testmap->mntWriteMmap(msg);
    //sleep(1);
    myque->sendMsgOp(VixMntMsgOp::MntWrite,0);

    delete testmap;
    return 0;
}
