#ifndef VIXMNTFUSE_H
#define VIXMNTFUSE_H


#define FUSE_USE_VERSION 30
#include <fuse.h>
#include <stdlib.h>
#include <vixDiskLib.h>

#ifdef __cplusplus
extern "C" {
#endif

#define FAKE_FUSE_PROGRAM_NAME  "fusemountIPClib"
#define FUSE_VAR_DIR            "/var/run/vmware/fuse"
#define FALT_FILE_FILE_NAME     "flat"
#define METADATA_EXTN_STR       "info"


int
VixMntFuseMount(const char*);

void*
FuseMntInit(fuse_conn_info*);

int
FuseMntGetattr(
        const char *path,
        struct stat *stbuf);

int
FuseMntAccess(
        const char *path,
        int mask);

int
FuseMntReaddir(
         const char*path,
         void *buf,
         fuse_fill_dir_t filler,
         off_t offset,
         struct fuse_file_info *fi,
         fuse_readdir_flags);

int
FuseMntFsync(
         const char *path,
         int isdatasync,
         struct fuse_file_info *fi);

int
FuseMntRead(
         const char *path,
         char *buf,
         size_t size,
         off_t offset,
         struct fuse_file_info *fi = NULL);

int
FuseMntWrite(
        const char *path,
        const char *buf,
        size_t size,
        off_t offset,
        struct fuse_file_info *fi = NULL);

struct fuse_mntIPC_operations :  fuse_operations
{
    fuse_mntIPC_operations(){
        init = FuseMntInit;
        getattr = FuseMntGetattr;
        access = FuseMntAccess;
        readdir = FuseMntReaddir;
        read = FuseMntRead;
        write = FuseMntWrite;
        fsync = FuseMntFsync;

    }
} fuse_oper;


#ifdef __cplusplus
}
#endif

#endif //VIXMNTMMAP_H

