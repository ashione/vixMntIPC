Implementation of MountApi in Advanced Transport Mode
====
----
## 1. Introduction
Undoubtedly, as we know, the virtual disk API, or VixDiskLib, is a set of function calls to manipulate virtual disk files in VMDK format (virtual machine disk). But VixDisklib only offer certain functions in disk level. Actually many customers want a useful tool to manipulate file without reading or writing whole disk. 

For this reason, we proposed a filesystem level tool named VixMountApi ( as is mountapi )that analyzes disk partition and stored file meta-data via VixDisklib. Then users are able to open or write any files of remote VMDK and enjoy IO operations like local filestystem on linux os.   

Besides, there are additional motivations :

1. The install ratio of linux guest os has been increasing dramatically.
*  With disk capacity growing, the original NBD mode can’t meet the requirements.  * These advanced transport modes realize significantly performance improvements, however these are still not supported in file level.

----
   
### 1.1 Issue

Although mountapi have capability to mount remote VMDK or snapshot via VixDiskLib, 
a [bug][bugid] demonstrated that mount test on advanced transport mode ( such as nbdssl, hotadd and san) had JVM crash on alternate runs at VixMntapi_GetVolumeHandles as VixMntapi_OpenDisks does not return a valid disksetHandle. Some efforts have been utilized to fix this bug, but VixMntapi_OpenDisks still doesn't return a valid disksetHandle. 

We strive to search a constructive way to slove this problem and finally  Fletcher‘s diagnostics may help us to some degree or another.
	
	As I dig into this it's all coming back to me now.  A few years ago I looked into this issue, and I concluded that no matter which transport mode is used, when the plugin is loaded, you cannot use vixMntapi on Linux.  The problem is that vmacore spawns multiple threads to do its job, and these threads to not survive across a fork.  This means that the spawned process is broken and cannot function correctly.  This is why we put a fix into the Linux version of vixMntapi.  The fix causes an error for vixMntapi on Linux for any transport mode including nbd when the plugin is loaded.
	
Based on above investigates, we summarize that 

**the fork subprocess libfuse can't connect vmdk disk when the plugin is loaded because vmacore spawns multiple threads are isolated from [libfuse][libfuse] read and write callback.**

----  
### 1.2 Solutions
Firstly, it's necessary to review original metric and 
entire workflow can be descirbed as ths following image.
![image][mntapi]

From above samplecode workflow, we know libfuse is empolyed by mountapi. (_FUSE (Filesystem in Userspace) is an interface for userspace programs to export a filesystem to the Linux kernel._)  

When advcanced transport mode is applied, vmacore initialize and 
produce multiple threads in vixDiskLib initializition phase 
if dislibplugin is loaded successfully. 
After that, VixDisklib try to open target VMDK and return its vixDiskhandle 
that is indispensable for followed other operations. 
Unfortunately, fusemount will fork a process, 
but father process step into infinite waiting 
because that can't open or connect target VM to initialize libfuse configurations. 
Finally, all of them are stagnating.

According this failure, we drafted three solutions:

1.  open vixdisklib or initalize vmacore spawns multiply threads after fusedaemon. ![union_mode][union_mode]
*   use multithread instead of multiprocess forked by fusemount. ![multithread_mntapi][multithread_mntapi]
*   setup share memory or global message based on IPC ( inner process communication) ![processcom][processcom]


All these solutions look like good designs at first appearance. 
However, postponed vmacore (1) initialization and multiply threads metric (2) 
replacing forked multiply process cannot bear closer analysis. Generally speaking, these disadvantages cancel their possibility as final model:

* vixdisklib programming guideline suggestes users to connect/open VMDK/disk at first. Besides, vixdisklib/vixmntapi only export interfaces to customers rather than change users' pace. ( vs. 1 )
* parameters adjustment of API will make users confused among different versions in short time. (vs. 1)
* libfuse will fork inside, which is not depended on our control.(vs. 2)
* its design limits programmer and imposes more restrictions on extension. (vs. 2)
  
**Hence, solution 3 is chosed finnally.**

---
## 2. Overview of fusemountIPC
As we know, vmacore threads have been created at beginning, 
which make waiting fuse initialization function hanging out here.
To slove above problem, note that we define some class objects :

+ **vixMntDiskHandle** :`It can open a disk and does IO operations by passing diskHandle parameter.
Moreover, a permanent thread will be invoked for listening to system messages.
Then, according different message type, listen thread should call a proper handle function.`
+ **vixMntMmap** : `Packaged share memory for data path by memory map.`
+ **vixMntMsgQue** :`Packaged message queue for control path`
	
+ **vixMntMsgOp** :`Enum class, these message types include MntRead,MntReadDone,MntWrite,MntWriteDone and so on.
	A object of this class will be shipped from sender to receiver by message queue. ` 
	
+ **vixMntOperation** :`Operations consist of read and write parameters needed by libfuse.`
 
+ **vixMntUtility** :`Export interface to bottom layer. ( Initialization, Main handle function, clear function)`

+ **vixMntSocket** :`The other metric differs from the solution about data tranposort by share memory`

+ **vixMntFuse** :`Entry point for libfuse callback.`

+ **vixMntLock&vixMntException** : `Specific lock & exception class`


From the system perspective, we abstract the middleware (fuse daemon)  as core communication layer  from  different modules, for which data control layer is able to focus on message passing. So that new features can be added easily.

By fusemount IPC module, we divide whole work into four principal components and its detail can be depcited as  :


![fuseIPC_timeline][fuseIPC_timeline]	

Frankly speaking, we modify the part of original fusemount module since the passed conncetion is invalid so that it's out of connection in fuse daemon. According this status, read/write operation functions are only design to send a nofitication instead of real disk IO. As shown above figure, a new module are proposed to connect remote disk, do some IO operations, lookup files in mounted disk and so on. 

Additionally, it's worth mentioning that a special class vixMntOperation, described as message protocol, is stored in buffer stream. In other word, a libfuse notification was serialized  as message buffer, then deserialized to class object after receiving notification by IPC module. Actually, both socket and message queue are supported in this module. 

The framwork can be described as follwing graph:
![fusemountIPC][fusemountIPC]

And its flow chart is :

![fuse][fuse]

-----

## 5. Reference 
1.  [bugid] ("https://bugzilla.eng.vmware.com/show_bug.cgi?id=1492312") 
2.  [mntapi]: mntapi.png =600
3.  [libfuse]: https://github.com/libfuse/libfuse
4.  [multithread_mntapi]: multithread.png =600
5.  [union_mode]: union_mode.png =600
6.  [processcom]: process_com.png =600
7.  [fusemountIPC]: fusemountIPC.png =600
[bugid]: https://bugzilla.eng.vmware.com/show_bug.cgi?id=1492312 "mntapibug"
[mntapi]:  https://github.com/ashione/vixMntIPC/blob/format/asset/mntapi.png =600 "mntapi_orin"
[libfuse]: https://github.com/libfuse/libfuse
[multithread_mntapi]: multithread.png =600
[union_mode]: https://github.com/ashione/vixMntIPC/blob/format/asset/union_mode.png =400
[processcom]:  https://github.com/ashione/vixMntIPC/blob/format/asset/process_com.png =600
[fusemountIPC]:  https://github.com/ashione/vixMntIPC/blob/format/asset/fusemountIPC.png =600
[fuse]:  https://github.com/ashione/vixMntIPC/blob/format/asset/fuse.png =600
[fuseIPC_timeline]:  https://github.com/ashione/vixMntIPC/blob/format/asset/fuseIPC_timeline.png =600
 	
