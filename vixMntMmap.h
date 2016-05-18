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


class VixMntMmap{

    public :
        VixMntMmap(size_t mmap_datasize = 0 , bool isRoot=false);
        ~VixMntMmap();
        void mntWriteMmap(const char* buf);
        void mntReadMmap(char* buf);

        inline std::string getMmapFileName(){
            return this->file_name;
        }
        inline int getMmapFileId(){
             return this->fid;
        }

    private :
        int fid;
        size_t mmap_datasize;
        size_t mmap_pagenum;
        char* mmap_data;
        std::string file_name;

        const static std::string fileRoot;


};

std::string
getRandomFileName(
        std::string rootPath,
        size_t max_random_len = MMAP_MAX_RANDOM);

#endif
