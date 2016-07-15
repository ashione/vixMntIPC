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
        VixMntOpBase(const char*, uint64, uint64 );
        VixMntOpBase(){};
        virtual void convertToBytes(char* buf) {
            memcpy(buf,this,sizeof(*this));
        }
        virtual void convertFromBytes(const char* buf) {
            memcpy(this,buf,sizeof(*this));
        }
        virtual inline size_t size()  {
            return sizeof(*this);
        }
    public :
        char fileName[VIXMNT_FILENAME_MAXLEN];
        uint64 bufsize;
        uint64  offset;
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

};

class VixMntOpWrite : public VixMntOpBase{

    public :
        explicit VixMntOpWrite( const char*, uint64, uint64);
        explicit VixMntOpWrite(){};
        BREW_CONVERTOR(VixMntOpWrite)

};

class VixMntOpSocket : public VixMntOpBase{
    public :
        VixMntOpSocket(){};
        explicit VixMntOpSocket(
                const char* fileName_,
                uint64 bufsize_,
                uint64 offset_,
                uint64 carriedBufSize_,
                uint64 token_,
                VixMntMsgOp carriedOp_):
            VixMntOpBase(fileName_,bufsize_,offset_),
            carriedBufSize(carriedBufSize_),
            token(token_),
            carriedOp(carriedOp_){}

        BREW_CONVERTOR(VixMntOpSocket)

    public :
        uint64 carriedBufSize;
        uint64 token;
        VixMntMsgOp carriedOp;


};

/*
class VixMntOpSocketRead : public VixMntOpSocketBase{
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

class VixMntOpSocketWrite :  public VixMntOpSocketBase{
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
*/

#ifdef __cplusplus
}
#endif

#endif // VIXMNT_OPERATION_H
