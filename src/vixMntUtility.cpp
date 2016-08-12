#include <vixMntDisk.h>
#include <vixMntMmap.h>
#include <vixMntMsgOp.h>
#include <vixMntMsgQue.h>
#include <vixMntSocket.h>
#include <vixMntUtility.h>

#include <map>
#include <string>
#include <errno.h>
#include <stdarg.h>
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <time.h>
#include <libgen.h>

const char *random_str =
   "0123456789abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ";

static VixMntMmap *mmap_instance = NULL;
static VixMntMsgQue *msgQ_instance = NULL;
static VixMntDiskHandle *diskHandle_instance = NULL;
static uint8 IPCTYPE_FLAG = 0;
static std::map<std::string,VixMntDiskHandle* > diskHandleMap;

static pthread_once_t socket_listen_ponce = PTHREAD_ONCE_INIT;


/**
 * @brief print a specific log information, like [level pid time info]
 *
 * @param level [in] log level
 * @param pid [in] current process id
 * @param line [in] current code line
 * @param func [in] current function name
 * @param fileName [in] curretn source code filename
 * @param format [in] argument fromat
 * @param ...
 */

void
vixMntLog(short level,
          pid_t pid,
          int line,
          const char *func,
          const char *fileName,
          const char *format, ...)
{
   const char *levelStr[] = {"I", "W", "E", "F"};
   char buffLog[] = "[%s%05d %s %s %s:%d] ";
   va_list args;
   va_start(args, format);
   char buffer[0x100];
   char timebuf[80] = "NOW";

   getnow(timebuf);

   snprintf(buffer, 0x100, buffLog, levelStr[level], pid,
            timebuf, basename((char*)fileName), func, line);
   vsnprintf(buffer + strlen(buffer), 80, format, args);

   va_end(args);
   printf("%s\n", buffer);
}


/**
 * @brief get current timestamp.
 *
 * @param buffer
 */

void
getnow(char *buffer)
{

   time_t rawtime;
   struct tm *timeinfo;
   time(&rawtime);
   timeinfo = localtime(&rawtime);
   strftime(buffer, 40, "%F %H:%M:%S", timeinfo);
   struct timeval tp;
   gettimeofday(&tp, NULL);
   float us = tp.tv_usec;
   snprintf(buffer + strlen(buffer), 10, ".%06.f", us);
}


/**
 * @brief exported interface for init mmap.
 *
 * @param mmap_datasize
 * @param isRoot
 *
 * @return
 */

int
vixMntIPC_InitMmap(size_t mmap_datasize, int isRoot)
{
   if (!mmap_instance) {
      mmap_instance = new VixMntMmap(mmap_datasize, isRoot != 0);
   } else {
      WLog("map instance already init");
   }

   return 0;
}


/**
 * @brief clean mmap after unmount disk.
 *
 * @return
 */

int
vixMntIPC_CleanMmap()
{
   if (mmap_instance)
      delete mmap_instance;
   mmap_instance = NULL;
   ILog("unmap instance ok");
   return 0;

   WLog("no instance set up");
   return -1;
}


/**
 * @brief deprecated now, only export to fusemount
 *
 * @param buf
 * @param write_pos
 * @param write_size
 */

void
vixMntIPC_WriteMmap(const char *buf,
                         size_t write_pos,
                         size_t write_size)
{
   if (!mmap_instance) {
      FLog("mmap instance never init");
      return;
   }
   mmap_instance->mntWriteMmap((uint8 *)buf, write_pos, write_size);
}


/**
 * @brief deprecated now, only export to fusemount
 *
 * @param buf
 * @param read_pos
 * @param read_size
 */

void
vixMntIPC_ReadMmap(char *buf,
                   size_t read_pos,
                   size_t read_size)
{
   if (!mmap_instance) {
      FLog("mmap instance never init");
      return;
   }

   mmap_instance->mntReadMmap((uint8 *)buf, read_pos, read_size);
}


/**
 * @brief open system message queue singleton
 */

void
vixMntIPC_InitMsgQue()
{
   if (!msgQ_instance) {
      msgQ_instance = VixMntMsgQue::getMsgQueInstance();
      ILog("msgQ_instance init ok");
   } else {
      ILog("msgQ_instance is already inited");
   }
}


/**
 * @brief  unlink system message queue
 */

void
vixMntIPC_CleanMsgQue()
{
   if (msgQ_instance) {
      VixMntMsgQue::releaseMsgQueInstance();
      ILog("release msgQ_instance");
      msgQ_instance = NULL;
   }
}

/**
 * @brief  diskhandle initialization for IO operation later
 *
 * @param connection [in]
 * @param path [in]
 * @param flag [in]
 * @param IPCType [in] IPC method : socket or message queue
 */

