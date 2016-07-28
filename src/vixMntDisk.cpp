#include <vixMntDisk.h>
#include <vixMntOperation.h>
#include <vixMntUtility.h>

#include <string>

/*
 ****************************************************************************
 * VixMntDiskHandle Constructor
 * -------------------------------------------------------------------------
 * input parameters
 * connection, connection of diskLib
 * path, disk path
 * flag, disk operation mode
 * -------------------------------------------------------------------------
 * output paremeters :
 * -------------------------------------------------------------------------
 * Side Effect
 * No
 ****************************************************************************
 */

VixMntDiskHandle::VixMntDiskHandle(VixDiskLibConnection connection, // IN
                                   const char *path,                // IN
                                   uint32 flag) {                   // IN
   _vixHandle = NULL;
   ILog("open disklib");
   VixError vixError = VixDiskLib_Open(connection, path, flag, &_vixHandle);

   SHOW_ERROR_INFO(vixError);
}

/*
 ****************************************************************************
 * VixMntDiskHandle deconstructor
 * -------------------------------------------------------------------------
 * input parameters :
 * No
 * -------------------------------------------------------------------------
 * output paremeters :
 * No
 * -------------------------------------------------------------------------
 * Side Effect
 * No
 ****************************************************************************
 */

VixMntDiskHandle::~VixMntDiskHandle() {
   VixError vixError = VixDiskLib_Close(_vixHandle);
   SHOW_ERROR_INFO(vixError);
   _vixHandle = NULL;
}

/*
 ****************************************************************************
 * VixMntDiskHandle::Prepare
 * prepare for listening function called by libfuse,
 * this msgQ can contract with the other process by sending or receving
 * message, and then write or read buf via mmap
 * -------------------------------------------------------------------------
 * input parameters  :
 * msgQ_, system message queue
 * mmap_, memory map handle
 * -------------------------------------------------------------------------
 * output paremeters :
 * No
 * -------------------------------------------------------------------------
 * Side Effect
 * No
 ****************************************************************************
 */

void VixMntDiskHandle::prepare(VixMntMsgQue *msgQ_, // IN
                               VixMntMmap *mmap_) { // IN
   _msgQ = msgQ_;
   _mmap = mmap_;
}

/*
 ****************************************************************************
 * VixMntDiskHandle::listen
 * call proper function via specific operator message type
 * -------------------------------------------------------------------------
 * input parameters  :
 * args, pthread passed arguments
 * -------------------------------------------------------------------------
 * output paremeters :
 * NULL
 * -------------------------------------------------------------------------
 * Side Effect
 * No
 ****************************************************************************
 */

void *VixMntDiskHandle::listen(void *args) { // IN

   if (!_vixHandle || !_msgQ || !_mmap) {
      ELog("no preparation before listening");
      return (void *)NULL;
   }

   VixMntMsgOp msg_op;
   VixMntMsgData msg_data;

   while (true) {
      _msgQ->receiveMsg(&msg_data);
      msg_op = msg_data.msg_op;

      if (msg_op == VixMntOp(ERROR)) {
         ILog("receive error, breaking");
         break;
      } else if (msg_op == VixMntOp(HALT)) {
         ILog(" stop listening, breaking");
         break;
      } else if (msg_op == VixMntOp(MntRead)) {
         // ILog("receive %s",getOpValue(msg_op));
         SHOW_ERROR_INFO(read(&msg_data));
      } else if (msg_op == VixMntOp(MntWrite)) {
         // ILog("receive %s",getOpValue(msg_op));
         SHOW_ERROR_INFO(write(&msg_data));
      } else {
         ELog("receive exception");
      }
   }

   return NULL;
}

/*
 ****************************************************************************
 * VixMntDiskHandle::read
 * packaged vixdisklib read function
 * read data only via sector by sector
 * -------------------------------------------------------------------------
 * input parameters  :
 * buf, the buf location
 * offset, disk's offset
 * numberSector, wanted buffer sector number of disk
 * -------------------------------------------------------------------------
 * output paremeters :
 * VixError
 * -------------------------------------------------------------------------
 * Side Effect
 * No
 ****************************************************************************
 */

VixError VixMntDiskHandle::read(uint8 *buf,          // IN
                                uint64 offset,       // IN
                                uint64 numberSector) // IN
{

   VixError vixError = VixDiskLib_Read(_vixHandle, offset, numberSector, buf);
   return vixError;
}

/*
 ****************************************************************************
 * FunctionName
 * -------------------------------------------------------------------------
 * input parameters  :
 * msg_data, libfuse operator, offset and data sector size
 * -------------------------------------------------------------------------
 * output paremeters :
 * VixError
 * -------------------------------------------------------------------------
 * Side Effect
 * if mmap size is less than read buffer size, mmap will throw exception
 ****************************************************************************
 */

