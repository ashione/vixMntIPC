#include <vixMntException.h>
#include <vixMntLock.h>
#include <vixMntMsgOp.h>
#include <vixMntMsgQue.h>

VixMntMsgQue *VixMntMsgQue::vixMntMsgInstance = NULL;
const std::string VixMntMsgQue::vixMntMsgName = "/vixMntApi";
std::map<std::string, mqd_t> VixMntMsgQue::vixMntMsgMap;
pthread_once_t VixMntMsgQue::ponce = PTHREAD_ONCE_INIT;

/**
 * @brief
 * produce singleton of VixMntMsgQue that is utilized bettween parent
 * process and son process.
 *
 * @param sem [in] make sure it's singleton if sem is passed
 *
 * @return
 */

VixMntMsgQue
*VixMntMsgQue::getMsgQueInstance(sem_t *sem) {

   if (sem)
      sem_wait(sem);
   pthread_once(&ponce, &VixMntMsgQue::initInstance);
   if (sem)
      sem_post(sem);

   assert(vixMntMsgInstance != NULL);
   return vixMntMsgInstance;
}


/**
 * @brief  guarantee singleton with pthread_once
 */

void
VixMntMsgQue::initInstance() { vixMntMsgInstance = new VixMntMsgQue(); }

/**
 * @brief
 *
 * @param msg_name [in] message queue name
 * @param readOnly [in] message IO arrt
 */

VixMntMsgQue::VixMntMsgQue(const char *msg_name,
                           bool readOnly) {

   // readOnly is unused now.
   this->readOnly = readOnly;

   if (!msg_name) {
      this->vixMntMsgMapFileName = VixMntMsgQue::vixMntMsgName;
   } else {
      this->vixMntMsgMapFileName = msg_name;
   }


   this->vixMntMsgID =
      mq_open(this->vixMntMsgMapFileName.c_str(), O_CREAT | O_RDWR, 0644, NULL);

   if (this->vixMntMsgID < 0) {
      if (errno == EEXIST) {
         WLog("exist mqid : %d | mq_name : %s", this->getVixMntMsgID(),
              vixMntMsgMapFileName.c_str());
      } else {
         ELog("open mesage queue error %s ", strerror(errno));
      }
   }

   assert(this->vixMntMsgID > 0);
   VixMntMsgQue::vixMntMsgMap[this->vixMntMsgMapFileName] = this->vixMntMsgID;
}


/**
 * @brief  reuse a certain message queue by msg_id
 *
 * @param msg_id
 */

VixMntMsgQue::VixMntMsgQue(mqd_t msg_id) {

   this->vixMntMsgID = msg_id;
   std::map<std::string, mqd_t>::iterator item =
      VixMntMsgQue::vixMntMsgMap.begin();
   for (; item != VixMntMsgQue::vixMntMsgMap.end(); item++) {
      if (this->vixMntMsgID == item->second) {
         this->vixMntMsgMapFileName = item->first;
      }
   }
   assert(item != VixMntMsgQue::vixMntMsgMap.end());
}


/**
 * @brief  unlink all created message queues by static member vixMntMsgMap.
 */

void
VixMntMsgQue::unlink() {

   std::map<std::string, mqd_t>::iterator itr =
      VixMntMsgQue::vixMntMsgMap.begin();
   while (itr != VixMntMsgQue::vixMntMsgMap.end()) {
      if (mq_unlink(itr->first.c_str()) < 0) {
         ILog("%s unlink faild.", itr->first.c_str());
      } else {
         ILog("%s unlink ok.", itr->first.c_str());
      }
      itr++;
   }
   VixMntMsgQue::vixMntMsgMap.clear();
}

/**
 * @brief  close system message queue bus.
 */

VixMntMsgQue::~VixMntMsgQue() {

   if (this->vixMntMsgID != -1) {
      mq_close(this->vixMntMsgID);
   }
}


/**
 * @brief release message queue
 *
 * @param sem [in] if enable singleton in multiprocess
 */

