#include <vixMntMmap.h>
#include <cstdlib>
#include <cstdio>
#include <cstring>

VixMntMmap::VixMntMmap(
        size_t mmap_datasize,
        int fid )
{
    this->mmap_datasize = mmap_datasize?mmap_datasize : sysconf(_SC_PAGESIZE);

    if(fid != -1){
        this->mmap_data =(char *) mmap(NULL,
                this->mmap_datasize,
                PROT_READ | PROT_WRITE,
                MAP_SHARED | MAP_ANONYMOUS,
                -1,0);
    }
    else
        this->mmap_data = (char *) mmap(NULL,
                this->mmap_datasize,
                PROT_READ | PROT_WRITE,
                MAP_SHARED,
                fid,0);

}

void
VixMntMmap::mntWriteMmap(char* buf){

    memcpy(this->mmap_data,buf,this->mmap_datasize);

}

void
VixMntMmap::mntReadMmap(char* buf){
    memcpy(buf,this->mmap_data,this->mmap_datasize);
}

