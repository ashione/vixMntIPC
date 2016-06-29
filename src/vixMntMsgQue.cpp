#include <vixMntMsgQue.h>
#include <vixMntMsgOp.h>
#include <vixMntLock.h>
#include <vixMntException.h>

VixMntMsgQue* VixMntMsgQue::vixMntMsgInstance = NULL;
const std::string VixMntMsgQue::vixMntMsgName = "/vixMntApi";
std::map<std::string,mqd_t> VixMntMsgQue::vixMntMsgMap;
pthread_once_t VixMntMsgQue::ponce = PTHREAD_ONCE_INIT;
//pthread_mutex_t VixMntMsgQue::vixMntMsgLock = PTHREAD_MUTEX_INITIALIZER;
/*
 * abort - > this static pthread_mutex_t lock maybe not work in different threads
 *  TODO :
 *      add semaphore in share memory
 *      add pthread_once for multithread safe
 */
VixMntMsgQue*
VixMntMsgQue::getMsgQueInstance(sem_t *sem){

    if(sem)
        sem_wait(sem);
/*
    if( vixMntMsgInstance  == NULL){
        VixMntMutex lock(&vixMntMsgLock);
        try{
            lock.lock();

            ILog("first init instance, thread ID %u",pthread_self());
            ILog("mutex lock add %x",&vixMntMsgLock);
            vixMntMsgInstance = new VixMntMsgQue();

            lock.unlock();
        }
        catch ( VixMntException& e ){
             ELog("%s",e.what());
        }

    }
    else{
        ILog("already init instance");
    }
*/
    pthread_once(&ponce,&VixMntMsgQue::initInstance);
    if(sem)
        sem_post(sem);

    assert(vixMntMsgInstance != NULL);
    return vixMntMsgInstance;
}

void
VixMntMsgQue::initInstance(){
    vixMntMsgInstance = new VixMntMsgQue();
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
        this->vixMntMsgMapFileName = VixMntMsgQue::vixMntMsgName;
    }
    else{
        this->vixMntMsgMapFileName = msg_name;
    }

    /*
     * return mq_id without open it again
     * if mq exist
     *
    std::map<std::string, mqd_t>::iterator itr = VixMntMsgQue::vixMntMsgMap.find(this->vixMntMsgMapFileName);
    if(itr != VixMntMsgQue::vixMntMsgMap.end()){
        this->vixMntMsgID = itr->second;
        return;
    }
    */

    //ILog("msg map filename %s",this->vixMntMsgMapFileName.c_str());

        this->vixMntMsgID =
            mq_open(
            this->vixMntMsgMapFileName.c_str(),
            O_CREAT | O_RDWR ,
            0644,NULL);

    if( this->vixMntMsgID < 0){
        if(errno == EEXIST){
            WLog("exist mqid : %d | mq_name : %s",this->getVixMntMsgID(),vixMntMsgMapFileName.c_str());
        }
        else{
            ELog("open mesage queue error %s ",strerror(errno));
        }
    }

    assert(this->vixMntMsgID > 0);
    //ILog("original %u, new %u",VixMntMsgQue::vixMntMsgMap[this->vixMntMsgMapFileName],this->vixMntMsgID);
    VixMntMsgQue::vixMntMsgMap[this->vixMntMsgMapFileName] = this->vixMntMsgID;

    //ILog("Messge size %d ",VixMntMsgQue::vixMntMsgMap.size());

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
VixMntMsgQue::releaseMsgQueInstance(sem_t* sem){
    if(sem)
        sem_wait(sem);

    if(vixMntMsgInstance){
        delete vixMntMsgInstance;
        mq_unlink(VixMntMsgQue::vixMntMsgName.c_str());
    }
    vixMntMsgInstance = NULL;

    if(sem)
        sem_post(sem);

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
#if defined(__cplusplus) && __cplusplus >= 201103L
    assert(msg_op != VixMntMsgOp::ERROR);
#else
    assert(msg_op != ERROR);
#endif
    const char* msg_str = getOpValue(msg_op);
    return send(msg_str,strlen(msg_str), msg_prio) >=0 ;
}

void
VixMntMsgQue::receiveMsgOp(VixMntMsgOp* msg_op,
        unsigned* msg_prio)
{
     this->getattr(&this->vixMntMsgAttr);

     char *buf = new char[this->vixMntMsgAttr.mq_msgsize];
    // if not memeset, it may be old value
     memset(buf,0,this->vixMntMsgAttr.mq_msgsize);

     if( receive(buf,this->vixMntMsgAttr.mq_msgsize,msg_prio) < 0 )
#if defined(__cplusplus) && __cplusplus >= 201103L
         *msg_op = VixMntMsgOp::ERROR;
#else
         *msg_op = ERROR;
#endif
     else
         *msg_op = getOpIndex(buf);

     delete[] buf;

}

/*
 * @input parma : msg_data
 * @input parma : msg_prio
 * @ouput bool : send msg is ok if return true
 *
 */
bool
VixMntMsgQue::sendMsg(VixMntMsgData* msg_data,
                      unsigned msg_prio)
{
    char *buf = new char[sizeof(VixMntMsgData)];
    memcpy(buf,msg_data,sizeof(VixMntMsgData));
    bool flag = send(buf,sizeof(VixMntMsgData),msg_prio) >= 0;
    delete[] buf;

    return flag;

}

void
VixMntMsgQue::receiveMsg(VixMntMsgData* msg_data,
                      unsigned* msg_prio)
{
    mq_attr tempAttr;
    this->getattr(&tempAttr);
    //ILog("receiveMsg mq_msgsize = %ld,mq_curmsg %ld received msg size = %ld",
    //        vixMntMsgAttr.mq_msgsize,
    //        vixMntMsgAttr.mq_curmsgs,
    //        tempAttr.mq_msgsize);

    assert( 8192 >= tempAttr.mq_msgsize && tempAttr.mq_msgsize > 0);

    //char *buf = new char[tempAttr.mq_msgsize];
    char *buf = new char[tempAttr.mq_msgsize];

    if( receive(buf,tempAttr.mq_msgsize,msg_prio) <0 ){
        ELog("receive error");

#if defined(__cplusplus) && __cplusplus >= 201103L
        msg_data->msg_op = VixMntMsgOp::ERROR;
#else
        msg_data->msg_op = ERROR;
#endif
    }
    else{
        memcpy(msg_data,buf,sizeof(VixMntMsgData));
    }

    delete[] buf;

}
