#include <vixMntFuse.h>
#include <vixMntUtility.h>

#include <cassert>

int
VixMntFuseMount(const char *mountpoint){
    int argc = 7;
    /*
     * usage : mountpoint [-d] [-o xxxx]
     */
    char* argv[] = {
        FAKE_FUSE_PROGRAM_NAME,
        FUSE_VAR_DIR,
        "-d",
        "-o",
        "allow_other",
        "-o",
        "nonempty",
    };

    //makeDirectoryHierarchy(FUSE_VAR_DIR);
    if(isDirectoryExist(mountpoint)){
        ILog("mounpoint %s is exist",mountpoint);
    }
    else{
        ILog("create directory %s",mountpoint);
        makeDirectoryHierarchy(mountpoint);
    }
/*
    struct fuse *fuse;
    int res;
    char *opt = NULL;
    struct fuse_args args = FUSE_ARGS_INIT(0,NULL);

    res = fuse_opt_add_opt(&opt,"allow_other");
    assert(!res);

    // need read from config file
    res = fuse_opt_add_opt(&opt,"debug");
    assert(!res);

    res = fuse_opt_add_opt(&opt,"ro");
    assert(!res);

    res = fuse_opt_add_opt(&opt,"nonempty");
    assert(!res);

    ILog("opt : %s",opt);

    res = fuse_opt_add_arg(&args,FAKE_FUSE_PROGRAM_NAME);
    assert(!res);

    res = fuse_opt_add_arg(&args,"-o");
    assert(!res);

    res = fuse_opt_add_args(&args, opt);
    assert(!res);
    free(opt);

    fuse_chan *fd = fuse_mount(mountpoint, &args);

    fuse =fuse_new(fd, &args, &fuse_oper, sizeof fuse_oper);
*/
    fuse_main(argc,argv,&fuse_oper,NULL);
    return 0;
}

void*
FuseMntInit(fuse_conn_info* fi){
    return NULL;
}
VixError
FuseMnt_DiskLib_Read(
        VixDiskLibHandle vixHandle,
        VixDiskLibSectorType startSector,
        VixDiskLibSectorType numSectors,
        uint8 *readBuffer)
{

    return VixDiskLib_Read(vixHandle,startSector,
                    numSectors,readBuffer);
}

VixError
FuseMnt_DiskLib_Write(
        VixDiskLibHandle vixHandle,
        VixDiskLibSectorType startSector,
        VixDiskLibSectorType numSectors,
        uint8 *readBuffer)
{

    return VixDiskLib_Write(vixHandle,startSector,
                    numSectors,readBuffer);
}
