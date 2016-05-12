#ifndef VIXMNTAPI_MSQQUE_H
#define VIXMNTAPI_MSQQUE_H
#include <cstring>
#include <cstdio>
#include <unistd.h>
#include <mqueue.h>
#include <fcntl.h>
#include <errno.h>
#include <cstdlib>
#include <iostream>
#include <memory>

class VixMntMsgQue {
    private  :
        VixMntMsgQue();
    public :

        static VixMntMsgQue* getMsgQueInstance();

        static void releaseMsgQueInstance();

        inline mqd_t getVixMntMsgID() const {
         return this->vixMntMsgID;
        }

        inline void getattr(mq_attr *mqAttr) const { mq_getattr(getVixMntMsgID(),mqAttr); }

        inline void setattr(mq_attr *mqAttr){ mq_setattr(getVixMntMsgID(),mqAttr,&this->vixMntMsgAttr); }
        ~VixMntMsgQue();

        mqd_t send( const char* , size_t , unsigned);
        mqd_t receive( char* , size_t , unsigned*);


    public :
        static VixMntMsgQue* vixMntMsgInstance;
        static const char* vixMntMsgName;

    private :
        mqd_t vixMntMsgID;
        mq_attr vixMntMsgAttr;


};

//VixMntMsgQue* VixMntMsgQue::vixMntMsgInstance = NULL;
#endif
