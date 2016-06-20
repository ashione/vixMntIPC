#include <vixMntOperation.h>

VixMntOpRead::VixMntOpRead(
        const char* fileName_,
        uint64 bufsize_,
        uint64 offsize_
        )
{
    memcpy(fileName,fileName_,strlen(fileName));
    bufsize  = bufsize_;
    offsize  = offsize_;
}

VixMntOpWrite::VixMntOpWrite(
        const char* fileName_,
        uint64 bufsize_,
        uint64 offsize_
        )
{
    memcpy(fileName,fileName_,strlen(fileName));
    bufsize  = bufsize_;
    offsize  = offsize_;
}

