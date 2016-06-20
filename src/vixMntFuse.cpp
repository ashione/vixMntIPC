#include <vixMntFuse.h>
#include <vixMntUtility.h>
#include <vixMntOperation.h>
#include <vixMntMsgQue.h>

#include <cassert>

// for disable warning : char* convert from string
#define const_str(str) const_cast<char *>(str)
// get stanlone msgque instance
static VixMntMsgQue* fuseMsgQue = VixMntMsgQue::getMsgQueInstance();

int
VixMntFuseMount(const char *mountpoint){
    int argc = 7;
    /*
     * usage : mountpoint [-d] [-o xxxx]
     */
    char* argv[] = {
        const_str(FAKE_FUSE_PROGRAM_NAME),
        const_str(FUSE_VAR_DIR),
        const_str("-d"),
        const_str("-o"),
        const_str( "allow_other" ),
        const_str( "-o" ),
        const_str( "nonempty" ),
    };

    //makeDirectoryHierarchy(FUSE_VAR_DIR);
    if(isDirectoryExist(mountpoint)){
        ILog("mounpoint %s is exist",mountpoint);
    }
    else{
        ILog("create directory %s",mountpoint);
        makeDirectoryHierarchy(mountpoint);
    }

    fuse_main(argc,argv,&fuse_oper,NULL);
    return 0;
}

void*
FuseMntInit(fuse_conn_info* fi){
    return NULL;
}

int
FuseMntGetattr(
        const char *path,
        struct stat *stbuf)
{
    return 0;
}

int
FuseMntAccess(
        const char *path,
        int mask)
{
     return 0;;
}

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


int
FuseMntFsync(
         const char *path,
         int isdatasync,
         struct fuse_file_info *fi)
{
    return 0;
}

int
FuseMntRead(
         const char *path,
         char *buf,
         size_t size,
         off_t offset,
         struct fuse_file_info *fi )
{
    VixMntOpRead opRead(path,size,offset);
    const char* readMsgQName = getRandomFileName("/read",10);
    VixMntMsgData *opReadMsgData =
#if defined(__cplusplus) && __cplusplus >= 201103L
        new VixMntMsgData(VixMntMsgOp::MntRead,sizeof opRead,readMsgQName,&opRead);
#else
        new VixMntMsgData(MntRead,sizeof opRead,readMsgQName,(char *)&opRead);
#endif
    fuseMsgQue->sendMsg(opReadMsgData);

    delete opReadMsgData;

    VixMntMsgQue readMsgQ(readMsgQName);

    VixMntMsgData readMsgResult;
    readMsgQ.receiveMsg(&readMsgResult);
    // the received result data containing result op msg and result size
#if defined(__cplusplus) && __cplusplus >= 201103L
    if ( readMsgResult.msg_op == VixMntMsgOp::MntReadDone ){
#else
    if ( readMsgResult.msg_op  == MntReadDone ){
#endif
        uint64 sizeResult;
        memcpy(&sizeResult,readMsgResult.msg_buff,readMsgResult.msg_datasize);
        vixMntIPC_ReadMmap(buf,0,sizeResult);

        return sizeResult;
    }
    WLog("addr %u, size %u,read %s error",offset,size,path);
    return 0;
}

int
FuseMntWrite(
        const char *path,
        const char *buf,
        size_t size,
        off_t offset,
        struct fuse_file_info *fi )
{

    vixMntIPC_WriteMmap(buf,0,size);

    VixMntOpWrite opWrite(path,size,offset);
    const char* writeMsgQName = getRandomFileName("/write",10);
    VixMntMsgData *opWriteMsgData =
#if defined(__cplusplus) && __cplusplus >= 201103L
        new VixMntMsgData(VixMntMsgOp::MntWrite,sizeof opWrite,writeMsgQName,&opWrite);
#else
        new VixMntMsgData(MntWrite,sizeof opWrite,writeMsgQName,(char *)&opWrite);
#endif
    fuseMsgQue->sendMsg(opWriteMsgData);

    VixMntMsgQue writeMsgQ(writeMsgQName);
    delete opWriteMsgData;

    VixMntMsgData writeMsgResult;
    writeMsgQ.receiveMsg(&writeMsgResult);
    // the received result data containing result op msg and result size
#if defined(__cplusplus) && __cplusplus >= 201103L
    if ( writeMsgResult.msg_op == VixMntMsgOp::MntWriteDone ){
#else
    if ( writeMsgResult.msg_op  == MntWriteDone ){
#endif
        uint64 sizeResult;
        memcpy(&sizeResult,writeMsgResult.msg_buff,writeMsgResult.msg_datasize);
        return sizeResult;
    }
    WLog("addr %u, size %u,write %s error",offset,size,path);
    return 0;
}

VixError
FuseMnt_DiskLib_Read(
        VixDiskLibHandle vixHandle,
        VixDiskLibSectorType startSector,
        VixDiskLibSectorType numSectors,
        uint8 *readBuffer)
{

    return VixDiskLib_Read(vixHandle,startSector,
                    numSectors,readBuffer);
}

VixError
FuseMnt_DiskLib_Write(
        VixDiskLibHandle vixHandle,
        VixDiskLibSectorType startSector,
        VixDiskLibSectorType numSectors,
        uint8 *readBuffer)
{

    return VixDiskLib_Write(vixHandle,startSector,
                    numSectors,readBuffer);
}
