#include <vixMntOperation.h>

VixMntOpRead::VixMntOpRead(
        const char* fileName_,
        size_t bufsize_,
        off_t offsize_
        )
{
    memcpy(fileName,fileName_,strlen(fileName));
    bufsize  = bufsize_;
    offsize  = offsize_;
}

VixMntOpWrite::VixMntOpWrite(
        const char* fileName_,
        size_t bufsize_,
        off_t offsize_
        )
{
    memcpy(fileName,fileName_,strlen(fileName));
    bufsize  = bufsize_;
    offsize  = offsize_;
}

