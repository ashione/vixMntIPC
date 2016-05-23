#include <iostream>
#include <vixMntMsgQue.h>
#include <vixMntMsgOp.h>

using namespace std;


VixMntMsgQue* VixMntMsgQue::vixMntMsgInstance = NULL;
const char* VixMntMsgQue::vixMntMsgName = "/vixMntApi";
std::map<const char*,mqd_t> VixMntMsgQue::vixMntMsgMap;
VixMntMsgQue*
VixMntMsgQue::getMsgQueInstance(){

        if( vixMntMsgInstance ){
            return vixMntMsgInstance;
        }

        vixMntMsgInstance = new VixMntMsgQue();

        return vixMntMsgInstance;
}

VixMntMsgQue::VixMntMsgQue(const char* msg_name,bool readOnly){

    this->vixMntMsgAttr.mq_flags = 0;
    this->vixMntMsgAttr.mq_maxmsg = 8192;
    this->vixMntMsgAttr.mq_msgsize = 8192;
    this->vixMntMsgAttr.mq_curmsgs = 0;

    this->readOnly = readOnly;
    if(!msg_name){
        strcpy(this->vixMntMsgMapFileName , VixMntMsgQue::vixMntMsgName);
    }
    else{
        strcpy(this->vixMntMsgMapFileName , msg_name);
    }

    printf("msg map filename %s\n",this->vixMntMsgMapFileName);

    this->vixMntMsgID =
        mq_open(
            this->vixMntMsgMapFileName,
            this->readOnly? O_RDONLY : O_RDWR
            | O_CREAT | O_EXCL,0666,NULL);

    if( this->vixMntMsgID < 0){
        if(errno == EEXIST){
            //this->unlink();
            //mq_unlink(this->vixMntMsgMapFileName);
            //this->vixMntMsgID =
            //       mq_open(this->vixMntMsgMapFileName,
            //       O_RDWR | O_CREAT | O_EXCL,
            //       0644, NULL);

            printf("exist mqid : %d | mq_name : %s\n",this->getVixMntMsgID(),vixMntMsgMapFileName);
        }
        else{
            cout<<" open mesage queue error ... "<<strerror(errno)<<endl;
        }
    }

    assert(this->vixMntMsgID > 0);
    VixMntMsgQue::vixMntMsgMap[this->vixMntMsgMapFileName] = this->vixMntMsgID;
    //printf("Log : msg_queue size %ld\n",VixMntMsgQue::vixMntMsgMap.size());
    ILog("1 Messge size %d ",VixMntMsgQue::vixMntMsgMap.size());
    ILog("2 Messge size %s","sdf");
    ILog("3 Messge size");

}

VixMntMsgQue::VixMntMsgQue(mqd_t msg_id){

     this->vixMntMsgID = msg_id;
     std::map<const char*,mqd_t>::iterator item = VixMntMsgQue::vixMntMsgMap.begin();
     for( ;item != VixMntMsgQue::vixMntMsgMap.end();item++){
         if( this->vixMntMsgID == item->second ){
             this->vixMntMsgName = item->first;
         }
     }
     assert(item!= VixMntMsgQue::vixMntMsgMap.end());
}

void
VixMntMsgQue::unlink(){

    std::map<const char*, mqd_t>::iterator itr = VixMntMsgQue::vixMntMsgMap.begin();
    while(itr != VixMntMsgQue::vixMntMsgMap.end()){
        if(mq_unlink(itr->first) < 0 ){
            printf("Log %s unlink faild.\n",itr->first);
        }
        else{
            printf("Log %s unlink ok.\n",itr->first);
        }
        itr++;
    }
}
VixMntMsgQue::~VixMntMsgQue(){

    if(this->vixMntMsgID != -1){
        mq_close(this->vixMntMsgID);
    }
}

void
VixMntMsgQue::releaseMsgQueInstance(){
    if(vixMntMsgInstance){
        delete vixMntMsgInstance;
        mq_unlink(VixMntMsgQue::vixMntMsgName);
    }
    vixMntMsgInstance = NULL;

}

mqd_t
VixMntMsgQue::send(const char* msg_ptr,
        size_t msg_len,
        unsigned msg_prio)
{
    //disable send function, when msg queue is readonly
    assert(!this->readOnly);
    return mq_send(this->getVixMntMsgID(),msg_ptr,msg_len,msg_prio);

}

mqd_t
VixMntMsgQue::receive(char* msg_ptr,
         size_t msg_len,
         unsigned* msg_prio )
{
    return mq_receive(this->getVixMntMsgID(),msg_ptr,msg_len,msg_prio);

}

bool
VixMntMsgQue::sendMsgOp(VixMntMsgOp msg_op,
        unsigned msg_prio)
{
    const char* msg_str = getOpValue(msg_op);
    return send(msg_str,strlen(msg_str), msg_prio) >=0 ;
}

void
VixMntMsgQue::receiveMsgOp(VixMntMsgOp* msg_op,
        unsigned* msg_prio)
{
     this->getattr(&this->vixMntMsgAttr);

     char *buf = new char[this->vixMntMsgAttr.mq_msgsize];

     if( receive(buf,this->vixMntMsgAttr.mq_msgsize,msg_prio) < 0 )
         *msg_op = VixMntMsgOp::ERROR;
     else
         *msg_op = getOpIndex(buf);

     delete buf;

}

bool
VixMntMsgQue::sendMsg(VixMntMsgData* msg_data,
                      unsigned msg_prio)
{
    char *buf = new char[sizeof(VixMntMsgData)];
    memcpy(buf,msg_data,sizeof(VixMntMsgData));
    return send(buf,sizeof(VixMntMsgData),msg_prio) >= 0;

}

void
VixMntMsgQue::receiveMsg(VixMntMsgData* msg_data,
                      unsigned* msg_prio)
{
    mq_attr tempAttr;
    this->getattr(&tempAttr);
    printf("Log : [ receiveMsg function in vixMntMsgQue.cpp ] mq_msgsize = %ld,mq_curmsg %ld received msg size = %ld\n",
            vixMntMsgAttr.mq_msgsize,
            vixMntMsgAttr.mq_curmsgs,
            tempAttr.mq_msgsize);

    assert( 8192 >= tempAttr.mq_msgsize && tempAttr.mq_msgsize > 0);

    //char *buf = new char[tempAttr.mq_msgsize];
    char *buf = new char[tempAttr.mq_msgsize];

    if( receive(buf,tempAttr.mq_msgsize,msg_prio) <0 ){
        printf("Log : line %d - %s [ receive error]",__LINE__,__FILE__);

        msg_data->msg_op = VixMntMsgOp::ERROR;
    }
    else{
        memcpy(msg_data,buf,sizeof(VixMntMsgData));
    }

    delete buf;

}
