#include <vixMntOperation.h>
#include <vixMntSocket.h>

/**
 ****************************************************************************
 * VixMntSocketServer Constructor
 * bind a given port on localhost, it will exit whenever bind error.
 * -------------------------------------------------------------------------
 * input parameters
 * No
 * -------------------------------------------------------------------------
 * output paremeters :
 * No
 * -------------------------------------------------------------------------
 * Side Effect
 * No
 ****************************************************************************
 */

VixMntSocketServer::VixMntSocketServer() : VixMntSocket()
{
   sockaddr_in servaddr;
   listenfd = socket(AF_INET, SOCK_STREAM, 0);

   if (listenfd == -1) {
      ELog("socket error");
      exit(1);
   }

   bzero(&servaddr, sizeof(servaddr));
   servaddr.sin_family = AF_INET;
   inet_pton(AF_INET, SOCKET_IPADDRESS, &servaddr.sin_addr);
   servaddr.sin_port = htons(SOCKET_PORT);

   if (bind(listenfd, (sockaddr *)&servaddr, sizeof(servaddr)) == -1) {
      ELog("bind error");
      exit(1);
   }
   ILog("Server Start.");
}

VixMntSocketServer::~VixMntSocketServer() { close(listenfd); }

/**
 ****************************************************************************
 * VixMntSocketServer:;serverListen
 * starting server listening and enable epoll for socket fd
 * -------------------------------------------------------------------------
 * input parameters
 * vixdh
 * -------------------------------------------------------------------------
 * output paremeters :
 * No
 * -------------------------------------------------------------------------
 * Side Effect
 * No
 ****************************************************************************
 */

void
VixMntSocketServer::serverListen(
        std::map<std::string,VixMntDiskHandle *> &vixdh)
{
   if (listen(listenfd, SOCKET_LISTENQ)) {
      ELog("Server Listen Error, %s", strerror(errno));
   } else {
      ILog("Server Listen Start");
      this->vixdhMap = &vixdh;
      doEpoll();
   }
}

/**
 ****************************************************************************
 * VixMntSocketServer::doEpoll
 * epoll starting
 * -------------------------------------------------------------------------
 * input parameters
 * No
 * -------------------------------------------------------------------------
 * output paremeters :
 * No
 * -------------------------------------------------------------------------
 * Side Effect
 * No
 ****************************************************************************
 */

void
VixMntSocketServer::doEpoll()
{
   epoll_event events[SOCKET_EPOLLEVENTS];
   int ret;
   char buf[SOCKET_BUF_MAX_SIZE];
   memset(buf, 0, SOCKET_BUF_MAX_SIZE);
   epollfd = epoll_create(SOCKET_FD_MAX_SIZE);
   addEvent(listenfd, EPOLLIN);
   while (true) {

      ret = epoll_wait(epollfd, events, SOCKET_EPOLLEVENTS, -1);

      handleEvents(events, ret, buf);
      if (ret <= 0) {
         return;
      }
   }
   close(epollfd);
}

/**
 ****************************************************************************
 * VixMntSocketServer::handleEvents
 * handle different events in different functions / operations
 * -------------------------------------------------------------------------
 * input parameters
 * events
 * num, the number of events
 * buf
 * -------------------------------------------------------------------------
 * output paremeters :
 * -------------------------------------------------------------------------
 * Side Effect
 * No
 ****************************************************************************
 */

void
VixMntSocketServer::handleEvents(epoll_event *events,
                                 int num,
                                 char *buf)
{

   for (int i = 0; i < num; i++) {
      int fd = events[i].data.fd;

      if (fd == listenfd && (events[i].events & EPOLLIN))
         handleAccept();
      else if (events[i].events & EPOLLIN)
         doRead(fd, buf);
      else if (events[i].events & EPOLLOUT)
         doWrite(fd, buf);
   }
}

/**
 ****************************************************************************
 * vixMntSocketServer::handleAccept
 * accpet a new socket client
 * -------------------------------------------------------------------------
 * input parameters :
 * No
 * -------------------------------------------------------------------------
 * output paremeters :
 * No
 * -------------------------------------------------------------------------
 * Side Effect
 * No
 ****************************************************************************
 */

void
VixMntSocketServer::handleAccept()
{

   int clientFd;
   sockaddr_in clientAddr;
   socklen_t clientAddrLen;
   clientFd = accept(listenfd, (sockaddr *)&clientAddr, &clientAddrLen);

   if (clientFd == -1) {
      ELog("Accept Error");

   } else {
      /**
       * ILog("Accept a new client : %s : %d", inet_ntoa(clientAddr.sin_addr),
           clientAddr.sin_port);
      */
      addEvent(clientFd, EPOLLIN);
      clientMap4Write[clientFd] = 0;
   }
}

