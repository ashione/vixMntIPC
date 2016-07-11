#include <vixMntDisk.h>
#include <vixMntUtility.h>
#include <vixMntOperation.h>

#include <string>


VixMntDiskHandle::VixMntDiskHandle(
        VixDiskLibConnection connection,
        const char* path,
        uint32 flag)
{
    _vixHandle = NULL;
    ILog("open disklib");
    VixError vixError = VixDiskLib_Open(connection,path,flag,&_vixHandle);

    SHOW_ERROR_INFO(vixError);


}

VixMntDiskHandle::~VixMntDiskHandle(){
    VixError vixError = VixDiskLib_Close(_vixHandle);
    SHOW_ERROR_INFO(vixError);
    _vixHandle = NULL;
}

void
VixMntDiskHandle::prepare(VixMntMsgQue* msgQ_,
        VixMntMmap* mmap_){
    _msgQ = msgQ_;
    _mmap = mmap_;
}

void*
VixMntDiskHandle::listen(void *args){

    if( !_vixHandle || !_msgQ || !_mmap ){
        ELog("no preparation before listening");
        return (void*)NULL;
    }

    VixMntMsgOp msg_op;
    VixMntMsgData msg_data;

    while(true){
        _msgQ->receiveMsg(&msg_data);
        msg_op = msg_data.msg_op;

        if(msg_op == VixMntOp(ERROR)){
            ILog("receive error, breaking");
            break;
        }
        else if(msg_op == VixMntOp(HALT)) {
            ILog(" stop listening, breaking");
            break;
        }
        else if(msg_op == VixMntOp(MntRead)){
            //ILog("receive %s",getOpValue(msg_op));
            SHOW_ERROR_INFO(read(&msg_data));
        }
        else if(msg_op == VixMntOp(MntWrite)){
            //ILog("receive %s",getOpValue(msg_op));
            SHOW_ERROR_INFO(write(&msg_data));
        }
        else{
            ELog("receive exception");
        }
    }

    return NULL;

}

VixError
VixMntDiskHandle::read(
        uint8* buf,
        uint64 offset,
        uint64 numberSector)
{

     VixError vixError = VixDiskLib_Read(_vixHandle,offset,numberSector,buf);
     return vixError;
}

VixError
VixMntDiskHandle::read(VixMntMsgData* msg_data){
     assert(_vixHandle);
     // TODO : write readed buf to mmap
     VixMntOpRead opReadData;
     opReadData.convertFromBytes(msg_data->msg_buff);
    uint64 sizeResult = opReadData.bufsize * VIXDISKLIB_SECTOR_SIZE;

     uint8 buf[sizeResult];

     VixError vixError = read(buf,opReadData.offset,opReadData.bufsize);

     // write buf data for IPC terminal
     //
     _mmap->mntWriteMmap(buf,0,sizeResult);

    VixMntMsgData readMsgResult;
    readMsgResult.msg_op = VixMntOp(MntReadDone);
    readMsgResult.msg_datasize = sizeof(uint64);
    memcpy(readMsgResult.msg_buff,&sizeResult,readMsgResult.msg_datasize);
    VixMntMsgQue* readMsgQ = new VixMntMsgQue(msg_data->msg_response_q);
    readMsgQ->sendMsg(&readMsgResult);

    delete readMsgQ;

    return vixError;

}

VixError
VixMntDiskHandle::write(
        uint8* buf,
        uint64 offset,
        uint64 numberSector)
{

     VixError vixError = VixDiskLib_Write(_vixHandle,offset,numberSector,buf);
     return vixError;
}

VixError
VixMntDiskHandle::write(VixMntMsgData* msg_data){
     // TODO : write writed buf to mmap
     VixMntOpRead opWriteData;
     opWriteData.convertFromBytes(msg_data->msg_buff);
     uint64 sizeResult = opWriteData.bufsize * VIXDISKLIB_SECTOR_SIZE;

     uint8 buf[sizeResult];
     // first read buf data from mmap area
     _mmap->mntReadMmap(buf,0,sizeResult);



     VixError vixError = write(buf,opWriteData.offset,opWriteData.bufsize);

    VixMntMsgData writeMsgResult;
    writeMsgResult.msg_op = VixMntOp(MntWriteDone);
    writeMsgResult.msg_datasize = sizeof(uint64);
    memcpy(writeMsgResult.msg_buff,&sizeResult,writeMsgResult.msg_datasize);
    VixMntMsgQue* writeMsgQ = new VixMntMsgQue(msg_data->msg_response_q);
    writeMsgQ->sendMsg(&writeMsgResult);

    delete writeMsgQ;

    return vixError;
}

VixError
VixMntDiskHandle::getDiskInfo(VixDiskLibInfo **info){
     // TODO :
     //VixDiskLibInfo *info = NULL;
     VixError vixError = VixDiskLib_GetInfo(_vixHandle,info);

     return vixError;

}

void
VixMntDiskHandle::freeDiskInfo(VixDiskLibInfo *info){
     VixDiskLib_FreeInfo(info);
}

std::string VixMntDiskHandle::getErrorMsg(VixError vixError){
    char* msg = VixDiskLib_GetErrorText(vixError,NULL);
    std::string descp = msg;
    VixDiskLib_FreeErrorText(msg);
    return descp;
}
