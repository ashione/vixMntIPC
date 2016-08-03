#include <vixMntMmap.h>
#include <vixMntUtility.h>

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <ctime>
#include <exception>

std::string VixMntMmap::fileRoot = "/vmware_mnt_shm";
/*
#ifdef __cplusplus
extern "C" {
#endif
 * VixMntMmap Constructor,
 * share memory will be used when isRoot is True.
 * it opens [datasize/pagesize] +1 frame for share memory or memory map.
 */

/**
 ****************************************************************************
 * VixMntMmap Constructor
 * -------------------------------------------------------------------------
 * input parameters  :
 * mmap_datasize,  allocate a set number of share memory for memory map
 * -------------------------------------------------------------------------
 * output paremeters :
 * -------------------------------------------------------------------------
 * Side Effect:
 * No
 ****************************************************************************
 */

VixMntMmap::VixMntMmap(size_t mmap_datasize, // IN
                       bool isRoot)          // IN
{
   try {
      if (isRoot) {
         this->file_name = VixMntMmap::fileRoot;
      } else
         //   this->file_name = getRandomFileName(fileRoot);
         this->file_name = VixMntMmap::fileRoot;
   } catch (std::exception &e) {
      ELog("set file_name error");
      this->file_name = "/vmware_mnt_shm";
   }

   this->fid =
      shm_open(this->file_name.c_str(), O_RDWR | O_CREAT | O_TRUNC, 0666);

   if (this->fid > 0) {
      ILog("open share memory %d %s", this->fid, this->file_name.c_str());
   } else {
      ILog("open share memory faild, map to file%d", this->fid);
   }

   this->mmap_datasize = mmap_datasize > 0 ? mmap_datasize : MMAP_PAGE_SIZE;
   this->mmap_pagenum = this->mmap_datasize / MMAP_PAGE_SIZE + 1;
   int sh_result = ftruncate(this->fid, this->mmap_pagenum * MMAP_PAGE_SIZE);
   if (sh_result < 0) {
      ELog("shm ftruncate error ");
   }

   if (this->fid == -1) {
      this->mmap_data = (char *)mmap(NULL, this->mmap_pagenum * MMAP_PAGE_SIZE,
                                     PROT_READ | PROT_WRITE,
                                     MAP_SHARED | MAP_ANONYMOUS, -1, 0);
   } else
      this->mmap_data =
         (char *)mmap(NULL, this->mmap_pagenum * MMAP_PAGE_SIZE,
                      PROT_READ | PROT_WRITE, MAP_SHARED, this->fid, 0);

   ILog("shm mmap addr : %x", this->mmap_data);
}

/**
 ****************************************************************************
 * VixMntMmap::mntWriteMmap
 * A packaged function for writing buffer to shared memory.
 * -------------------------------------------------------------------------
 * input parameters  :
 * buf,        data buffer pointer
 * write_pos,  write position ( offset  )
 * write_size, data size
 * -------------------------------------------------------------------------
 * output paremeters :
 * No
 * -------------------------------------------------------------------------
 * Side Effect:
 * No
 ****************************************************************************
 */

void VixMntMmap::mntWriteMmap(const uint8 *buf,  // IN
                              size_t write_pos,  // IN
                              size_t write_size) // IN
{

   memcpy(this->mmap_data + write_pos, buf,
          write_size > 0 ? write_size : this->mmap_datasize);
}

/**
 ****************************************************************************
 * VixMntMmap::mntReadMmap
 * Await for reading data from shared memory.
 * -------------------------------------------------------------------------
 * input parameters  :
 * buf,      data buffer pointer
 * read_pos , read position (offset)
 * read_size, data size
 * -------------------------------------------------------------------------
 * output paremeters :
 * No
 * -------------------------------------------------------------------------
 * Side Effect:
 * No
 ****************************************************************************
 */

void VixMntMmap::mntReadMmap(uint8 *buf,       // IN/OUT
                             size_t read_pos,  // IN
                             size_t read_size) // IN
{
   memcpy(buf, this->mmap_data + read_pos,
          read_size > 0 ? read_size : this->mmap_datasize);
}

/**
 ****************************************************************************
 * VixMntMmap Deconstructor
 * -------------------------------------------------------------------------
 * input parameters  :
 * -------------------------------------------------------------------------
 * output paremeters :
 * -------------------------------------------------------------------------
 * Side Effect:
 * No
 ****************************************************************************
 */

VixMntMmap::~VixMntMmap() {

   munmap(this->mmap_data, this->mmap_pagenum * MMAP_PAGE_SIZE);
   shm_unlink(this->file_name.c_str());
}
