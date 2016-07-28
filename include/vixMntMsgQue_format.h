#ifndef VIXMNTAPI_MSQQUE_H
#define VIXMNTAPI_MSQQUE_H

#include <vixMntMsgOp.h>
#include <vixMntUtility.h>

#include <assert.h>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <errno.h>
#include <fcntl.h>
#include <iostream>
#include <map>
#include <memory>
#include <mqueue.h>
#include <pthread.h>
#include <semaphore.h>
#include <unistd.h>

#define MSG_FILENAME_LEN_MAX 256

#ifdef __cplusplus
extern "C" {
#endif
/*
 * class VixMntMsgQue is designed to deliver control path message
 * among different process.
 * Actually, suitable queue message size will be helpful.
 */

class VixMntMsgQue {
   // private  :
   //   VixMntMsgQue();
public:
   explicit VixMntMsgQue(const char *msg_name = NULL, bool readOnly = false);
   explicit VixMntMsgQue(mqd_t msg_id);

   static VixMntMsgQue *getMsgQueInstance(sem_t *sem = NULL);
   static void initInstance();
   static void releaseMsgQueInstance(sem_t *sem = NULL);

   inline mqd_t getVixMntMsgID() const { return this->vixMntMsgID; }

   inline void getattr(mq_attr *mqAttr) {
      mq_getattr(getVixMntMsgID(), mqAttr);
   }

   inline void setattr(mq_attr *mqAttr) {
      mq_setattr(getVixMntMsgID(), mqAttr, &this->vixMntMsgAttr);
   }

   ~VixMntMsgQue();

   mqd_t send(const char *msg_data, size_t msg_size, unsigned msg_prio = 0);
   mqd_t receive(char *msg_data, size_t msg_size, unsigned *msg_prio = NULL);

   bool sendMsgOp(VixMntMsgOp msg_op, unsigned msg_prio = 0);
   void receiveMsgOp(VixMntMsgOp *msg_op, unsigned *msg_prio = NULL);

   bool sendMsg(VixMntMsgData *msg_data, unsigned msg_prio = 0);
   void receiveMsg(VixMntMsgData *msg_data, unsigned *msg_prio = NULL);
   static void unlink();

public:
   static VixMntMsgQue *vixMntMsgInstance;
   static const std::string vixMntMsgName;
   static std::map<std::string, mqd_t> vixMntMsgMap;
   // static pthread_mutex_t vixMntMsgLock;
   static pthread_once_t ponce;

public:
   mqd_t vixMntMsgID;
   mq_attr vixMntMsgAttr;
   std::string vixMntMsgMapFileName;
   bool readOnly;
};

#ifdef __cplusplus
}
#endif
#endif // end VIXMNT_API_MSGQUE_H
