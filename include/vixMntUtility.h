#ifndef VIXMNTUTILITY_H
#define VIXMNTUTILITY_H

#include <vixDiskLib.h>

#include <pthread.h>
#include <sys/types.h>
#include <unistd.h>


#define MAX_RANDOM_LEN 10
#define STR_RANDOM_NUM_LEN 62
#define MMAP_MEMORY_SIZE 1 << 21

#define VIXMNTIPC_MMAP 0
#define VIXMNTIPC_SOCKET 1

#ifdef __cplusplus
extern "C" {
#endif

#define ILog(format, ...)                                                      \
   vixMntLog(0, getpid(), __LINE__, __func__, __FILE__, format, ##__VA_ARGS__)

#define WLog(format, ...)                                                      \
   vixMntLog(1, getpid(), __LINE__, __func__, __FILE__, format, ##__VA_ARGS__)

#define ELog(format, ...)                                                      \
   vixMntLog(2, getpid(), __LINE__, __func__, __FILE__, format, ##__VA_ARGS__)

#define FLog(format, ...)                                                      \
   vixMntLog(3, getpid(), __LINE__, __func__, __FILE__, format, ##__VA_ARGS__)

int isDirectoryExist(const char *path);

int makeDirectoryHierarchy(const char *path);

void vixMntLog(short level, pid_t pid, int line, const char *func,
               const char *fileName, const char *format, ...);

void getnow(char *buffer);

int vixMntIPC_InitMmap(size_t mmap_datasize, int isRoot);

int vixMntIPC_CleanMmap();

void vixMntIPC_WriteMmap(const char *buf, size_t write_pos, size_t write_size);
void vixMntIPC_ReadMmap(char *buf, size_t read_pos, size_t read_size);

void vixMntIPC_InitMsgQue();

void vixMntIpC_CleanMsgQue();

/**
 * initialization for IPC module
 * use memory map + share memory when IPCType =0
 * otherwise socket server
 */

void vixMntIPC_InitDiskHandle(VixDiskLibConnection connection, const char *path,
                              uint32 flag, uint8 IPCType);

void vixMntIPC_CleanDiskHandle();

VixError vixMntIPC_GetDiskInfo(VixDiskLibInfo **info);

void vixMntIPC_FreeDiskInfo(VixDiskLibInfo *info);

void *vixMntIPC_run(void *);

void getRandomFileName(const char *, size_t max_random_len, char *);

pthread_t vixMntIPC_listen();

pthread_t listening();

int vixMntIPC_main();

uint8 getVixMntIPCType();

unsigned long hashString(unsigned char *str);

unsigned long portMap(unsigned char *str);

uint32 getSectorSize();

#ifdef __cplusplus
}
#endif

#endif // VIXMNTUTILITY_H