void
vixMntIPC_InitDiskHandle(VixDiskLibConnection connection,
                         const char *path,
                         uint32 flag,
                         uint8 IPCType)
{
   IPCTYPE_FLAG = IPCType;

   if(diskHandleMap.find(path) != diskHandleMap.end()) {
      ELog("disk already opened");
      diskHandle_instance = diskHandleMap[path];
   } else {
      diskHandle_instance = new VixMntDiskHandle(connection, path, flag);
      ILog("init diskname %s",path);
      diskHandleMap[path] = diskHandle_instance;
    }

   if (IPCTYPE_FLAG == VIXMNTIPC_MMAP) {
      vixMntIPC_InitMmap(MMAP_MEMORY_SIZE, 0);
      vixMntIPC_InitMsgQue();
      diskHandle_instance->prepare(msgQ_instance, mmap_instance);
   }
}

/**
 * @brief unlink all message queues and release memory map when the process
 * exit
 */

void
vixMntIPC_CleanDiskHandle()
{
   ILog("Clean DiskHandle");

   if (IPCTYPE_FLAG == VIXMNTIPC_MMAP) {
      delete diskHandle_instance;
      vixMntIPC_CleanMmap();
      msgQ_instance->unlink();
      vixMntIPC_CleanMsgQue();
   } else {
      std::map<std::string,VixMntDiskHandle*>::iterator itr
          = diskHandleMap.begin();
      while( itr!=diskHandleMap.end() ) {
         delete itr->second;
         itr++;
      }
   }
}

/**
 * @brief Encapsulation for vixdisklib getDiskInfo function
 *
 * @param info [out]
 *
 * @return
 */

VixError
vixMntIPC_GetDiskInfo(VixDiskLibInfo **info)
{
   ILog("get vixdisklibinfo ");
   return diskHandle_instance->getDiskInfo(info);
};


/**
 * @brief  Encapsulation for vixDisklib freeDiskInfo function
 *
 * @param info
 */

void
vixMntIPC_FreeDiskInfo(VixDiskLibInfo *info)
{
   ILog("free vixdisklibinfo");
   diskHandle_instance->freeDiskInfo(info);
}


/**
 * @brief deprecated now
 *
 * @param arg [in] pthread arguments
 *
 * @return
 */

void*
vixMntIPC_run(void *arg)
{
   VixMntMsgQue *vixmntmsg = VixMntMsgQue::getMsgQueInstance();
   ILog("tranfer arg to vixHandle");

   pthread_t run_tid = pthread_self();
   while (true) {

      VixMntMsgOp msg_op;
      vixmntmsg->receiveMsgOp(&msg_op);

      if (msg_op == VixMntOp(ERROR)) {
         ILog("thread ID %u, receive error, breaking", run_tid);
      } else if (msg_op == VixMntOp(HALT)) {

         ILog("thread ID %u, stop listening, breaking", run_tid);
         break;

      } else {
         ILog("thread ID %u, receive %s", run_tid, getOpValue(msg_op));
      }
   }
   return NULL;
}


/**
 * @brief setup socket server to bind ip and port only once
 */

void vixMntIPC_listenSocketOnce()
{
   VixMntSocketServer *socketServer_instance = new VixMntSocketServer();
   socketServer_instance->serverListen(diskHandleMap);
}


/**
 * @brief start handle mount disk operations
 *
 * @param args
 *
 * @return
 */

void*
vixMntIPC_listen(void *args)
{
   if (IPCTYPE_FLAG == VIXMNTIPC_MMAP) {
      return diskHandle_instance->listen(args);
   } else {
      pthread_once(&socket_listen_ponce, &vixMntIPC_listenSocketOnce);
      return NULL;
   }
}


/**
 * @brief  start IPC module and creating a new thread to listen
 *
 * @return
 */

int
vixMntIPC_main()
{

   pthread_t pt_id;
   int err = pthread_create(&pt_id, NULL, vixMntIPC_listen, NULL);

   if (err) {
      ELog("can't create thread");
      return 0;
   }

   ILog("thread running, %u", pt_id);
   return pt_id;
}


/**
 * @brief generate a random string when given string prefix and length
 *
 * @param rootPath [in]
 * @param max_random_len [in]
 * @param destination [out]
 */

void
getRandomFileName(const char *rootPath,
                  size_t max_random_len,
                  char *destination)
{

   srand((unsigned)time(NULL));

   std::string rfile_name = rootPath;
   for (size_t i = 0; i < max_random_len - 1; ++i) {
      rfile_name += random_str[rand() % STR_RANDOM_NUM_LEN];
   }
   strncpy(destination, rfile_name.c_str(), rfile_name.size());
}


/**
 * @brief export interface to fusemount
 *
 * @return
 */

uint8
getVixMntIPCType() { return IPCTYPE_FLAG; }

