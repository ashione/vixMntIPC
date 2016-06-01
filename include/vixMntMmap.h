#ifndef VIXMNTMMAP_H
#define VIXMNTMMAP_H


#include <sys/mman.h>
#include <sys/types.h>
#include <fcntl.h>
#include <unistd.h>
#include <memory>
#include <string>

#define MMAP_MAX_RANDOM 10
#define MMAP_PAGE_SIZE sysconf(_SC_PAGESIZE)

/*
 * Todo :
 *  fullfill share memory address in multi-process
 */

#ifdef __cplusplus
extern "C" {
#endif

class VixMntMmap{

    public :
        VixMntMmap(size_t mmap_datasize = 0 , bool isRoot=false);
        VixMntMmap(){};
        ~VixMntMmap();
        void mntWriteMmap(const char* buf, size_t write_pos= 0,size_t write_size = 0);
        void mntReadMmap(char* buf, size_t read_pos = 0, size_t read_size = 0);

        inline std::string getMmapFileName(){
            return this->file_name;
        }
        inline int getMmapFileId(){
             return this->fid;
        }

        inline char* getDataAddr(){
            return this->mmap_data;
        }

    private :
        int fid;
        size_t mmap_datasize;
        size_t mmap_pagenum;
        char* mmap_data;
        std::string file_name;

    public :
        static std::string fileRoot;


};

std::string
getRandomFileName(
        std::string rootPath,
        size_t max_random_len = MMAP_MAX_RANDOM);

#ifdef __cplusplus
}
#endif

#endif // end VIXMNTMMAP_H
