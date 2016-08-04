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

const char *random_str =
   "0123456789abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ";

static VixMntMmap *mmap_instance = NULL;
static VixMntMsgQue *msgQ_instance = NULL;
static VixMntDiskHandle *diskHandle_instance = NULL;
static uint8 IPCTYPE_FLAG = 0;
static std::map<std::string,VixMntDiskHandle* > diskHandleMap;

static pthread_once_t socket_listen_ponce = PTHREAD_ONCE_INIT;

/**
 ****************************************************************************
 * vixMntLog
 * print a specific log information, like [level pid  time info]
 * -------------------------------------------------------------------------
 * input parameters  :
 * level, log level
 * pid, current process id
 * line, current code line
 * func, current function name
 * filename, current source code file name
 * format , argument format
 * -------------------------------------------------------------------------
 * output paremeters :
 * No
 * -------------------------------------------------------------------------
 * Side Effect:
 * No
 ****************************************************************************
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

   snprintf(buffer, 0x100, buffLog, levelStr[level], pid, timebuf, fileName,
            func, line);
   vsnprintf(buffer + strlen(buffer), 80, format, args);

   va_end(args);

   printf("%s\n", buffer);
}

/**
 ****************************************************************************
 * getnow
 * get current timestamp
 * -------------------------------------------------------------------------
 * input parameters  :
 * No
 * -------------------------------------------------------------------------
 * output paremeters :
 * buffer
 * -------------------------------------------------------------------------
 * Side Effect:
 * No
 ****************************************************************************
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
   snprintf(buffer + strlen(buffer), 10, ":%06.f", us);
}

/**
 ****************************************************************************
 * vixMntIPC_initMmap
 * -------------------------------------------------------------------------
 * input parameters  :
 * mmap_datasize
 * isRoot
 * -------------------------------------------------------------------------
 * output paremeters :
 * -------------------------------------------------------------------------
 * Side Effect:
 * No
 ****************************************************************************
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
 ****************************************************************************
 * vixMntIPC_CleanMmap
 * clear memory map
 * -------------------------------------------------------------------------
 * input parameters  :
 * No
 * -------------------------------------------------------------------------
 * output paremeters :
 * No
 * -------------------------------------------------------------------------
 * Side Effect:
 * No
 ****************************************************************************
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
 ****************************************************************************
 * vixMntIPC_WriteMmap
 * deprecated now, only export to fusemount
 * -------------------------------------------------------------------------
 * input parameters  :
 * buf
 * write_pos
 * write_size
 * -------------------------------------------------------------------------
 * output paremeters :
 * No
 * -------------------------------------------------------------------------
 * Side Effect:
 * No
 ****************************************************************************
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
 ****************************************************************************
 * vixMntIPC_ReadMmap
 * deprecated now, only export to fusemount
 * -------------------------------------------------------------------------
 * input parameters  :
 * read_pos
 * read_size
 * -------------------------------------------------------------------------
 * output paremeters :
 * buf
 * -------------------------------------------------------------------------
 * Side Effect:
 * No
 ****************************************************************************
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
 ****************************************************************************
 * vixMntIpc_InitMsgQue
 * open system message queue singleton
 * -------------------------------------------------------------------------
 * input parameters  :
 * No
 * -------------------------------------------------------------------------
 * output paremeters :
 * No
 * -------------------------------------------------------------------------
 * Side Effect:
 * No
 ****************************************************************************
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
 ****************************************************************************
 * vixMntIPC_CleanMsgQue
 * unlink system message queue
 * -------------------------------------------------------------------------
 * input parameters  :
 * No
 * -------------------------------------------------------------------------
 * output paremeters :
 * No
 * -------------------------------------------------------------------------
 * Side Effect:
 * No
 ****************************************************************************
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
 ****************************************************************************
 * vixMntIPC_InitDiskHandle
 * diskhanlde initialization for IO operation later
 * -------------------------------------------------------------------------
 * input parameters  :
 * connection
 * path, unused
 * flag,
 * IPCType, socket or message queue
 * -------------------------------------------------------------------------
 * output paremeters :
 * -------------------------------------------------------------------------
 * Side Effect:
 * No
 ****************************************************************************
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
 ****************************************************************************
 * vixMntIPC_CleanDiskHandle
 * unlink all  message queues and release memory map when the process
 * exit
 * -------------------------------------------------------------------------
 * input parameters  :
 * No
 * -------------------------------------------------------------------------
 * output paremeters :
 * No
 * -------------------------------------------------------------------------
 * Side Effect:
 * No
 ****************************************************************************
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
 ****************************************************************************
 * vixMntIPC_GetDiskInfo
 * Encapsulation for vixdisklib getDiskInfo function
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

VixError
vixMntIPC_GetDiskInfo(VixDiskLibInfo **info)
{
   ILog("get vixdisklibinfo ");
   return diskHandle_instance->getDiskInfo(info);
};

/**
 ****************************************************************************
 * vixMntIPC_FreeDiskInfo
 * Encapsulation for vixdisklib freeDiskInfo function
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

void
vixMntIPC_FreeDiskInfo(VixDiskLibInfo *info)
{
   ILog("free vixdisklibinfo");
   diskHandle_instance->freeDiskInfo(info);
}

/**
 ****************************************************************************
 * VixMntIPC_run
 * deprecated now
 * -------------------------------------------------------------------------
 * input parameters  :
 * arg, pthread arguments
 * -------------------------------------------------------------------------
 * output paremeters :
 * No
 * -------------------------------------------------------------------------
 * Side Effect:
 * No
 ****************************************************************************
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
 ****************************************************************************
 * listening
 * deprecated now
 * -------------------------------------------------------------------------
 * input parameters  :
 * No
 * -------------------------------------------------------------------------
 * output paremeters :
 * No
 * -------------------------------------------------------------------------
 * Side Effect:
 * No
 ****************************************************************************
 */

