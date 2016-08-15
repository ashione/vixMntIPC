#include <vixMntOperation.h>

/**
 * @brief fuse parameters protocol
 *
 * @param fileName_ [in] diskname or message queue
 * @param bufsize_ [in] buffer size of one operation
 * @param offset_ [in] offset of disk
 */

VixMntOpBase::VixMntOpBase(const char *fileName_, uint64 bufsize_,
                           uint64 offset_) {
   int fileNameLen = strlen(fileName_);
   memcpy(fileName, fileName_, fileNameLen);
   fileName[fileNameLen]='\0';
   bufsize = bufsize_;
   offset = offset_;
}


/**
 * @brief read opeartion protocol
 *
 * @param fileName_ [in]
 * @param bufsize_ [in]
 * @param offset_ [in]
 */

VixMntOpRead::VixMntOpRead(const char *fileName_, uint64 bufsize_,
                           uint64 offset_)
   : VixMntOpBase(fileName_, bufsize_, offset_) {}

/**
 * @brief write operation protocol
 *
 * @param fileName_ [in]
 * @param bufsize_ [in]
 * @param offset_ [in]
 */

VixMntOpWrite::VixMntOpWrite(const char *fileName_, uint64 bufsize_,
                             uint64 offset_)
   : VixMntOpBase(fileName_, bufsize_, offset_) {}
