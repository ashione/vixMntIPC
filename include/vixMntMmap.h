#ifndef VIXMNTMMAP_H
#define VIXMNTMMAP_H

#include <vixMntUtility.h>

#include <fcntl.h>
#include <memory>
#include <string>
#include <sys/mman.h>
#include <sys/types.h>
#include <unistd.h>

#define MMAP_PAGE_SIZE sysconf(_SC_PAGESIZE)

/*
 * Todo :
 *  fullfill share memory address in multi-process
 */

#ifdef __cplusplus
extern "C" {
#endif

class VixMntMmap {

public:
   VixMntMmap(size_t mmap_datasize = 0, bool isRoot = false);
   VixMntMmap(){};
   ~VixMntMmap();

   // for vixdisklib, convert char* to uint8*

   void mntWriteMmap(const uint8 *buf, size_t write_pos = 0,
                     size_t write_size = 0);
   void mntReadMmap(uint8 *buf, size_t read_pos = 0, size_t read_size = 0);
   // void mntWriteMmap(const char* buf, size_t write_pos= 0,size_t write_size =
   // 0);
   // void mntReadMmap(char* buf, size_t read_pos = 0, size_t read_size = 0);

   inline std::string getMmapFileName() { return this->file_name; }
   inline int getMmapFileId() { return this->fid; }

   inline char *getDataAddr() { return this->mmap_data; }

private:
   int fid;
   size_t mmap_datasize;
   size_t mmap_pagenum;
   char *mmap_data;
   std::string file_name;

public:
   static std::string fileRoot;
};

#ifdef __cplusplus
}
#endif

#endif // end VIXMNTMMAP_H
