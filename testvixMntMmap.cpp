#include <iostream>
#include <vixMntMmap.h>
#include <cstring>

using namespace std;

int main(int args,char** argv){
    int fd;
    //mode_t mode = S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH;
    char *filename = "/tmp/file";
    fd = open(filename, O_RDWR | O_CREAT );

    char msg[]= "testmap";
    //write(fd,"1",1);
    VixMntMmap* testmap = new VixMntMmap(strlen(msg),fd);
    if( strcmp(argv[1],"1" )){
        char* buff = new char[strlen(msg)];
        testmap->mntReadMmap(buff);
        cout<<buff<<endl;
        delete testmap;
        exit(0);
    }
    testmap->mntWriteMmap(msg);
    sleep(10);
    testmap->mntWriteMmap("finished");
    delete testmap;
    return 0;
}
