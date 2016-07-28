#include <vixMntFuse.h>
#include <vixMntMsgQue.h>
#include <vixMntOperation.h>
#include <vixMntSocket.h>
#include <vixMntUtility.h>

#include <cassert>

// for disable warning : char* convert from string
#define const_str(str) const_cast<char *>(str)
// get stanlone msgque instance
static VixMntMsgQue *fuseMsgQue = VixMntMsgQue::getMsgQueInstance();

#ifndef FUSE_DEBUG

/*
 ****************************************************************************
 * VixMntFuseMount
 * mount libfuse interface, deprectaed now
 * -------------------------------------------------------------------------
 * input parameters  :
 * mountpoint, string of system directory
 * -------------------------------------------------------------------------
 * output paremeters :
 * -------------------------------------------------------------------------
 * Side Effect:
 * No
 ****************************************************************************
 */

int VixMntFuseMount(const char *mountpoint) { // IN

   int argc = 7;
   /*
   * usage : mountpoint [-d] [-o xxxx]
   */
   char *argv[] = {
      const_str(FAKE_FUSE_PROGRAM_NAME),
      const_str(FUSE_VAR_DIR),
      const_str("-d"),
      const_str("-o"),
      const_str("allow_other"),
      const_str("-o"),
      const_str("nonempty"),
   };

   // makeDirectoryHierarchy(FUSE_VAR_DIR);
   if (isDirectoryExist(mountpoint)) {
      ILog("mounpoint %s is exist", mountpoint);
   } else {
      ILog("create directory %s", mountpoint);
      makeDirectoryHierarchy(mountpoint);
   }
   // delete for fuse version 25
   // fuse_main(argc,argv,&fuse_oper,NULL);
   fuse_main(argc, argv, &fuse_oper);
   return 0;
}

/*
void*
FuseMntInit(fuse_conn_info* fi){
   return NULL;
}
*/

/*
 ****************************************************************************
 * FuseMntGetattr
 * -------------------------------------------------------------------------
 * input parameters  :
 * path
 * -------------------------------------------------------------------------
 * output paremeters :
 * stbuf
 * -------------------------------------------------------------------------
 * Side Effect:
 * No
 ****************************************************************************
 */

int FuseMntGetattr(const char *path,   // IN
                   struct stat *stbuf) // OUT
{
   return 0;
}

/*
 ****************************************************************************
 * FuseMntAccess
 * -------------------------------------------------------------------------
 * input parameters  :
 * path
 * -------------------------------------------------------------------------
 * output paremeters :
 * mask
 * -------------------------------------------------------------------------
 * Side Effect:
 * No
 ****************************************************************************
 */

int FuseMntAccess(const char *path, int mask) { return 0; }
/*
int
FuseMntReaddir(
      const char*path,
      void *buf,
      fuse_fill_dir_t filler,
      off_t offset,
      struct fuse_file_info *fi,
      fuse_readdir_flags)
{
   return 0;
}
*/

/*
 ****************************************************************************
 * FuseMntFsync
 * -------------------------------------------------------------------------
 * input parameters  :
 * path
 * isdatasync
 * fi
 * -------------------------------------------------------------------------
 * output paremeters :
 * No
 * -------------------------------------------------------------------------
 * Side Effect:
 * No
 ****************************************************************************
 */

int FuseMntFsync(const char *path,          // IN
                 int isdatasync,            // IN
                 struct fuse_file_info *fi) // IN
{
   return 0;
}

#endif

/*
 ****************************************************************************
 * FuseMntIPC_Read
 * libfuse recall function
 * If memory map & message solution : insert a message into
 * system message queue, then waiting for the result after providing a
 * response message queue name;
 * If socket solution  : setup a socket client to connect socket server, then
 * waiting for returing.
 * -------------------------------------------------------------------------
 * input parameters  :
 * path,
 * size,
 * offset,
 * fi
 * -------------------------------------------------------------------------
 * output paremeters :
 * buf
 * -------------------------------------------------------------------------
 * Side Effect:
 * No
 ****************************************************************************
 */

int FuseMntIPC_Read(const char *path,          // IN
                    char *buf,                 // OUT
                    size_t size,               // IN
                    off_t offset,              // IN
                    struct fuse_file_info *fi) // IN
{
   uint8 IPCType_ = getVixMntIPCType();
   if (IPCType_ == VIXMNTIPC_MMAP) {
      VixMntOpRead opRead(path, size, offset);
      char readMsgQName[32] = {"/readMsgQ"};
      // readMsgQName[strlen(readMsgQName)] = '\0';
      // getRandomFileName("/read",0,readMsgQName);
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

         // readMsgQ.unlink();
         // mq_unlink(readMsgQName);

         return sizeResult;
      }
      WLog("addr %u, size %u,read %s error", offset, size, path);

      // readMsgQ.unlink();
      // mq_unlink(readMsgQName);
   } else {
      VixMntOpSocket fuseOpSocket(path, size, offset, 0, offset + size,
                                  VixMntOp(MntRead));
      VixMntSocketClient *fuseSocketClient = new VixMntSocketClient();
      fuseSocketClient->rawWrite((char *)(&fuseOpSocket),
                                 sizeof(VixMntOpSocket));
      fuseSocketClient->rawRead(buf, size * VIXDISKLIB_SECTOR_SIZE);
      // ILog("fuse mnt read %d",nread);
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
 * output paremeters :
 * No
 * -------------------------------------------------------------------------
 * Side Effect:
 * No
 ****************************************************************************
 */

int FuseMntIPC_Write(         // IN
   const char *path,          // IN
   const char *buf,           // IN
   size_t size,               // IN
   off_t offset,              // IN
   struct fuse_file_info *fi) // IN
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
         // writeMsgQ.unlink();
         return sizeResult;
      }
      WLog("addr %u, size %u,write %s error", offset, size, path);
      // writeMsgQ.unlink();
   } else {

      VixMntOpSocket fuseOpSocket(path, size, offset, 0, offset + size,
                                  VixMntOp(MntWrite));
      VixMntSocketClient *fuseSocketClient = new VixMntSocketClient();
      // socket client send raw data after notification
      fuseSocketClient->rawWrite((char *)(&fuseOpSocket),
                                 sizeof(VixMntOpSocket));
      fuseSocketClient->rawWrite(buf, size * VIXDISKLIB_SECTOR_SIZE);

      delete fuseSocketClient;

      VixMntOpSocket fuseOpSocketResult;
      fuseSocketClient->rawRead((char *)&fuseOpSocketResult,
                                sizeof(VixMntOpSocket));
      if (fuseOpSocketResult.carriedOp == VixMntOp(MntWriteDone))
         return size;
   }
   return 0;
}
