#ifndef VIXMNT_DISK_H
#define VIXMNT_DISK_H

#include <vixMntMmap.h>
#include <vixMntMsgOp.h>
#include <vixMntMsgQue.h>

#include <vixDiskLib.h>
#ifdef __cplusplus
extern "C" {
#endif

/**
 * SHOW_ERROR_INFO mainly convert char* errorMsg  to string that can be printed
 * in Log.
 */

#define SHOW_ERROR_INFO(vixError)                                              \
   if (VIX_FAILED(vixError)) {                                                 \
      std::string errorMsg = VixMntDiskHandle::getErrorMsg(vixError);          \
      ELog("%s", errorMsg.c_str());                                            \
   }

/**
 * Note VixMntDiskHandle is defined to be an operator for remote disk. It has
 * capability to read/write remote disk with opening a diskhandle. Actually,
 * New disk operations can be added easily.
 */

class VixMntDiskHandle {
private:
   VixDiskLibHandle _vixHandle;
   VixDiskLibInfo * _vixInfo;
   VixMntMsgQue *_msgQ;
   VixMntMmap *_mmap;

public:
   std::string diskBackingPath;

public:
   explicit VixMntDiskHandle(VixDiskLibConnection connection, const char *path,
                             uint32 flag);
   ~VixMntDiskHandle();
   void prepare(VixMntMsgQue *msgQ_, VixMntMmap *mmap_);
   void *listen(void *args);
   VixError read(uint8 *buf, uint64 offset, uint64 numberSector);
   VixError read(VixMntMsgData *msg_data);
   VixError write(VixMntMsgData *msg_data);
   VixError write(uint8 *buf, uint64 offset, uint64 numberSector);
   VixError getDiskInfo(VixDiskLibInfo **info);
   void freeDiskInfo(VixDiskLibInfo *info);
   static std::string getErrorMsg(VixError vixError);
   inline SectorType getCapacity() {
       assert(_vixInfo);
       return _vixInfo->capacity;
   }
   inline uint32 getSectorSize() {
       assert(_vixInfo);
       return _vixInfo->sectorSize;
   }
};

#ifdef __cplusplus
}
#endif

#endif // VIXMNT_DISK_H