/**
 ****************************************************************************
 * VixMntSocketServer::doRead
 * parser control data, then read data from remote disk via vixmntdiskhandle.
 * if realted filename(diskname) not exist in vixdhMap, no handle will be
 * response.
 * -------------------------------------------------------------------------
 * input parameters :
 * fd
 * buf
 * maxLen, deprecated now
 * -------------------------------------------------------------------------
 * output paremeters :
 * No
 * -------------------------------------------------------------------------
 * Side Effect
 * No
 ****************************************************************************
 */

void VixMntSocketServer::doRead(int fd,
                                char *buf,
                                int maxLen)
{

   maxLen = maxLen > 0 ? maxLen : sizeof(VixMntOpSocket);
   int nread = recv(fd, buf, maxLen, 0);
   if (nread == -1) {
      ELog("Read Error");
      close(fd);
   } else if (nread == 0) {
      clientMap4Write.erase(fd);
      close(fd);
      deleteEvent(fd, EPOLLIN);
   } else {
      VixMntOpSocket vixskop;
      vixskop.convertFromBytes(buf);
      std::string diskHandleName = vixskop.fileName;
      //ILog("operation diskname %s",vixskop.fileName);
      if (vixskop.carriedOp == VixMntOp(MntRead)) {
         uint64 maxOps =
            vixskop.bufsize * VIXDISKLIB_SECTOR_SIZE / SOCKET_BUF_MAX_SIZE;
         uint64 eachSectorSize = SOCKET_BUF_MAX_SIZE / VIXDISKLIB_SECTOR_SIZE;
         uint32 i = 0;
         for (; i < maxOps; ++i) {

            VixError vixError =
               (*vixdhMap)[diskHandleName]->read(
                       (uint8 *)buf,
                       vixskop.offset + i * eachSectorSize,
                       eachSectorSize);
            SHOW_ERROR_INFO(vixError);
            int nwrite = rawWrite(fd, buf, SOCKET_BUF_MAX_SIZE);

            if (nwrite <= 0) {
               ELog("Batch Write Error.");
            }
         }
         uint64 leftSector = vixskop.bufsize - i * eachSectorSize;

         if (leftSector > 0) {

            /* mark client needed buffer size for next write operation*/
            VixError vixError = (*vixdhMap)[diskHandleName]->read(
               (uint8 *)buf, vixskop.offset + i * eachSectorSize, leftSector);
            SHOW_ERROR_INFO(vixError);
            int nwrite = rawWrite(fd, buf, leftSector * VIXDISKLIB_SECTOR_SIZE);

            if (nwrite <= 0) {
               ELog("Reminded Write Error.");
            }
         }
      } else if (vixskop.carriedOp == VixMntOp(MntWrite)) {
         // continue to read reminded buf
         int ndataread = recv(fd, buf, vixskop.carriedBufSize, 0);
         vixskop.carriedBufSize = 0;

         if (ndataread == -1) {
            ELog("Read Error");
            close(fd);
         } else if (ndataread == 0) {
            ILog("Client Close");
            clientMap4Write.erase(fd);
            close(fd);
            deleteEvent(fd, EPOLLIN);
         } else {
            VixError vixError =
               (*vixdhMap)[diskHandleName]->write(
                       (uint8 *)buf,
                       vixskop.offset,
                       vixskop.bufsize);
            SHOW_ERROR_INFO(vixError);
            vixskop.carriedOp = VixMntOp(MntWriteDone);
            ILog("Write offset %u , bufsize %u,token %u", vixskop.offset,
                 vixskop.bufsize, vixskop.token);

            // nofity socket client write successful
            memcpy(buf, (char *)&vixskop, sizeof(vixskop));
            clientMap4Write[fd] = sizeof(vixskop);
            modifyEvent(fd, EPOLLOUT);
         }

      } else {
         ELog("Error");
      }
   }
}

/**
 ****************************************************************************
 * VixMntSocketServer::doWrite
 * deprecated now if read size is bigger than SOCKET_BUF_MAX_SIZE
 * -------------------------------------------------------------------------
 * input parameters :
 * fd
 * buf
 * maxLen
 * -------------------------------------------------------------------------
 * output paremeters :
 * -------------------------------------------------------------------------
 * Side Effect
 * No
 ****************************************************************************
 */

