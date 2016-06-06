#ifndef VIXMNTFUSE_H
#define VIXMNTFUSE_H

#define FUSE_USE_VERSION 30
#include <fuse.h>
#include <stdlib.h>

#ifdef __cplusplus
extern "C" {
#endif

#define FAKE_FUSE_PROGRAM_NAME "fusemountIPClib"


//int
//vixmntfusemount(int argc, char* argv[]);

int
VixMntFuseMount(const char*);

void*
FuseMntInit(void){

    return NULL;
};

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
         struct fuse_file_info *fi)
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
         struct fuse_file_info *fi = NULL)
{
    return size;
}

int
FuseMntWrite(
        const char *path,
        const char *buf,
        size_t size,
        off_t offset,
        struct fuse_file_info *fi = NULL)
{
     return size;
}

struct fuse_mntIPC_operations :  fuse_operations
{
    fuse_mntIPC_operations(){
        //init = FuseMntInit;
        getattr = FuseMntGetattr;
        //access = FuseMntAccess;
        //readdir = FuseMntReaddir;
        read = FuseMntRead;
        write = FuseMntWrite;
        fsync = FuseMntFsync;

    }
} fuse_oper;


#ifdef __cplusplus
}
#endif

#endif //VIXMNTMMAP_H

