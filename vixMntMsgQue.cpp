#include <iostream>
#include <vixMntMsgQue.h>
#include <vixMntMsgOp.h>

using namespace std;


VixMntMsgQue* VixMntMsgQue::vixMntMsgInstance = NULL;
const char* VixMntMsgQue::vixMntMsgName = "/vixMntApi";

VixMntMsgQue*
VixMntMsgQue::getMsgQueInstance(){

        if( vixMntMsgInstance ){
            return vixMntMsgInstance;
        }

        vixMntMsgInstance = new VixMntMsgQue();

        return vixMntMsgInstance;
}

VixMntMsgQue::VixMntMsgQue(const char* msg_name, bool readonly){
    if(!msg_name){
        this->vixMntMsgMapFileName = VixMntMsgQue::vixMntMsgName;
    }
    else{
        this->vixMntMsgMapFileName = msg_name;
    }
    this->vixMntMsgID = mq_open(
            this->vixMntMsgMapFileName,
            (readonly? O_RDONLY : O_RDWR)
            | O_CREAT , 0644,NULL);
    if(this->vixMntMsgID < 0){
        if(errno == EEXIST){
/*
            mq_unlink(this->vixMntMsgMapFileName);
            this->vixMntMsgID =
            mq_open(this->vixMntMsgMapFileName,
                    O_RDWR | O_CREAT | O_EXCL,
                    0666, NULL);
*/
            cout<<"exist mqId : "<<this->getVixMntMsgID()<<endl;
        }
        else{
            cout<<" open mesage queue error ... "<<strerror(errno)<<endl;
        }
    }

}

VixMntMsgQue::VixMntMsgQue(mqd_t msg_id){
     this->vixMntMsgID = msg_id;;
}

VixMntMsgQue::~VixMntMsgQue(){
    if(this->vixMntMsgID != -1){
        mq_close(this->vixMntMsgID);
        //mq_unlink(this->vixMntMsgMapFileName);
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
    char *buf = new char[tempAttr.mq_msgsize];

    if( receive(buf,tempAttr.mq_msgsize,msg_prio) <0 ){
        msg_data->msg_op = VixMntMsgOp::ERROR;
    }
    else{
        memcpy(msg_data,buf,sizeof(VixMntMsgData));
    }

    delete buf;

}
