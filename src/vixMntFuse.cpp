#include <vixMntFuse.h>
#include <vixMntMsgQue.h>
#include <vixMntOperation.h>
#include <vixMntSocket.h>
#include <vixMntUtility.h>

#include <cassert>

// get stanlone msgque instance
static VixMntMsgQue *fuseMsgQue = VixMntMsgQue::getMsgQueInstance();

/**
 ****************************************************************************
 * FuseMntIPC_Read
 * libfuse recall function
 * If memory map & message solution : insert a message into system message
 * queue, then waiting for the result after providing a response message
 * queue name; If socket solution  : setup a socket client to connect socket
 * server, then waiting for returing.
 * -------------------------------------------------------------------------
 * input parameters  :
 * path,
 * size,
 * offset,
 * fi
 * -------------------------------------------------------------------------
 * output parameters :
 * buf
 * -------------------------------------------------------------------------
 * Side Effect:
 * No
 ****************************************************************************
 */

int
FuseMntIPC_Read(const char *path,
                char *buf,
                size_t size,
                off_t offset,
                struct fuse_file_info *fi,
                const uint32 sectorSize)
{
   uint8 IPCType_ = getVixMntIPCType();
   if (IPCType_ == VIXMNTIPC_MMAP) {
      VixMntOpRead opRead(path, size, offset);
      char readMsgQName[32] = {"/readMsgQ"};
      ILog("randomly generate msgQ : %s", readMsgQName);
      VixMntMsgData *opReadMsgData = new VixMntMsgData(
         VixMntOp(MntRead), sizeof opRead, readMsgQName, (char *)&opRead);

      fuseMsgQue->sendMsg(opReadMsgData);

      delete opReadMsgData;

      VixMntMsgQue readMsgQ(readMsgQName);

      VixMntMsgData readMsgResult;
      readMsgQ.receiveMsg(&readMsgResult);
      // the received result data containing result op msg and result size
      if (readMsgResult.msg_op == VixMntOp(MntReadDone)) {
         uint64 sizeResult;
         memcpy(&sizeResult, readMsgResult.msg_buff,
                readMsgResult.msg_datasize);
         vixMntIPC_ReadMmap(buf, 0, sizeResult);
         return sizeResult;
      }
      WLog("addr %u, size %u,read %s error", offset, size, path);

   } else {
      VixMntOpSocket fuseOpSocket(path, size, offset, 0, offset + size,
                                  VixMntOp(MntRead));
      VixMntSocketClient *fuseSocketClient = new VixMntSocketClient();
      fuseSocketClient->rawWrite((char *)(&fuseOpSocket),
                                 sizeof(VixMntOpSocket));
      fuseSocketClient->rawRead(buf, size * sectorSize);
      delete fuseSocketClient;

      return size;
   }
   return 0;
}

/*
 ****************************************************************************
 * FuseMntIPC_Write
 * libfuse recall function,
 * As same as read operation
 * -------------------------------------------------------------------------
 * input parameters  :
 * path,
 * buf,
 * size,
 * offset,
 * fi
 * -------------------------------------------------------------------------
 * output parameters :
 * No
 * -------------------------------------------------------------------------
 * Side Effect:
 * No
 ****************************************************************************
 */

int
FuseMntIPC_Write(const char *path,
                 const char *buf,
                 size_t size,
                 off_t offset,
                 struct fuse_file_info *fi,
                 const uint32 sectorSize)
{
   uint8 IPCType_ = getVixMntIPCType();
   if (IPCType_ == VIXMNTIPC_MMAP) {
      vixMntIPC_WriteMmap(buf, 0, size);

      VixMntOpWrite opWrite(path, size, offset);
      char writeMsgQName[32] = {"/writeMsgQ"};
      // getRandomFileName("/write",0,writeMsgQName);
      ILog("randomly generate msgQ : %s", writeMsgQName);
      VixMntMsgData *opWriteMsgData = new VixMntMsgData(
         VixMntOp(MntWrite), sizeof opWrite, writeMsgQName, (char *)&opWrite);

      fuseMsgQue->sendMsg(opWriteMsgData);

      mq_unlink(writeMsgQName);

      VixMntMsgQue writeMsgQ(writeMsgQName);
      delete opWriteMsgData;

      VixMntMsgData writeMsgResult;
      writeMsgQ.receiveMsg(&writeMsgResult);
      // the received result data containing result op msg and result size
      if (writeMsgResult.msg_op == VixMntOp(MntWriteDone)) {
         uint64 sizeResult;
         memcpy(&sizeResult, writeMsgResult.msg_buff,
                writeMsgResult.msg_datasize);
         return sizeResult;
      }
      WLog("addr %u, size %u,write %s error", offset, size, path);
   } else {

      VixMntOpSocket fuseOpSocket(path, size, offset, 0, offset + size,
                                  VixMntOp(MntWrite));
      VixMntSocketClient *fuseSocketClient = new VixMntSocketClient();
      // socket client send raw data after notification
      fuseSocketClient->rawWrite((char *)(&fuseOpSocket),
                                 sizeof(VixMntOpSocket));
      fuseSocketClient->rawWrite(buf, size * sectorSize);

      delete fuseSocketClient;

      VixMntOpSocket fuseOpSocketResult;
      fuseSocketClient->rawRead((char *)&fuseOpSocketResult,
                                sizeof(VixMntOpSocket));
      if (fuseOpSocketResult.carriedOp == VixMntOp(MntWriteDone)) {
         return size;
      }
   }
   return 0;
}
