#ifndef VIXMNT_OPEARTION_H
#define VIXMNT_OPEARTION_H

#include <vixDiskLib.h>
#include <vixMntMsgOp.h>

#include <sys/types.h>
#include <cstring>
#include <memory>

#ifdef __cplusplus
extern "C" {
#endif

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

/*
 * warning : bufsize is sector number in vixdisklib
 *
 */

class VixMntOpRead : public VixMntOpBase{

    public :
        explicit VixMntOpRead( const char*, uint64, uint64);
        explicit VixMntOpRead(){};
        BREW_CONVERTOR(VixMntOpRead)

    public :
        //def_SHARE(const char*, fileName);
        //def_SHARE(char*, buf);
        char fileName[VIXMNT_FILENAME_MAXLEN];
        //char buf[VIXMNT_TRANSPORT_BUF_MAXLEN];
        //char* buf;
        uint64 bufsize;
        uint64  offset;
};

class VixMntOpWrite : public VixMntOpBase{

    public :
        explicit VixMntOpWrite( const char*, uint64, uint64);
        explicit VixMntOpWrite(){};
        BREW_CONVERTOR(VixMntOpWrite)

    public :
        char fileName[VIXMNT_FILENAME_MAXLEN];
        uint64 bufsize;
        uint64 offset;
};

class VixMntOpSocketBase{
    public :
        VixMntOpSocketBase(){};
        explicit VixMntOpSocketBase(
                uint64 carriedBufSize_,
                uint64 token_,
                VixMntMsgOp carriedOp_):
            carriedBufSize(carriedBufSize_),
            token(token_),
            carriedOp(carriedOp_){}

    public :
        uint64 carriedBufSize;
        uint64 token;
        VixMntMsgOp carriedOp;


};

class VixMntOpSocketRead : public VixMntOpRead, public VixMntOpSocketBase{
    public :
        VixMntOpSocketRead(){};
        explicit VixMntOpSocketRead(
                uint64 bufsize,
                uint64 offset,
                uint64 token = 0,
                uint64 carriedBufSize = 0) :
            VixMntOpRead("/fakeReadToken",bufsize,offset),
            VixMntOpSocketBase(carriedBufSize,token,VixMntOp(MntRead))
    {

    }
        BREW_CONVERTOR(VixMntOpSocketRead)

};

class VixMntOpSocketWrite : public VixMntOpWrite, public VixMntOpSocketBase{
    public :
        VixMntOpSocketWrite();
        explicit VixMntOpSocketWrite(
                uint64 bufsize,
                uint64 offset,
                uint64 token =0,
                uint64 carriedBufSize = 0) :
            VixMntOpWrite("/fakeWriteToken",bufsize,offset),
            VixMntOpSocketBase(carriedBufSize,token,VixMntOp(MntWrite))
    {

    }
        BREW_CONVERTOR(VixMntOpSocketWrite)

};

#ifdef __cplusplus
}
#endif

#endif // VIXMNT_OPERATION_H
