#include <vixMntOperation.h>

VixMntOpBase::VixMntOpBase(const char *fileName_, uint64 bufsize_,
                           uint64 offset_) {
   int fileNameLen = strlen(fileName_);
   memcpy(fileName, fileName_, fileNameLen);
   fileName[fileNameLen]='\0';
   bufsize = bufsize_;
   offset = offset_;
}

VixMntOpRead::VixMntOpRead(const char *fileName_, uint64 bufsize_,
                           uint64 offset_)
   : VixMntOpBase(fileName_, bufsize_, offset_) {}

VixMntOpWrite::VixMntOpWrite(const char *fileName_, uint64 bufsize_,
                             uint64 offset_)
   : VixMntOpBase(fileName_, bufsize_, offset_) {}
