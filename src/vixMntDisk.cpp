#include <vixMntDisk.h>
#include <vixMntUtility.h>
#include <vixMntOperation.h>

#include <string>

#define SHOW_ERROR_INFO(vixError) \
    if(VIX_FAILED(vixError)){\
        std::string errorMsg = getErrorMsg(vixError); \
        ELog("%s",errorMsg.c_str()); \
    }

VixMntDiskHandle::VixMntDiskHandle(
        VixDiskLibConnection connection,
        const char* path,
        uint32 flag)
{
    _vixHandle = NULL;

    VixError vixError = VixDiskLib_Open(connection,path,flag,&_vixHandle);

    SHOW_ERROR_INFO(vixError);


}

VixMntDiskHandle::~VixMntDiskHandle(){
    VixError vixError = VixDiskLib_Close(_vixHandle);
    SHOW_ERROR_INFO(vixError);
    _vixHandle = NULL;
}

VixError
VixMntDiskHandle::read(VixMntMsgData* msg_data){
     assert(_vixHandle);
     // TODO : write readed buf to mmap
     VixMntOpRead opReadData;
     opReadData.convertFromBytes(msg_data->msg_buff);
     uint8 buf[opReadData.bufsize * VIXDISKLIB_SECTOR_SIZE];


     VixError vixError = VixDiskLib_Read(_vixHandle,opReadData.offsize,opReadData.bufsize,buf);

     return vixError;

}

VixError
VixMntDiskHandle::write(VixMntMsgData* msg_data){
     // TODO : write writed buf to mmap
     VixMntOpRead opWriteData;
     opWriteData.convertFromBytes(msg_data->msg_buff);
     uint8 buf[opWriteData.bufsize * VIXDISKLIB_SECTOR_SIZE];


     VixError vixError = VixDiskLib_Write(_vixHandle,opWriteData.offsize,opWriteData.bufsize,buf);

     return vixError;
}

VixError
VixMntDiskHandle::getDiskInfo(VixMntMsgData* msg_data){
     // TODO :
     VixDiskLibInfo *info = NULL;
     VixError vixError = VixDiskLib_GetInfo(_vixHandle,&info);

     return vixError;

}
