#include <vixMntSocket.h>
#include <vixMntOperation.h>

VixMntSocketServer::VixMntSocketServer() : VixMntSocket(){
    sockaddr_in servaddr;
    listenfd = socket(AF_INET,SOCK_STREAM,0);

    if(listenfd == -1){
        ELog("socket error");
        //perror("socket error:");
        exit(1);
    }

    bzero(&servaddr,sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    inet_pton(AF_INET,SOCKET_IPADDRESS,&servaddr.sin_addr);
    servaddr.sin_port = htons(SOCKET_PORT);

    if( bind(listenfd,(sockaddr*)&servaddr,sizeof(servaddr)) == -1 ){
        //perror("bind error:");
        ELog("bind error");
        exit(1);
    }
}

VixMntSocketServer::~VixMntSocketServer(){
    close(listenfd);
}

void
VixMntSocketServer::serverListen(VixMntDiskHandle* vixdh){
    listen(listenfd,SOCKET_LISTENQ);
    this->vixdh = vixdh;
    doEpoll();

}

void
VixMntSocketServer::doEpoll(){
    epoll_event events[SOCKET_EPOLLEVENTS];
    int ret;
    char buf[SOCKET_BUF_MAX_SIZE];
    memset(buf,0,SOCKET_BUF_MAX_SIZE);
    epollfd = epoll_create(SOCKET_FD_MAX_SIZE);
    addEvent(listenfd,EPOLLIN);
    while(true){

       ret = epoll_wait(epollfd,events,SOCKET_EPOLLEVENTS,-1);

       handleEvents(events,ret,buf);
       if(ret <=0 ) return;
    }
    close(epollfd);

}

void
VixMntSocketServer::handleEvents(
         epoll_event *events,
         int num,
         char *buf){
    for(int i=0 ; i< num; i++){
        int fd = events[i].data.fd;

        if(fd ==listenfd && ( events[i].events & EPOLLIN ))
            handleAccept();
        else if (events[i].events & EPOLLIN)
            doRead(fd,buf);
        else if(events[i].events & EPOLLOUT)
            doWrite(fd,buf);
    }
}

void
VixMntSocketServer::handleAccept(){
     int clientFd;
     sockaddr_in clientAddr;
     socklen_t clientAddrLen;
     clientFd = accept(listenfd,(sockaddr*)&clientAddr,&clientAddrLen);

     if(clientFd == -1){
         ELog("Accept Error");
         //perror("accept error.");

     }
     else{
          ILog("Accept a new client : %s : %d",inet_ntoa(clientAddr.sin_addr),clientAddr.sin_port);
          addEvent(clientFd,EPOLLIN);
          clientMap4Write[clientFd] = 0;
     }
}


void
VixMntSocketServer::doRead(int fd, char* buf,int maxLen){
     maxLen = maxLen>0?maxLen:SOCKET_BUF_MAX_SIZE;
     //int nread = read(fd,buf,maxLen);
     int nread = recv(fd,buf,maxLen,0);
    //ILog("doread fd : %d, maxLen : %d",fd,maxLen);
     if(nread == -1){
         ELog("Read Error");
         //perror("read error");
         close(fd);
     }
     else if(nread == 0){
          ILog("Client Close");
          clientMap4Write.erase(fd);
          close(fd);
          deleteEvent(fd,EPOLLIN);
     }
     else{
         //VixMntOpRead opReadData;
         //opReadData.convertFromBytes(buf);
         //uint64 sizeResult = opReadData.bufsize * VIXDISKLIB_SECTOR_SIZE;

         //VixError vixError = vixdh->read((uint8*)buf,opReadData.offset,opReadData.bufsize);
         //deleteEvent(fd,EPOLLIN);
        /*
         uint64 offset,bufsize;
         uint64 token;
         memcpy(&offset,buf,sizeof(offset));
         memcpy(&bufsize,buf+sizeof(offset),sizeof(bufsize));
         memcpy(&token,buf+2*sizeof(offset),sizeof(bufsize));
         */
         VixMntOpSocketRead vixskop;
         vixskop.convertFromBytes(buf);
         VixError vixError = vixdh->read((uint8*)buf,vixskop.offset,vixskop.bufsize);
         SHOW_ERROR_INFO(vixError);
         ILog("Read offset %u , bufsize %u,token %u",vixskop.offset,vixskop.bufsize,vixskop.token);
         /* mark client needed buffer size for next write operation*/
         clientMap4Write[fd] = vixskop.bufsize * VIXDISKLIB_SECTOR_SIZE;
        modifyEvent(fd,EPOLLOUT);
        // addEvent(fd,EPOLLIN);
     }
}

void
VixMntSocketServer::doWrite(int fd, char* buf,int maxLen){
    //maxLen =  maxLen>0?maxLen:strlen(buf);
    ILog("dowrite fd : %d, maxLen : %d",fd,clientMap4Write[fd]);
    //int nwrite = write(fd,buf,maxLen);
    //int nwrite = write(fd,buf,clientMap4Write[fd]);
    int nwrite = send(fd,buf,clientMap4Write[fd],0);
    if ( nwrite == -1 ){
         ELog("Write Error");
         close(fd);
         deleteEvent(fd,EPOLLOUT);
    }
    else
        modifyEvent(fd,EPOLLIN);
    //memset(buf,0,maxLen);
}

void
VixMntSocket::addEvent(int fd, int state){
     epoll_event ev;
     ev.events = state;
     ev.data.fd = fd;
     epoll_ctl(epollfd,EPOLL_CTL_ADD,fd,&ev);
}

void
VixMntSocket::deleteEvent(int fd, int state){
     epoll_event ev;
     ev.events = state;
     ev.data.fd = fd;
     epoll_ctl(epollfd,EPOLL_CTL_DEL,fd,&ev);
}

void
VixMntSocket::modifyEvent(int fd, int state){
     epoll_event ev;
     ev.events = state;
     ev.data.fd = fd;
     epoll_ctl(epollfd,EPOLL_CTL_MOD,fd,&ev);
}

VixMntSocketClient::VixMntSocketClient() : VixMntSocket(){
    sockfd = socket(AF_INET,SOCK_STREAM,0);
    sockaddr_in servaddr;
    bzero(&servaddr,sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(SOCKET_PORT);
    inet_pton(AF_INET,SOCKET_IPADDRESS,&servaddr.sin_addr);
    connect(sockfd,(sockaddr*)&servaddr,sizeof(servaddr));

}

VixMntSocketClient::~VixMntSocketClient(){
    close(sockfd);
}

void
VixMntSocketClient::doRead(int fd,char *buf,int maxLen){
    // TODO

}

void
VixMntSocketClient::doWrite(int fd,char *buf,int maxLen){
    // TODO
}

void
VixMntSocketClient::handleConnect(){
    epoll_event events[SOCKET_EPOLLEVENTS];
    char buf[SOCKET_BUF_MAX_SIZE];
    int ret;
    memset(buf,0,SOCKET_BUF_MAX_SIZE);
    epollfd = epoll_create(SOCKET_FD_MAX_SIZE);
    addEvent(STDIN_FILENO,EPOLLIN);

    while(true){
         ret = epoll_wait(epollfd,events,SOCKET_EPOLLEVENTS,-1);
         handleEvents(events,ret,buf);

         if(ret <=0 ) return;
    }

    close(epollfd);
}

void
VixMntSocketClient::handleEvents(epoll_event* events,int num,char* buf){
    for(int i=0 ; i<num ; ++i){
         int fd = events[i].data.fd;
         if(events[i].events & EPOLLIN)
             doRead(fd,buf);
         else if(events[i].events & EPOLLOUT)
             doWrite(fd,buf);
    }
}

/*
 * read whole needed sectors
 * if not do this, client receiver will get incomplete data
 */

int
VixMntSocketClient::rawRead(char *buf,int bufsize){
    int recvSize = read(sockfd,buf,bufsize);
    if(recvSize <=0 ) return recvSize;
    while(recvSize<bufsize){
         int tempRecvSize = read(sockfd,buf+recvSize,bufsize-recvSize);
         recvSize+=tempRecvSize;
    }
    return recvSize;
}
int
VixMntSocketClient::rawWrite(char *buf,int bufsize){
    //return write(sockfd,buf,bufsize);
    return send(sockfd,buf,bufsize,0);
}