pthread_t
listening()
{
   pthread_t pt_id;
   int err = pthread_create(&pt_id, NULL, vixMntIPC_run, NULL);

   if (err) {
      ELog("can't create thread");
      return 0;
   }

   ILog("thread running, %u", pt_id);
   return pt_id;
}
/**
 ****************************************************************************
 * vixMntIPC_listenSocketOnce
 * setup sokcet server to bind ip and port only once.
 * -------------------------------------------------------------------------
 * input parameters  :
 * No
 * -------------------------------------------------------------------------
 * output paremeters :
 * No
 * -------------------------------------------------------------------------
 * Side Effect:
 * No
 ****************************************************************************
 */

void vixMntIPC_listenSocketOnce()
{
   VixMntSocketServer *socketServer_instance = new VixMntSocketServer();
   socketServer_instance->serverListen(diskHandleMap);
}

/**
 ****************************************************************************
 * vixMntIPC_listen
 * -------------------------------------------------------------------------
 * input parameters  :
 * args, pthread carried arguments
 * -------------------------------------------------------------------------
 * output paremeters :
 * No
 * -------------------------------------------------------------------------
 * Side Effect:
 * No
 ****************************************************************************
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
 ****************************************************************************
 * vixMntIPC_main
 * starting IPC module and creating a new thread to listen
 * -------------------------------------------------------------------------
 * input parameters  :
 * No
 * -------------------------------------------------------------------------
 * output paremeters :
 * No
 * -------------------------------------------------------------------------
 * Side Effect:
 * No
 ****************************************************************************
 */

int
vixMntIPC_main()
{

   pthread_t pt_id;
   // int err = pthread_create(&pt_id,NULL,vixMntIPC_run,(void *)vixHandle);
   int err = pthread_create(&pt_id, NULL, vixMntIPC_listen, NULL);

   if (err) {
      ELog("can't create thread");
      return 0;
   }

   ILog("thread running, %u", pt_id);
   return pt_id;
}

