#include <vixMntMmap.h>
#include <cstdlib>
#include <cstdio>
#include <cstring>

VixMntMmap::VixMntMmap(
        size_t mmap_datasize,
        int fid )
{
    this->mmap_datasize = mmap_datasize?mmap_datasize : MMAP_PAGE_SIZE;
    this->mmap_pagenum = this->mmap_datasize/MMAP_PAGE_SIZE + 1;

    if(fid == -1){
        this->mmap_data =(char *) mmap(NULL,
                this->mmap_pagenum * MMAP_PAGE_SIZE,
                PROT_READ | PROT_WRITE,
                MAP_SHARED | MAP_ANONYMOUS,
                -1,0);
    }
    else
        this->mmap_data = (char *) mmap(NULL,
                this->mmap_pagenum * MMAP_PAGE_SIZE,
                PROT_READ | PROT_WRITE,
                MAP_SHARED,
                fid,0);

}

void
VixMntMmap::mntWriteMmap(const char* buf){

    memcpy(this->mmap_data,buf,this->mmap_datasize);

}

void
VixMntMmap::mntReadMmap(char* buf){
    memcpy(buf,this->mmap_data,this->mmap_datasize);
}

VixMntMmap::~VixMntMmap(){

    munmap(this->mmap_data,this->mmap_pagenum * MMAP_PAGE_SIZE);
    //printf("mmap_data : %x\n",this->mmap_data);
    if(fid > 0)
        close(fid);
}
