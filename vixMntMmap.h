#ifndef VIXMNTMMAP_H
#define VIXMNTMMAP_H
#include <sys/mman.h>
#include <sys/types.h>
#include <fcntl.h>
#include <unistd.h>
#include <memory>


class VixMntMmap{

    public :
        VixMntMmap(size_t mmap_datasize = 0 ,int fid = -1);
        void mntWriteMmap(char* buf);
        void mntReadMmap(char* buf);

    private :
        int fid;
        size_t mmap_datasize;
        char* mmap_data;

};

#endif