void
VixMntSocketServer::doWrite(int fd,
                            char *buf,
                            int maxLen)
{

   ILog("dowrite fd : %d, maxLen : %d", fd, clientMap4Write[fd]);
   int nwrite = rawWrite(fd, buf, clientMap4Write[fd]);
   if (nwrite == -1) {
      ELog("Write Error");
      close(fd);
      deleteEvent(fd, EPOLLOUT);
   } else
      modifyEvent(fd, EPOLLIN);
}

/**
 ****************************************************************************
 * VixMntSocket::addEvent
 * add event for socket epoll
 * -------------------------------------------------------------------------
 * input parameters :
 * fd
 * state
 * -------------------------------------------------------------------------
 * output paremeters :
 * No
 * -------------------------------------------------------------------------
 * Side Effect
 * No
 ****************************************************************************
 */

void
VixMntSocket::addEvent(int fd,
                       int state)
{

   epoll_event ev;
   ev.events = state;
   ev.data.fd = fd;
   epoll_ctl(epollfd, EPOLL_CTL_ADD, fd, &ev);
}

/**
 ****************************************************************************
 * VixMntSocket::deleteEvent
 * -------------------------------------------------------------------------
 * input parameters :
 * fd
 * state
 * -------------------------------------------------------------------------
 * output paremeters :
 * No
 * -------------------------------------------------------------------------
 * Side Effect
 * No
 ****************************************************************************
 */

void
VixMntSocket::deleteEvent(int fd,
                          int state)
{

   epoll_event ev;
   ev.events = state;
   ev.data.fd = fd;
   epoll_ctl(epollfd, EPOLL_CTL_DEL, fd, &ev);
}

/**
 ****************************************************************************
 * VixMntSocket::modifyEvent
 * -------------------------------------------------------------------------
 * input parameters  :
 * fd
 * state
 * -------------------------------------------------------------------------
 * output paremeters :
 * No
 * -------------------------------------------------------------------------
 * Side Effect
 * No
 ****************************************************************************
 */

void
VixMntSocket::modifyEvent(int fd,
                          int state)
{

   epoll_event ev;
   ev.events = state;
   ev.data.fd = fd;
   epoll_ctl(epollfd, EPOLL_CTL_MOD, fd, &ev);
}

/**
 ****************************************************************************
 * VixMntSocket::rawRead
 * read data directly, but it will keep reading until recived data length
 * is equal to buffer size
 * -------------------------------------------------------------------------
 * input parameters  :
 * fd
 * bufsize
 * -------------------------------------------------------------------------
 * output paremeters :
 * buf
 * -------------------------------------------------------------------------
 * Side Effect
 * No
 ****************************************************************************
 */

int
VixMntSocket::rawRead(int fd,
                      char *buf,
                      int bufsize)
{
   if (bufsize <= 0)
      return bufsize;

   int recvSize = read(fd, buf, bufsize);

   if (recvSize == 0) {
      ELog("Server close");
   } else if (recvSize == 0) {
      ELog("Send Error");
   } else
      while (recvSize < bufsize) {
         int tempRecvSize = read(fd, buf + recvSize, bufsize - recvSize);
         recvSize += tempRecvSize;
      }
   return recvSize;
}

/**
 ****************************************************************************
 * VixMntSocket::rawWrite
 * keep writing until all data are sent out
 * -------------------------------------------------------------------------
 * input parameters  :
 * fd
 * buf
 * bufsize
 * -------------------------------------------------------------------------
 * output paremeters :
 * -------------------------------------------------------------------------
 * Side Effect
 * No
 ****************************************************************************
 */

int
VixMntSocket::rawWrite(int fd,
                       const char *buf,
                       int bufsize)
{
   // return write(sockfd,buf,bufsize);
   if (bufsize > 0) {
      int sendSize = write(fd, buf, bufsize);
      if (sendSize == 0) {
         ELog("Server close");
         // close(sockfd);
      } else if (sendSize == 0) {
         ELog("Send Error");
         // close(sockfd);
      } else {
         while (sendSize < bufsize) {
            int tempSendSize = write(fd, buf + sendSize, bufsize - sendSize);
            sendSize += tempSendSize;
         }
      }
   }
   return bufsize;
}

/**
 ****************************************************************************
 * VixMntSocketClient Constructor
 * connect socket server automatically
 * -------------------------------------------------------------------------
 * input parameters  :
 * No
 * -------------------------------------------------------------------------
 * output paremeters :
 * No
 * -------------------------------------------------------------------------
 * Side Effect
 * No
 ****************************************************************************
 */

