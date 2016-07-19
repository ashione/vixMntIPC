#ifndef VIXMNTSOCKET_H

#include <vixMntUtility.h>
#include <vixMntDisk.h>
#include <vixMntOperation.h>

#include <map>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

#include <netinet/in.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <sys/epoll.h>

#ifdef __cplusplus
extern "C"{
#endif

#define SOCKET_IPADDRESS   "127.0.0.1"
#define SOCKET_PORT      45541
#define SOCKET_BUF_MAX_SIZE (1<<20)
#define SOCKET_FD_MAX_SIZE  1000
#define SOCKET_EPOLLEVENTS  100
#define SOCKET_LISTENQ    50

class VixMntSocket{
   public :
     VixMntSocket(int epollfd_ = -1): epollfd(epollfd_){};
     virtual ~VixMntSocket(){ if(epollfd>0) close(epollfd); }

   protected :
     virtual void doRead(int fd,char* buf,int maxLen = 0) = 0;
     virtual void doWrite(int fd,char* buf,int maxLen = 0) = 0;
     virtual void handleEvents(epoll_event*,int,char*) = 0;
     virtual int rawRead(int fd,char* buf, int maxLen);
     virtual int rawWrite(int fd,const char* buf, int maxLen);

     void addEvent(int fd,int state);
     void deleteEvent(int fd,int state);
     void modifyEvent(int fd,int state);

   protected :
     int epollfd;

};

class VixMntSocketServer : public VixMntSocket{

   public :
     VixMntSocketServer();
     ~VixMntSocketServer();

     void serverListen(VixMntDiskHandle*);
   private :
     void doEpoll();
     void handleEvents(epoll_event*,int,char*);
     void handleAccept();
     void doRead(int fd,char* buf,int maxLen = 0);
     void doWrite(int fd,char* buf,int maxLen = 0);

     /*
     void addEvent(int fd,int state);
     void deleteEvent(int fd,int state);
     void modifyEvent(int fd,int state);
     */

   private :
     int listenfd;
     VixMntDiskHandle* vixdh;
     std::map<int,uint64> clientMap4Write;

};

class VixMntSocketClient : public VixMntSocket{
   public :
     VixMntSocketClient();
     ~VixMntSocketClient();

     void clientConnect();
     int rawRead(char *buf,int bufsize);
     int rawWrite(const char *buf,int bufsize);

   private :
     void handleConnect();
     void handleEvents(epoll_event*,int,char*);
     void doRead(int fd,char* buf,int maxLen = 0);
     void doWrite(int fd,char* buf,int maxLen = 0);

   private :
     int sockfd;

};

#ifdef __cplusplus
}
#endif

#endif // VIXMNTSOCKET_H
