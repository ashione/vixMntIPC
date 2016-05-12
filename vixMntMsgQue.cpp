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
        if(vixMntMsgInstance->vixMntMsgID < 0){
            if(errno == EEXIST){

            mq_unlink(VixMntMsgQue::vixMntMsgName);
            vixMntMsgInstance->vixMntMsgID =
                mq_open(VixMntMsgQue::vixMntMsgName,
                        O_RDWR | O_CREAT | O_EXCL,
                        0664,
                        NULL);
            cout<<"reopen mqId : "<<vixMntMsgInstance->getVixMntMsgID()<<endl;
            }
            else{
                cout<<" open mesage queue error ... "<<strerror(errno)<<endl;
            }
        }

        return vixMntMsgInstance;
}

VixMntMsgQue::VixMntMsgQue(){

    this->vixMntMsgID = mq_open(VixMntMsgQue::vixMntMsgName,O_RDWR | O_CREAT | O_EXCL , 0664,NULL);

}

VixMntMsgQue::~VixMntMsgQue(){
    mq_unlink(VixMntMsgQue::vixMntMsgName);
}

void
VixMntMsgQue::releaseMsgQueInstance(){
    if(vixMntMsgInstance)
        delete vixMntMsgInstance;
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
        unsigned msg_prio = 0)
{
    const char* msg_str = getOpValue(msg_op);
    return send(msg_str,strlen(msg_str), msg_prio) >=0 ;
}

void
VixMntMsgQue::receiveMsgOp(VixMntMsgOp* msg_op,
        unsigned* msg_prio = NULL)
{
     this->getattr(&this->vixMntMsgAttr);
     VixMntMsgOp* result = new VixMntMsgOp();

     char *buf = new char[this->vixMntMsgAttr.mq_msgsize];

     if( receive(buf,this->vixMntMsgAttr.mq_msgsize,msg_prio) <= 0 )
         *result = VixMntMsgOp::ERROR;
     else
         *result = getOpIndex(buf);

     *msg_op = *result;
     delete result;
     delete buf;

}
