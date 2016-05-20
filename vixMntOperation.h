#ifndef VIXMNT_OPEARTION_H
#define VIXMNT_OPEARTION_H
#include <sys/types.h>
#include <cstring>
#include <memory>
//#include "fuse_common.h"

#define VIXMNT_FILENAME_MAXLEN 256
#define VIXMNT_TRANSPORT_BUF_MAXLEN 4096

#define BREW_CONVERTOR(classname) \
        void convertToBytes(char* buf) { \
            memcpy(buf,this,sizeof(classname));  \
        } \
         void convertFromBytes(const char* buf) { \
            memcpy(this,buf,sizeof(classname)); \
        } \
        inline size_t size()  { \
            return sizeof(classname); \
        } \
        virtual ~classname(){}

#define def_SHARE(classname,inst) std::shared_ptr<classname> inst

class VixMntOpBase{

    public :

        virtual void convertToBytes(char* buf) {
            memcpy(buf,this,sizeof(*this));
        }
        virtual void convertFromBytes(const char* buf) {
            memcpy(this,buf,sizeof(*this));
        }
        virtual inline size_t size()  {
            return sizeof(*this);
        }
        //virtual ~VixMntOpBase();

};

class VixMntOpRead : public VixMntOpBase{

    public :
        explicit VixMntOpRead(const char*,char* ,size_t,off_t);
        explicit VixMntOpRead(){};
        BREW_CONVERTOR(VixMntOpRead)

    public :
        //def_SHARE(const char*, fileName);
        //def_SHARE(char*, buf);
        char fileName[VIXMNT_FILENAME_MAXLEN];
        //char buf[VIXMNT_TRANSPORT_BUF_MAXLEN];
        char* buf;
        size_t bufsize;
        off_t offsize;
};


#endif // VIXMNT_OPERATION_H
