#include <vixMntOperation.h>

VixMntOpRead::VixMntOpRead(
        const char* fileName_,
        char* buf_,
        size_t bufsize_,
        off_t offsize_
        )
{
    memcpy(fileName,fileName_,strlen(fileName));
    //memcpy(buf,buf_,bufsize_);
    //fileName = std::make_shared<const char*>(fileName_);
    //buf      = std::make_shared<char*>(buf_);
    buf = buf_;
    bufsize  = bufsize_;
    offsize  = offsize_;
}

