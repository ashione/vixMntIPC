#ifndef VIXMNT_DISK_H
#define VIXMNT_DISK_H

#include <vixMntMsgQue.h>
#include <vixMntMsgOp.h>

#include <vixDiskLib.h>
#ifdef __cplusplus
extern "C"{
#endif

class VixMntDiskHandle{

    private :
        //VixDiskLibConnectParams *connectParams;
        //VixDiskLibConnection connection;
        VixDiskLibHandle _vixHandle;

    public  :
        VixMntDiskHandle(VixDiskLibConnection connection,const char* path,uint32 flag);

        ~VixMntDiskHandle();

        VixError read(VixMntMsgData* msg_data);

        VixError write(VixMntMsgData* msg_data);

        VixError getDiskInfo(VixMntMsgData* msg_data);

};

#ifdef __cplusplus
}
#endif

#endif // VIXMNT_DISK_H
