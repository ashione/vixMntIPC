#include <vixMntMsgQue.h>
#include <vixMntMsgOp.h>
#include <assert.h>
#include <cstdlib>
//#include <str.h>

using namespace std;


VixMntMsgQue* VixMntMsgQue::vixMntMsgInstance = NULL;
const std::string VixMntMsgQue::vixMntMsgName = "/vixMntApi";
std::map<std::string,mqd_t> VixMntMsgQue::vixMntMsgMap;
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
    /*
     * readOnly is unused now.
     */
    this->readOnly = readOnly;

    if(!msg_name){
        //strcpy(this->vixMntMsgMapFileName , VixMntMsgQue::vixMntMsgName);
        //Str_Strcpy(this->vixMntMsgMapFileName , VixMntMsgQue::vixMntMsgName,Str_Strlen(VixMntMsgQue::vixMntMsgName,0x100));
        this->vixMntMsgMapFileName = VixMntMsgQue::vixMntMsgName;
    }
    else{
        //Str_Strcpy(this->vixMntMsgMapFileName , msg_name,Str_Strlen(msg_name,0x100));
        //strcpy(this->vixMntMsgMapFileName , msg_name);
        this->vixMntMsgMapFileName = msg_name;
    }

    ILog("msg map filename %s",this->vixMntMsgMapFileName.c_str());

    //if(!readOnly){
        this->vixMntMsgID =
            mq_open(
            this->vixMntMsgMapFileName.c_str(),
            O_CREAT | O_RDWR ,
            0644,NULL);
    //}
    //else{
    //    this->vixMntMsgID =
    //        mq_open(
    //        this->vixMntMsgMapFileName,O_RDONLY);
    //}

    if( this->vixMntMsgID < 0){
        if(errno == EEXIST){
            WLog("exist mqid : %d | mq_name : %s",this->getVixMntMsgID(),vixMntMsgMapFileName.c_str());
        }
        else{
            ELog("open mesage queue error %s ",strerror(errno));
        }
    }

    assert(this->vixMntMsgID > 0);
    VixMntMsgQue::vixMntMsgMap[this->vixMntMsgMapFileName] = this->vixMntMsgID;
    ILog("Messge size %d ",VixMntMsgQue::vixMntMsgMap.size());

}

VixMntMsgQue::VixMntMsgQue(mqd_t msg_id){

     this->vixMntMsgID = msg_id;
     std::map<std::string,mqd_t>::iterator item = VixMntMsgQue::vixMntMsgMap.begin();
     for( ;item != VixMntMsgQue::vixMntMsgMap.end();item++){
         if( this->vixMntMsgID == item->second ){
             this->vixMntMsgMapFileName = item->first;
         }
     }
     assert(item!= VixMntMsgQue::vixMntMsgMap.end());
}

void
VixMntMsgQue::unlink(){

    std::map<std::string, mqd_t>::iterator itr = VixMntMsgQue::vixMntMsgMap.begin();
    while(itr != VixMntMsgQue::vixMntMsgMap.end()){
        if(mq_unlink(itr->first.c_str()) < 0 ){
            ILog("%s unlink faild.",itr->first.c_str());
        }
        else{
            ILog("%s unlink ok.",itr->first.c_str());
        }
        itr++;
    }
    VixMntMsgQue::vixMntMsgMap.clear();
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
        mq_unlink(VixMntMsgQue::vixMntMsgName.c_str());
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
    /*
     * bug : if op equal to ERROR
     */
    assert(msg_op != VixMntMsgOp::ERROR);
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
    ILog("receiveMsg mq_msgsize = %ld,mq_curmsg %ld received msg size = %ld",
            vixMntMsgAttr.mq_msgsize,
            vixMntMsgAttr.mq_curmsgs,
            tempAttr.mq_msgsize);

    assert( 8192 >= tempAttr.mq_msgsize && tempAttr.mq_msgsize > 0);

    //char *buf = new char[tempAttr.mq_msgsize];
    char *buf = new char[tempAttr.mq_msgsize];

    if( receive(buf,tempAttr.mq_msgsize,msg_prio) <0 ){
        ELog("receive error");

        msg_data->msg_op = VixMntMsgOp::ERROR;
    }
    else{
        memcpy(msg_data,buf,sizeof(VixMntMsgData));
    }

    delete buf;

}
