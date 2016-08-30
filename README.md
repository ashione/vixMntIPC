An IPC tool
===
## simple implementation 
* __Message Queue__
* __Share Memory & Memory Map__
* __Socket with epoll__
* __Simple Log__

## usage ( on \*inx )
* **make** will generate share library
* **make test ** can compile all test cases from src/test to executables in bin/


## Hint
* if you wanto run sample test case, remember to add .so to LD_LIBRARY_PATH


## Resouces 
* vixlink : link source files for whole project in subdirectory root
* _mksg_ : unused now

## Dependency
* vixDiskLib
* fuse