void
VixMntMsgQue::releaseMsgQueInstance(sem_t *sem) {

   if (sem) {
      sem_wait(sem);
   }

   if (vixMntMsgInstance) {
      delete vixMntMsgInstance;
      mq_unlink(VixMntMsgQue::vixMntMsgName.c_str());
   }
   vixMntMsgInstance = NULL;

   if (sem)
      sem_post(sem);
}


/**
 * @brief  send a message
 *
 * @param msg_ptr [in] message buffer pointer
 * @param msg_len [in] message buffer length
 * @param msg_prio [in] message buffer priority
 *
 * @return
 */

mqd_t
VixMntMsgQue::send(const char *msg_ptr,
                   size_t msg_len,
                   unsigned msg_prio)
{
   // disable send function, when msg queue is readonly
   assert(!this->readOnly);
   return mq_send(this->getVixMntMsgID(), msg_ptr, msg_len, msg_prio);
}


/**
 * @brief  receive message  (block)
 *
 * @param msg_ptr [in] message buffer pointer
 * @param msg_len [in] message buffer length
 * @param msg_prio [in] message buffer priority
 *
 * @return
 */

mqd_t
VixMntMsgQue::receive(char *msg_ptr,
                      size_t msg_len,
                      unsigned *msg_prio)
{
   return mq_receive(this->getVixMntMsgID(), msg_ptr, msg_len, msg_prio);
}


/**
 * @brief message message with op
 *
 * @param msg_op [in] message operation
 * @param msg_prio [in] message buffer priority
 *
 * @return
 */

bool
VixMntMsgQue::sendMsgOp(VixMntMsgOp msg_op,
                        unsigned msg_prio)
{
// throw exception if op equal to ERROR
#if defined(__cplusplus) && __cplusplus >= 201103L
   assert(msg_op != VixMntMsgOp::ERROR);
#else
   assert(msg_op != ERROR);
#endif
   const char *msg_str = getOpValue(msg_op);
   return send(msg_str, strlen(msg_str), msg_prio) >= 0;
}


/**
 * @brief receive message operations
 *
 * @param msg_op [out] message operation
 * @param msg_prio [in] message buffer priority
 */

void
VixMntMsgQue::receiveMsgOp(VixMntMsgOp *msg_op,
                                unsigned *msg_prio)
{
   this->getattr(&this->vixMntMsgAttr);

   char *buf = new char[this->vixMntMsgAttr.mq_msgsize];
   // if not memeset, it may be old value
   memset(buf, 0, this->vixMntMsgAttr.mq_msgsize);

   if (receive(buf, this->vixMntMsgAttr.mq_msgsize, msg_prio) < 0) {
      *msg_op = VixMntOp(ERROR);
   } else {
      *msg_op = getOpIndex(buf);
   }

   delete[] buf;
}


/**
 * @brief  send message
 *
 * @param msg_data [in] message data containing message operation and message
 * data of control path
 * @param msg_prio [in] message buffer priority
 *
 * @return
 */

bool
VixMntMsgQue::sendMsg(VixMntMsgData *msg_data,
                      unsigned msg_prio)
{
   char *buf = new char[sizeof(VixMntMsgData)];
   memcpy(buf, msg_data, sizeof(VixMntMsgData));
   bool flag = send(buf, sizeof(VixMntMsgData), msg_prio) >= 0;
   delete[] buf;

   return flag;
}


/**
 * @brief  block waiting for receiving message
 *
 * @param msg_data [out] libfuse operation, offset and data sector size
 * @param msg_prio
 */

void
VixMntMsgQue::receiveMsg(VixMntMsgData *msg_data,
                         unsigned *msg_prio)
{
   mq_attr tempAttr;
   this->getattr(&tempAttr);
   assert(8192 >= tempAttr.mq_msgsize && tempAttr.mq_msgsize > 0);

   char *buf = new char[tempAttr.mq_msgsize];

   if (receive(buf, tempAttr.mq_msgsize, msg_prio) < 0) {
      ELog("receive error");
      msg_data->msg_op = VixMntOp(ERROR);
      throw  VixMntException("Receive Error");
   } else {
      memcpy(msg_data, buf, sizeof(VixMntMsgData));
   }

   delete[] buf;
}
