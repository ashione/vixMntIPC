#include <vixMntFuse.h>
#include <vixMntUtility.h>


int
main(int argc, char* argv[]){
    //ILog("result : %d",VixMntFuseMount("/var/vmare/fuse"));
    VixMntFuseMount("./mnt");
    return 0;
}
