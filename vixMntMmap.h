#ifndef VIXMNTMMAP_H
#define VIXMNTMMAP_H
#include <sys/mman.h>
#include <sys/types.h>
#include <fcntl.h>
#include <unistd.h>
#include <memory>

#define MMAP_PAGE_SIZE sysconf(_SC_PAGESIZE)

class VixMntMmap{

    public :
        VixMntMmap(size_t mmap_datasize = 0 ,int fid = -1);
        ~VixMntMmap();
        void mntWriteMmap(const char* buf);
        void mntReadMmap(char* buf);

    private :
        int fid;
        size_t mmap_datasize;
        size_t mmap_pagenum;
        char* mmap_data;

};

extern *char
getRandomFileName(){

}
#endif
