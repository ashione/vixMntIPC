#ifndef VIXMNTFUSE_H
#define VIXMNTFUSE_H

#define FUSE_USE_VERSION 25
#include <fuse.h>
#include <stdlib.h>
#include <vixDiskLib.h>

#ifdef __cplusplus
extern "C" {
#endif

#define FAKE_FUSE_IPC_PROGRAM_NAME "fusemountIPClib"

#ifndef FAKE_FUSE_PROGRAM_NAME
#define FAKE_FUSE_PROGRAM_NAME "fusemountIPClib"
#endif

#define FUSE_VAR_DIR "/var/run/vmware/fuse"
#define FALT_FILE_FILE_NAME "flat"
#define METADATA_EXTN_STR "info"

/**
 * FuseMnt_IPC_Read, embedded communication function between plugin process
 * and fusedaemon, will be invoked in fusedaemon process.
 */

int FuseMntIPC_Read(const char *path, char *buf, size_t size, off_t offset,
                    struct fuse_file_info *fi);

int FuseMntIPC_Write(const char *path, const char *buf, size_t size,
                     off_t offset, struct fuse_file_info *fi);

#ifdef __cplusplus
}
#endif

#endif // VIXMNTMMAP_H
