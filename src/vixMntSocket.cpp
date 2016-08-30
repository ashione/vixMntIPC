#include <vixMntOperation.h>
#include <vixMntSocket.h>
#include <vixMntException.h>

/**
 * @brief  bind a given port on localhost, it will exit whenever bind error.
 */

VixMntSocketServer::VixMntSocketServer() : VixMntSocket()
{
   sockaddr_in servaddr;
   listenfd = socket(AF_INET, SOCK_STREAM, 0);

   if (listenfd == -1) {
      ELog("socket error");
      throw VixMntException("Socket Error");
   }

   bzero(&servaddr, sizeof(servaddr));
   servaddr.sin_family = AF_INET;
   inet_pton(AF_INET, SOCKET_IPADDRESS, &servaddr.sin_addr);
   servaddr.sin_port = htons(SOCKET_PORT);

   if (bind(listenfd, (sockaddr *)&servaddr, sizeof(servaddr)) == -1) {
      ELog("bind error");
      throw VixMntException("Bind Error");
   }
   ILog("Server Start.");
}

VixMntSocketServer::~VixMntSocketServer() { close(listenfd); }


/**
 * @brief starting server listening and enbale epoll for socket fd.
 *
 * @param vixdh [in] opened disk handle
 */

void
VixMntSocketServer::serverListen(
        std::map<std::string,VixMntDiskHandle *> &vixdh)
{
   if (listen(listenfd, SOCKET_LISTENQ)) {
      ELog("Server Listen Error, %s", strerror(errno));
      throw VixMntException("Server Listen Error");
   } else {
      ILog("Server Listen Start");
      this->vixdhMap = &vixdh;
      doEpoll();
   }
}


/**
 * @brief  epolling starting
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
 * @brief  handle different event in propety operating function.
 *
 * @param events [in]
 * @param num [in]
 * @param buf [in] local buffer array
 */

void
VixMntSocketServer::handleEvents(epoll_event *events,
                                 int num,
                                 char *buf)
{
   try {
      for (int i = 0; i < num; i++) {
         int fd = events[i].data.fd;

         if (fd == listenfd && (events[i].events & EPOLLIN))
            handleAccept();
         else if (events[i].events & EPOLLIN)
            doRead(fd, buf);
         else if (events[i].events & EPOLLOUT)
            doWrite(fd, buf);
      }
   } catch (VixMntException& e) {
      ELog("Exception : %s",e.what());
   }

}


/**
 * @brief  Accept a new socket client or report error.
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
      throw VixMntException("Accept Error");

   } else {
      addEvent(clientFd, EPOLLIN);
      clientMap4Write[clientFd] = 0;
   }
}


/**
 * @brief  parser control data, then read data from remote disk via
 * vixmntdiskhandle. If realted filename(diskname) not exist in vixdhMap,
 * no handle will be responsed.
 *
 * @param fd
 * @param buf
 * @param maxLen
 */

