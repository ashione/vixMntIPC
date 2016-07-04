#include <vixMntOperation.h>

VixMntOpRead::VixMntOpRead(
        const char* fileName_,
        uint64 bufsize_,
        uint64 offset_
        )
{
    memcpy(fileName,fileName_,strlen(fileName));
    bufsize  = bufsize_;
    offset  = offset_;
}

VixMntOpWrite::VixMntOpWrite(
        const char* fileName_,
        uint64 bufsize_,
        uint64 offset_
        )
{
    memcpy(fileName,fileName_,strlen(fileName));
    bufsize  = bufsize_;
    offset  = offset_;
}