/**
 ****************************************************************************
 * isDirectoryExist
 * -------------------------------------------------------------------------
 * input parameters  :
 * path
 * -------------------------------------------------------------------------
 * output paremeters :
 * No
 * -------------------------------------------------------------------------
 * Side Effect:
 * No
 ****************************************************************************
 */

int
isDirectoryExist(const char *path)
{
   struct stat info;

   if (stat(path, &info))
      return false;

   return (info.st_mode & S_IFDIR) != 0;
}

/**
 ****************************************************************************
 * makeDirectoryHierarchy
 * create a hierarchy directory
 * -------------------------------------------------------------------------
 * input parameters  :
 * path
 * -------------------------------------------------------------------------
 * output paremeters :
 * No
 * -------------------------------------------------------------------------
 * Side Effect:
 * No
 ****************************************************************************
 */

int
makeDirectoryHierarchy(const char *path)
{
   /*
    * check path was created.
    * If it's not, create whole path by hierarchy
    */
   mode_t mode = 0666;
   std::string spath(path);

   int status = mkdir(spath.c_str(), mode);

   if (!status)
      return true;

   switch (errno) {
   case ENOENT: {
      size_t pos = spath.find_last_of('/');
      if (pos == std::string::npos) {
         pos = spath.find_last_of('\\');
      }

      if (pos == std::string::npos) {
         return false;
      }

      if (!makeDirectoryHierarchy(spath.substr(0, pos).c_str())) {
         return false;
      }

      int status = mkdir(spath.c_str(), mode);
      return !status;
   }
   case EEXIST:
      return isDirectoryExist(path);

   default:
      return false;
   }
}

/**
 ****************************************************************************
 * getRandomFileName
 * generate a random string when given string prefix and length
 * -------------------------------------------------------------------------
 * input parameters  :
 * rootPath,
 * max_random_len
 * -------------------------------------------------------------------------
 * output paremeters :
 * destination
 * -------------------------------------------------------------------------
 * Side Effect:
 * No
 ****************************************************************************
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
 ****************************************************************************
 * getvixMntIPCType
 * export interface to fusemount
 * -------------------------------------------------------------------------
 * input parameters  :
 * No
 * -------------------------------------------------------------------------
 * output paremeters :
 * No
 * -------------------------------------------------------------------------
 * Side Effect:
 * No
 ****************************************************************************
 */

uint8
getVixMntIPCType() { return IPCTYPE_FLAG; }

/**
 ****************************************************************************
 * hashString
 * djb2 hash function,this algorithm (k=33) was first reported by dan
 * bernstein many years ago in comp.lang.c. another version of this
 * algorithm (now favored by bernstein) uses xor:
 * hash(i) = hash(i - 1) * 33 ^ str[i]; the magic of number 33
 * (why it works better than many other constants, prime or not) has never
 * been adequately explained.
 * -------------------------------------------------------------------------
 * input parameters  :
 * str
 * -------------------------------------------------------------------------
 * output paremeters :
 * unsigned long , hash number
 * -------------------------------------------------------------------------
 * Side Effect:
 * No
 ****************************************************************************
 */

unsigned long
hashString(unsigned char *str)
{
   unsigned long hash = 5381;
   int c;
   while ( (c = *str++) ){
      hash = ((hash << 5) + hash) + hash + c;
   }
   return hash;
}

/**
 ****************************************************************************
 * portMap
 * map diskname to a port
 * -------------------------------------------------------------------------
 * input parameters  :
 * str
 * -------------------------------------------------------------------------
 * output paremeters :
 * unsigned long , hash number
 * -------------------------------------------------------------------------
 * Side Effect:
 * No
 ****************************************************************************
 */

unsigned long
portMap(unsigned char *str)
{
   unsigned long port = hashString(str);
   return (port%20000)+41413;
}