VixError VixMntDiskHandle::read(VixMntMsgData *msg_data) { // IN
   assert(_vixHandle);
   // TODO : write readed buf to mmap
   VixMntOpRead opReadData;
   opReadData.convertFromBytes(msg_data->msg_buff);
   uint64 sizeResult = opReadData.bufsize * VIXDISKLIB_SECTOR_SIZE;

   uint8 buf[sizeResult];

   VixError vixError = read(buf, opReadData.offset, opReadData.bufsize);

   // write buf data for IPC terminal
   //
   _mmap->mntWriteMmap(buf, 0, sizeResult);

   VixMntMsgData readMsgResult;
   readMsgResult.msg_op = VixMntOp(MntReadDone);
   readMsgResult.msg_datasize = sizeof(uint64);
   memcpy(readMsgResult.msg_buff, &sizeResult, readMsgResult.msg_datasize);
   VixMntMsgQue *readMsgQ = new VixMntMsgQue(msg_data->msg_response_q);
   readMsgQ->sendMsg(&readMsgResult);

   delete readMsgQ;

   return vixError;
}

/*
 ****************************************************************************
 * VixMntDiskHandle::write
 * packaged vixdisklib write function
 * -------------------------------------------------------------------------
 * input parameters  :
 * buf, the buf location
 * offset, disk's offset
 * numberSector, wanted buffer sector number of disk
 * -------------------------------------------------------------------------
 * output paremeters :
 * VixError
 * -------------------------------------------------------------------------
 * Side Effect
 * No
 ****************************************************************************
 */

VixError VixMntDiskHandle::write(uint8 *buf, uint64 offset,
                                 uint64 numberSector) {

   VixError vixError = VixDiskLib_Write(_vixHandle, offset, numberSector, buf);
   return vixError;
}

/*
 ****************************************************************************
 * VixMntDiskHandle::write
 * -------------------------------------------------------------------------
 * input parameters  :
 * msg_data, libfuse operator, offset and data sector size
 * -------------------------------------------------------------------------
 * output paremeters :
 * VixError
 * -------------------------------------------------------------------------
 * Side Effect
 * if mmap size is less than read buffer size, mmap will throw exception
 ****************************************************************************
 */

VixError VixMntDiskHandle::write(VixMntMsgData *msg_data) { // IN
   // TODO : write writed buf to mmap
   VixMntOpRead opWriteData;
   opWriteData.convertFromBytes(msg_data->msg_buff);
   uint64 sizeResult = opWriteData.bufsize * VIXDISKLIB_SECTOR_SIZE;

   uint8 buf[sizeResult];
   // first read buf data from mmap area
   _mmap->mntReadMmap(buf, 0, sizeResult);

   VixError vixError = write(buf, opWriteData.offset, opWriteData.bufsize);

   VixMntMsgData writeMsgResult;
   writeMsgResult.msg_op = VixMntOp(MntWriteDone);
   writeMsgResult.msg_datasize = sizeof(uint64);
   memcpy(writeMsgResult.msg_buff, &sizeResult, writeMsgResult.msg_datasize);
   VixMntMsgQue *writeMsgQ = new VixMntMsgQue(msg_data->msg_response_q);
   writeMsgQ->sendMsg(&writeMsgResult);

   delete writeMsgQ;
   return vixError;
}

/*
 ****************************************************************************
 * VixMntDiskHandle::getDiskInfo
 * packaged VixDiskLib GetInfo function
 * -------------------------------------------------------------------------
 * input parameters  :
 * info
 * -------------------------------------------------------------------------
 * output paremeters :
 * info
 * -------------------------------------------------------------------------
 * Side Effect
 * No
 ****************************************************************************
 */

VixError VixMntDiskHandle::getDiskInfo(VixDiskLibInfo **info) { // IN/OUT

   VixError vixError = VixDiskLib_GetInfo(_vixHandle, info);
   return vixError;
}

/*
 ****************************************************************************
 * VixMntDiskHandle::freeDiskInfo
 * packaged vixdisklib FreeInfo function
 * -------------------------------------------------------------------------
 * input parameters  :
 * info
 * -------------------------------------------------------------------------
 * output paremeters :
 * No
 * -------------------------------------------------------------------------
 * Side Effect:
 * No
 ****************************************************************************
 */

void VixMntDiskHandle::freeDiskInfo(VixDiskLibInfo *info) { // IN

   VixDiskLib_FreeInfo(info);
}

/*
 ****************************************************************************
 * VixMntDiskHandle::getErrorMsg
 * packaged vixdisklib GetErrorText function
 * -------------------------------------------------------------------------
 * input parameters  :
 * vixError
 * -------------------------------------------------------------------------
 * output paremeters :
 * std::string
 * -------------------------------------------------------------------------
 * Side Effect:
 * No
 ****************************************************************************
 */

std::string VixMntDiskHandle::getErrorMsg(VixError vixError) {

   char *msg = VixDiskLib_GetErrorText(vixError, NULL);
   std::string descp = msg;
   VixDiskLib_FreeErrorText(msg);

   return descp;
}
