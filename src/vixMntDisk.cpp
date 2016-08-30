#include <vixMntDisk.h>
#include <vixMntOperation.h>
#include <vixMntUtility.h>

#include <string>

/**
 * @brief setup a disk object via passing connection.
 *
 * @param connection [in] connection of vixdiskLib
 * @param path [in] diskbaking path
 * @param flag [in] disk operating mode
 */

VixMntDiskHandle::VixMntDiskHandle(VixDiskLibConnection connection,
                                   const char *path,
                                   uint32 flag)
{
   _vixHandle = NULL;
   ILog("open disklib");
   assert(connection);
   VixError vixError = VixDiskLib_Open(connection, path, flag, &_vixHandle);
   diskBackingPath = path;

   SHOW_ERROR_INFO(vixError);
   this->getDiskInfo(&_vixInfo);
}

/**
 * @brief clean disk metadata
 */

VixMntDiskHandle::~VixMntDiskHandle()
{
   VixError vixError = VixDiskLib_Close(_vixHandle);
   SHOW_ERROR_INFO(vixError);
   this->freeDiskInfo(_vixInfo);
   _vixInfo = NULL;
   _vixHandle = NULL;
}


/**
 * @brief prepare for listening function called by libfuse,
 * this msgQ can contract with the other process by sending or receving
 * message, and then write or read buf via mmap.
 *
 * @param msgQ_ [in] system message queue
 * @param mmap_ [in] memory map handle
 */

void
VixMntDiskHandle::prepare(VixMntMsgQue * msgQ_,
                          VixMntMmap * mmap_)
{
   _msgQ = msgQ_;
   _mmap = mmap_;
}

/**
 * @brief call proper function via specific operator message type.
 *
 * @param args [in] pthread passed arguments
 *
 * @return
 */

void *
VixMntDiskHandle::listen(void * args)
{

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
         SHOW_ERROR_INFO(read(&msg_data));
      } else if (msg_op == VixMntOp(MntWrite)) {
         SHOW_ERROR_INFO(write(&msg_data));
      } else {
         ELog("receive exception");
      }
   }

   return NULL;
}

/**
 * @brief vixdisklib read function (read data only sector by sector)
 *
 * @param buf [out]
 * @param offset [in] disk offset
 * @param numberSector [in] buffer sector number of disk
 *
 * @return
 */

VixError
VixMntDiskHandle::read(uint8 *buf,
                       uint64 offset,
                       uint64 numberSector)
{

   VixError vixError = VixDiskLib_Read(_vixHandle, offset, numberSector, buf);
   return vixError;
}

/**
 * @brief read function by passing message
 *
 * @param msg_data [in] message
 *
 * @return
 */

VixError
VixMntDiskHandle::read(VixMntMsgData *msg_data)
{
   assert(_vixHandle);
   VixMntOpRead opReadData;
   opReadData.convertFromBytes(msg_data->msg_buff);
   uint64 sizeResult = opReadData.bufsize * getSectorSize();

   uint8 buf[sizeResult];

   VixError vixError = read(buf, opReadData.offset, opReadData.bufsize);

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


/**
 * @brief vixdisklib write function
 *
 * @param buf [in] buf location
 * @param offset [int] disk offset
 * @param numberSector buffer sector number of disk
 *
 * @return
 */

VixError
VixMntDiskHandle::write(uint8 *buf,
                        uint64 offset,
                        uint64 numberSector)
{

   VixError vixError = VixDiskLib_Write(_vixHandle, offset, numberSector, buf);
   return vixError;
}

/**
 * @brief ifmmap size is less than read buffer size, mmap will throw exception
 *
 * @param msg_data [in] libfuse operator, offset adn data sector size
 *
 * @return
 */

VixError
VixMntDiskHandle::write(VixMntMsgData *msg_data)
{
   VixMntOpRead opWriteData;
   opWriteData.convertFromBytes(msg_data->msg_buff);
   uint64 sizeResult = opWriteData.bufsize * getSectorSize();

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


/**
 * @brief get disk info from opened disk.
 *
 * @param info [in/out]
 *
 * @return
 */

VixError
VixMntDiskHandle::getDiskInfo(VixDiskLibInfo **info)
{

   VixError vixError = VixDiskLib_GetInfo(_vixHandle, info);
   return vixError;
}

/**
 * @brief free disk info pointer
 *
 * @param info [in]
 */

void
VixMntDiskHandle::freeDiskInfo(VixDiskLibInfo *info)
{
   VixDiskLib_FreeInfo(info);
}

/**
 * @brief
 *
 * @param vixError [in]
 *
 * @return message description by string format
 */

std::string
VixMntDiskHandle::getErrorMsg(VixError vixError)
{
   char *msg = VixDiskLib_GetErrorText(vixError, NULL);
   std::string descp = msg;
   VixDiskLib_FreeErrorText(msg);

   return descp;
}
