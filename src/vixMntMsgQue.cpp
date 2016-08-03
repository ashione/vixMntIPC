#include <vixMntException.h>
#include <vixMntLock.h>
#include <vixMntMsgOp.h>
#include <vixMntMsgQue.h>

VixMntMsgQue *VixMntMsgQue::vixMntMsgInstance = NULL;
const std::string VixMntMsgQue::vixMntMsgName = "/vixMntApi";
std::map<std::string, mqd_t> VixMntMsgQue::vixMntMsgMap;
pthread_once_t VixMntMsgQue::ponce = PTHREAD_ONCE_INIT;

/***
 ****************************************************************************
 * VixMntMsgQUe::getMsgQueInstance
 * produce singleton of VixMntMsgQue that is utilized bettween parent
 * process and son process.
 * -------------------------------------------------------------------------
 * input parameters  :
 * sem, make sure it's singleton if sem is passed
 * -------------------------------------------------------------------------
 * output paremeters :
 * VixMntMsgQue
 * -------------------------------------------------------------------------
 * Side Effect:
 * No
 ****************************************************************************
 */

/**
 * abort - > this static pthread_mutex_t lock maybe not work in different
 * threads
 *  TODO :
 *    add semaphore in share memory
 *    add pthread_once for multithread safe
 */

VixMntMsgQue
*VixMntMsgQue::getMsgQueInstance(sem_t *sem) { // IN

   if (sem)
      sem_wait(sem);
   pthread_once(&ponce, &VixMntMsgQue::initInstance);
   if (sem)
      sem_post(sem);

   assert(vixMntMsgInstance != NULL);
   return vixMntMsgInstance;
}

/**
 ****************************************************************************
 * VixMntMsgQue::initInstance
 * guarantee singleton with pthread_once
 * -------------------------------------------------------------------------
 * input parameters  :
 * no
 * -------------------------------------------------------------------------
 * output paremeters :
 * no
 * -------------------------------------------------------------------------
 * Side Effect:
 * No
 ****************************************************************************
 */

void
VixMntMsgQue::initInstance() { vixMntMsgInstance = new VixMntMsgQue(); }

/**
 ****************************************************************************
 * VixMntMsgQue Constructor
 * -------------------------------------------------------------------------
 * input parameters  :
 * msg_name , message queue name
 * readOnly , message queue IO attr
 * -------------------------------------------------------------------------
 * output paremeters :
 * -------------------------------------------------------------------------
 * Side Effect:
 * No
 ****************************************************************************
 */

