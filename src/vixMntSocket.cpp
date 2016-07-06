#include <vixMntSocket.h>

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
VixMntSocketServer::serverListen(){
    listen(listenfd,SOCKET_LISTENQ);
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
     }
}


void
VixMntSocketServer::doRead(int fd, char* buf,int maxLen){
     maxLen = maxLen>0?maxLen:SOCKET_BUF_MAX_SIZE;
     int nread = read(fd,buf,maxLen);

     if(nread == -1){
         ELog("Read Error");
         //perror("read error");
         close(fd);
     }
     else if(nread == 0){
          ILog("Client Close");
          close(fd);
          deleteEvent(fd,EPOLLIN);
     }
     else{
         ILog("Read msg is : %s",buf);
         modifyEvent(fd,EPOLLOUT);
     }
}

void
VixMntSocketServer::doWrite(int fd, char* buf,int maxLen){
    maxLen =  maxLen>0?maxLen:strlen(buf);
    int nwrite = write(fd,buf,maxLen);
    if ( nwrite == -1 ){
         ELog("Write Error");
         close(fd);
         deleteEvent(fd,EPOLLOUT);
    }
    else
        modifyEvent(fd,EPOLLIN);
    memset(buf,0,maxLen);
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

int
VixMntSocketClient::rawRead(char *buf,int bufsize){
    return read(sockfd,buf,bufsize);
}
int
VixMntSocketClient::rawWrite(char *buf,int bufsize){
    return write(sockfd,buf,bufsize);
}
