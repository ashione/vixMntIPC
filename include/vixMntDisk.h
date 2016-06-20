#ifndef VIXMNT_DISK_H
#define VIXMNT_DISK_H

#include <vixMntMsgQue.h>
#include <vixMntMsgOp.h>
#include <vixMntMmap.h>

#include <vixDiskLib.h>
#ifdef __cplusplus
extern "C"{
#endif

class VixMntDiskHandle{

    private :
        //VixDiskLibConnectParams *connectParams;
        //VixDiskLibConnection connection;
        VixDiskLibHandle _vixHandle;
        VixMntMsgQue* _msgQ;
        VixMntMmap* _mmap;

    public  :
        explicit VixMntDiskHandle(VixDiskLibConnection connection,const char* path,uint32 flag);

        ~VixMntDiskHandle();
        void prepare(VixMntMsgQue* msgQ_,VixMntMmap* mmap_);
        void* listen(void* args);

        VixError read(uint8* buf,uint64 offset,uint64 numberSector);

        VixError read(VixMntMsgData* msg_data);

        VixError write(VixMntMsgData* msg_data);

        VixError write(uint8* buf,uint64 offset,uint64 numberSector);

        VixError getDiskInfo(VixDiskLibInfo** info);

        static std::string getErrorMsg(VixError vixError);

};


#ifdef __cplusplus
}
#endif


#endif // VIXMNT_DISK_H