VixMntMsgQue::VixMntMsgQue(const char *msg_name, // IN
                           bool readOnly) {      // IN

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
 ****************************************************************************
 * VixMntMsgQue Constructor
 * reuse a certain message queue by msg_id
 * -------------------------------------------------------------------------
 * input parameters  :
 * msg_id
 * -------------------------------------------------------------------------
 * output paremeters :
 *
 * -------------------------------------------------------------------------
 * Side Effect:
 * No
 ****************************************************************************
 */

VixMntMsgQue::VixMntMsgQue(mqd_t msg_id) { // IN

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
 ****************************************************************************
 * VixMntMsgQue::unlink
 * unlink all created message queue by static member vixMntMsgMap.
 * -------------------------------------------------------------------------
 * input parameters  :
 * No
 * -------------------------------------------------------------------------
 * output paremeters :
 * No
 * -------------------------------------------------------------------------
 * Side Effect:
 * No
 ****************************************************************************
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
 ****************************************************************************
 * VixMntMsgQue Deconstructor
 * -------------------------------------------------------------------------
 * input parameters  :
 * No
 * -------------------------------------------------------------------------
 * output paremeters :
 * No
 * -------------------------------------------------------------------------
 * Side Effect:
 * No
 ****************************************************************************
 */

VixMntMsgQue::~VixMntMsgQue() {

   if (this->vixMntMsgID != -1) {
      mq_close(this->vixMntMsgID);
   }
}

/**
 ****************************************************************************
 * VixMntMsgQue::releaseMsgQueInstance
 * release singleton of message queue.
 * -------------------------------------------------------------------------
 * input parameters  :
 * sem, if enable singleton in multiprocess
 * -------------------------------------------------------------------------
 * output paremeters :
 * -------------------------------------------------------------------------
 * Side Effect:
 * No
 ****************************************************************************
 */

void
VixMntMsgQue::releaseMsgQueInstance(sem_t *sem) { // IN

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
 ****************************************************************************
 * VixMntMsgQue::send
 * -------------------------------------------------------------------------
 * input parameters  :
 * msg_ptr,    message buffer pointer
 * msg_len,    message buffer length
 * msg_prio,   message buffer priority
 * -------------------------------------------------------------------------
 * output paremeters :
 * mqd_t, return sender's mqd_t if operation is successful,
 *        otherwise return -1
 * -------------------------------------------------------------------------
 * Side Effect:
 * No
 ****************************************************************************
 */

mqd_t
VixMntMsgQue::send(const char *msg_ptr, // IN
                   size_t msg_len,      // IN
                   unsigned msg_prio)   // IN
{
   // disable send function, when msg queue is readonly
   assert(!this->readOnly);
   return mq_send(this->getVixMntMsgID(), msg_ptr, msg_len, msg_prio);
}

/**
 ****************************************************************************
 * VixMntMsgQue::receive
 * -------------------------------------------------------------------------
 * input parameters  :
 * msg_ptr,    message buffer pointer
 * msg_len,    message buffer length
 * msg_prio,   message buffer priority
 * -------------------------------------------------------------------------
 * output paremeters :
 * mqd_t, return sender's mqd_t if operation is successful,
 *        otherwise return -1
 * -------------------------------------------------------------------------
 * Side Effect:
 * No
 ****************************************************************************
 */

mqd_t
VixMntMsgQue::receive(char *msg_ptr,      // IN
                      size_t msg_len,     // IN
                      unsigned *msg_prio) // IN
{
   return mq_receive(this->getVixMntMsgID(), msg_ptr, msg_len, msg_prio);
}

/**
 ****************************************************************************
 * VixMntMsgQue::sendMsgOp
 * -------------------------------------------------------------------------
 * input parameters  :
 * msg_op,     message operation
 * msg_prio,   message buffer priority
 * -------------------------------------------------------------------------
 * output paremeters :
 * bool, return true otherwise false
 * -------------------------------------------------------------------------
 * Side Effect:
 * No
 ****************************************************************************
 */

bool
VixMntMsgQue::sendMsgOp(VixMntMsgOp msg_op, // IN
                        unsigned msg_prio)  // IN
{

/**
* bug : if op equal to ERROR
*/
#if defined(__cplusplus) && __cplusplus >= 201103L
   assert(msg_op != VixMntMsgOp::ERROR);
#else
   assert(msg_op != ERROR);
#endif
   const char *msg_str = getOpValue(msg_op);
   return send(msg_str, strlen(msg_str), msg_prio) >= 0;
}

/**
 ****************************************************************************
 * VixMntMsgQue::receiveMsgOp
 * -------------------------------------------------------------------------
 * input parameters  :
 * msg_prio,   message buffer priority
 * -------------------------------------------------------------------------
 * output paremeters :
 * msg_op,     message operation
 * -------------------------------------------------------------------------
 * Side Effect:
 * No
 ****************************************************************************
 */

void
VixMntMsgQue::receiveMsgOp(VixMntMsgOp *msg_op, // OUT
                                unsigned *msg_prio)  // IN
{
   this->getattr(&this->vixMntMsgAttr);

   char *buf = new char[this->vixMntMsgAttr.mq_msgsize];
   // if not memeset, it may be old value
   memset(buf, 0, this->vixMntMsgAttr.mq_msgsize);

   if (receive(buf, this->vixMntMsgAttr.mq_msgsize, msg_prio) < 0)
#if defined(__cplusplus) && __cplusplus >= 201103L
      *msg_op = VixMntMsgOp::ERROR;
#else
      *msg_op = ERROR;
#endif
   else
      *msg_op = getOpIndex(buf);

   delete[] buf;
}

/**
 ****************************************************************************
 * VixMntMsgQue::sendMsg
 * -------------------------------------------------------------------------
 * input parameters  :
 * msg_data,   message data includeing message operation
 *             and message data of control path
 * msg_prio,   message buffer priority
 * -------------------------------------------------------------------------
 * output paremeters :
 * bool , return true if send successful, otherwise return false
 * -------------------------------------------------------------------------
 * Side Effect:
 * No
 ****************************************************************************
 */

bool
VixMntMsgQue::sendMsg(VixMntMsgData *msg_data, // IN
                      unsigned msg_prio)       // IN
{
   char *buf = new char[sizeof(VixMntMsgData)];
   memcpy(buf, msg_data, sizeof(VixMntMsgData));
   bool flag = send(buf, sizeof(VixMntMsgData), msg_prio) >= 0;
   delete[] buf;

   return flag;
}

/**
 ****************************************************************************
 * VixMntMsgQue::receiveMsg
 * -------------------------------------------------------------------------
 * input parameters  :
 * msg_data, libfuse operator, offset and data sector size
 * msg_prio, message priority
 * -------------------------------------------------------------------------
 * output paremeters :
 * No
 * -------------------------------------------------------------------------
 * Side Effect
 * No
 ****************************************************************************
 */

void
VixMntMsgQue::receiveMsg(VixMntMsgData *msg_data, // IN
                         unsigned *msg_prio)      // IN
{
   mq_attr tempAttr;
   this->getattr(&tempAttr);
   assert(8192 >= tempAttr.mq_msgsize && tempAttr.mq_msgsize > 0);

   char *buf = new char[tempAttr.mq_msgsize];

   if (receive(buf, tempAttr.mq_msgsize, msg_prio) < 0) {
      ELog("receive error");

#if defined(__cplusplus) && __cplusplus >= 201103L
      msg_data->msg_op = VixMntMsgOp::ERROR;
#else
      msg_data->msg_op = ERROR;
#endif
   } else {
      memcpy(msg_data, buf, sizeof(VixMntMsgData));
   }

   delete[] buf;
}