VixMntSocketClient::VixMntSocketClient() : VixMntSocket()
{
   sockfd = socket(AF_INET, SOCK_STREAM, 0);
   sockaddr_in servaddr;
   bzero(&servaddr, sizeof(servaddr));
   servaddr.sin_family = AF_INET;
   servaddr.sin_port = htons(SOCKET_PORT);
   inet_pton(AF_INET, SOCKET_IPADDRESS, &servaddr.sin_addr);
   if (connect(sockfd, (sockaddr *)&servaddr, sizeof(servaddr))) {
      ELog(" Connect Error, %s", strerror(errno));
   }
}

/**
 ****************************************************************************
 * VixMntSocketClient Deconstructor
 * -------------------------------------------------------------------------
 * input parameters  :
 * No
 * -------------------------------------------------------------------------
 * output paremeters :
 * No
 * -------------------------------------------------------------------------
 * Side Effect
 * No
 ****************************************************************************
 */

VixMntSocketClient::~VixMntSocketClient() { close(sockfd); }

/**
 ****************************************************************************
 * VixMntSocketClient::doRead
 * unimplemented
 * -------------------------------------------------------------------------
 * input parameters  :
 * No
 * -------------------------------------------------------------------------
 * output paremeters :
 * No
 * -------------------------------------------------------------------------
 * Side Effect
 * No
 ****************************************************************************
 */

void
VixMntSocketClient::doRead(int fd,
                           char *buf,
                           int maxLen)
{
   // TODO
}

/**
 ****************************************************************************
 * VixMntSocketClinet::doWrite
 * unimplemented
 * -------------------------------------------------------------------------
 * input parameters  :
 * No
 * -------------------------------------------------------------------------
 * output paremeters :
 * No
 * -------------------------------------------------------------------------
 * Side Effect
 * No
 ****************************************************************************
 */

void
VixMntSocketClient::doWrite(int fd,
                            char *buf,
                            int maxLen)
{
   // TODO
}

/**
 ****************************************************************************
 * VixMntSocketCLient::handleConnect
 * deprecated now
 * -------------------------------------------------------------------------
 * input parameters  :
 * No
 * -------------------------------------------------------------------------
 * output paremeters :
 * No
 * -------------------------------------------------------------------------
 * Side Effect
 * No
 ****************************************************************************
 */

void
VixMntSocketClient::handleConnect()
{

   epoll_event events[SOCKET_EPOLLEVENTS];
   char buf[SOCKET_BUF_MAX_SIZE];
   int ret;
   memset(buf, 0, SOCKET_BUF_MAX_SIZE);
   epollfd = epoll_create(SOCKET_FD_MAX_SIZE);
   addEvent(STDIN_FILENO, EPOLLIN);

   while (true) {
      ret = epoll_wait(epollfd, events, SOCKET_EPOLLEVENTS, -1);
      handleEvents(events, ret, buf);

      if (ret <= 0) {
         return;
      }
   }

   close(epollfd);
}

/**
 ****************************************************************************
 * VixMntSocketClient:handleEvents
 * work in testing, deprecated now
 * -------------------------------------------------------------------------
 * input parameters  :
 * events
 * num, the number of events
 * buf
 * -------------------------------------------------------------------------
 * output paremeters :
 * No
 * -------------------------------------------------------------------------
 * Side Effect
 * No
 ****************************************************************************
 */

void
VixMntSocketClient::handleEvents(epoll_event *events,
                                 int num,
                                 char *buf)
{

   for (int i = 0; i < num; ++i) {
      int fd = events[i].data.fd;
      if (events[i].events & EPOLLIN) {
         doRead(fd, buf);
      } else if (events[i].events & EPOLLOUT) {
         doWrite(fd, buf);
      }
   }
}

/**
 ****************************************************************************
 * VixMntSocketClient::rawRead
 * class super class rawRead directly
 * -------------------------------------------------------------------------
 * input parameters  :
 * bufsize
 * -------------------------------------------------------------------------
 * output paremeters :
 * buf
 * -------------------------------------------------------------------------
 * Side Effect
 * No
 ****************************************************************************
 */

/**
 * read whole needed sectors
 * if not do this, client receiver will get incomplete data
 */

int
VixMntSocketClient::rawRead(char *buf,
                            int bufsize)
{
   return VixMntSocket::rawRead(sockfd, buf, bufsize);
}

/**
 ****************************************************************************
 * VixMntSocketClient::rawWrite
 * call super class directly
 * -------------------------------------------------------------------------
 * input parameters  :
 * buf
 * bufsize
 * -------------------------------------------------------------------------
 * output paremeters :
 * No
 * -------------------------------------------------------------------------
 * Side Effect
 * No
 ****************************************************************************
 */

int
VixMntSocketClient::rawWrite(const char *buf,
                             int bufsize)
{
   return VixMntSocket::rawWrite(sockfd, buf, bufsize);
}
