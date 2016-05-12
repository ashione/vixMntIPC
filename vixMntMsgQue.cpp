#include <iostream>
#include "vixMntMsgQue.h"

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