void VixMntSocketServer::doRead(int fd,
                                char *buf,
                                int maxLen)
{

   maxLen = maxLen > 0 ? maxLen : sizeof(VixMntOpSocket);
   int nread = recv(fd, buf, maxLen, 0);
   if (nread == -1) {
      WLog("Read Error");
      close(fd);
      throw VixMntException("Read Exception");
   } else if (nread == 0) {
      clientMap4Write.erase(fd);
      close(fd);
      deleteEvent(fd, EPOLLIN);
   } else {
      VixMntOpSocket vixskop;
      vixskop.convertFromBytes(buf);
      std::string diskHandleName = vixskop.fileName;
      VixMntDiskHandle *vixDiskHandle = NULL;
      if (vixdhMap->find(diskHandleName) != vixdhMap->end()) {
          vixDiskHandle = (*vixdhMap)[diskHandleName];
      }
      assert(vixDiskHandle);
      uint32 sectorSize = vixDiskHandle->getSectorSize();

      if (vixskop.carriedOp == VixMntOp(MntRead)) {
         uint64 maxOps = vixskop.bufsize * sectorSize/ SOCKET_BUF_MAX_SIZE;
         uint64 eachSectorSize = SOCKET_BUF_MAX_SIZE / sectorSize;
         uint32 i = 0;
         for (; i < maxOps; ++i) {

            VixError vixError =
               vixDiskHandle->read(
                       (uint8 *)buf,
                       vixskop.offset + i * eachSectorSize,
                       eachSectorSize);
            SHOW_ERROR_INFO(vixError);
            int nwrite = rawWrite(fd, buf, SOCKET_BUF_MAX_SIZE);

            if (nwrite <= 0) {
               ELog("Batch Write Error.");
               throw VixMntException("Batch Write Error");
            }
         }
         uint64 leftSector = vixskop.bufsize - i * eachSectorSize;

         if (leftSector > 0) {

            // mark client needed buffer size for next write operation
            VixError vixError = vixDiskHandle->read(
               (uint8 *)buf, vixskop.offset + i * eachSectorSize, leftSector);
            SHOW_ERROR_INFO(vixError);
            int nwrite = rawWrite(fd, buf, leftSector * sectorSize);

            if (nwrite <= 0) {
               ELog("Reminded Write Error.");
               throw VixMntException("Reminded Write Error");
            }
         }
      } else if (vixskop.carriedOp == VixMntOp(MntWrite)) {
         // continue to read reminded buf
         int ndataread = recv(fd, buf, vixskop.carriedBufSize, 0);
         vixskop.carriedBufSize = 0;

         if (ndataread == -1) {
            WLog("Read Error");
            close(fd);
            throw VixMntException("Read Exception");
         } else if (ndataread == 0) {
            ILog("Client Close");
            clientMap4Write.erase(fd);
            close(fd);
            deleteEvent(fd, EPOLLIN);
         } else {
            VixError vixError =
               vixDiskHandle->write(
                       (uint8 *)buf,
                       vixskop.offset,
                       vixskop.bufsize);
            SHOW_ERROR_INFO(vixError);
            vixskop.carriedOp = VixMntOp(MntWriteDone);
            ILog("Write offset %u , bufsize %u,token %u",
                  vixskop.offset, vixskop.bufsize, vixskop.token);

            // nofity socket client write successful
            memcpy(buf, (char *)&vixskop, sizeof(vixskop));
            clientMap4Write[fd] = sizeof(vixskop);
            modifyEvent(fd, EPOLLOUT);
         }

      } else {
         ELog("Error");
         throw VixMntException("Unkown Operation Error");
      }
   }
}


/**
 * @brief  deprecated.
 *
 * @param fd
 * @param buf
 * @param maxLen
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
      throw VixMntException("Write Error");
   } else
      modifyEvent(fd, EPOLLIN);
}


/**
 * @brief Add event for socket epoll
 *
 * @param fd
 * @param state [in] epoll action
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
 * @brief  delete event from epoll eventlist
 *
 * @param fd
 * @param state
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
 * @brief modify event from epoll eventlist
 *
 * @param fd
 * @param state
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
 * @brief  read data directly, but it will keep reading until received data
 * length is equal to buffer size.
 *
 * @param fd
 * @param buf
 * @param bufsize
 *
 * @return
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
      throw VixMntException("Server Close");
   } else if (recvSize == 0) {
      ELog("Send Error");
      throw VixMntException("Send Close");
   } else
      while (recvSize < bufsize) {
         int tempRecvSize = read(fd, buf + recvSize, bufsize - recvSize);
         recvSize += tempRecvSize;
      }
   return recvSize;
}


/**
 * @brief  keep writing until all data were sent out.
 *
 * @param fd
 * @param buf
 * @param bufsize
 *
 * @return  written length
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
         throw VixMntException("Server Close");
      } else if (sendSize == 0) {
         ELog("Send Error");
         throw VixMntException("Send Error");
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
 * @brief  connect socket server antomatically.
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
      throw VixMntException("Connection Error");
   }
}

VixMntSocketClient::~VixMntSocketClient() { close(sockfd); }


void
VixMntSocketClient::doRead(int fd,
                           char *buf,
                           int maxLen)
{
   throw VixMntException("Unimplemented");
}

void
VixMntSocketClient::doWrite(int fd,
                            char *buf,
                            int maxLen)
{
   throw VixMntException("Unimplemented");
}


/**
 * @brief  deprecated.
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
 * @brief  client epoll events handle.
 *
 * @param events
 * @param num
 * @param buf
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
 * @brief  super class rawRead directly
 *
 * @param buf
 * @param bufsize
 *
 * @return
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
 * @brief  call super class  write fucntion directly.
 *
 * @param buf
 * @param bufsize
 *
 * @return
 */

int
VixMntSocketClient::rawWrite(const char *buf,
                             int bufsize)
{
   return VixMntSocket::rawWrite(sockfd, buf, bufsize);
}
