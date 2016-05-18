#include <vixMntMmap.h>
#include <cstdlib>
#include <cstdio>
#include <cstring>
#include <ctime>

const std::string VixMntMmap::fileRoot = "/vmware_mnt_shm";

/*
 * VixMntMmap Constructor,
 * share memory will be used when isRoot is True.
 * XXX : it opens [datasize/pagesize] +1 frame for share memory or memory map.
 */

VixMntMmap::VixMntMmap(
        size_t mmap_datasize,
        bool isRoot)
{

    if(isRoot)
        this->file_name = VixMntMmap::fileRoot;
    else
        this->file_name = getRandomFileName(fileRoot);

    //this->fid = open(this->file_name.c_str(), O_RDWR | O_CREAT | O_TRUNC,0666);
    this->fid = shm_open(this->file_name.c_str(), O_RDWR | O_CREAT | O_TRUNC,0666);

    if(this->fid > 0)
        printf("Log : open share memory %d\n",this->fid);
    else
        printf("Log : open share memory faild, map to file%d\n",this->fid);

    this->mmap_datasize = mmap_datasize>0?mmap_datasize : MMAP_PAGE_SIZE;
    this->mmap_pagenum = this->mmap_datasize/MMAP_PAGE_SIZE + 1;
    ftruncate(this->fid,this->mmap_pagenum* MMAP_PAGE_SIZE);

    if(this->fid == -1){
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
                this->fid,0);

}

void
VixMntMmap::mntWriteMmap(
        const char* buf,
        size_t write_pos,
        size_t write_size)
{

    memcpy(this->mmap_data+write_pos,buf,write_size>0?write_size:this->mmap_datasize);

}

void
VixMntMmap::mntReadMmap(
        char* buf,
        size_t read_pos,
        size_t read_size)
{
    memcpy(buf,this->mmap_data+read_pos,read_size>0?read_size:this->mmap_datasize);
}

VixMntMmap::~VixMntMmap(){

    munmap(this->mmap_data,this->mmap_pagenum * MMAP_PAGE_SIZE);
    shm_unlink(this->file_name.c_str());
    //printf("mmap_data : %x\n",this->mmap_data);
    //if(fid > 0)
    //    close(fid);
}


const char* random_str = "0123456789";

std::string
getRandomFileName(std::string rootPath,size_t max_random_len){
    srand((unsigned) time(NULL));

    std::string rfile_name = rootPath;
    for(int i = 0 ; i < max_random_len - 1 ; ++i){
        rfile_name+= random_str[rand()%MMAP_MAX_RANDOM];
    }
    rfile_name += '\0';
    return rfile_name;;
}
