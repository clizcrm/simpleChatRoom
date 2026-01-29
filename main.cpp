#include <iostream>
#include <arpa/inet.h>
#include <unistd.h>
#include <sys/epoll.h>
#include <sys/socket.h>
#include <fcntl.h>
#include "dataReceiver.h"

int setNonBlocking(int fd) {
    int flags=fcntl(fd,F_GETFL);
    return fcntl(fd,F_SETFL,flags|O_NONBLOCK);
}

int main() {
    int lsSkFd=socket(AF_INET,SOCK_STREAM,IPPROTO_TCP);
    if(lsSkFd<0) {
        perror("socket");
        return 1;
    }
    int opt=1;
    setsockopt(lsSkFd,SOL_SOCKET,SO_REUSEADDR,&opt,sizeof opt);
    
    setNonBlocking(lsSkFd);

    sockaddr_in lsAd{};
    lsAd.sin_family=AF_INET;
    lsAd.sin_port=htons(8080);
    lsAd.sin_addr.s_addr=INADDR_ANY;

    if(bind(lsSkFd,(sockaddr*)&lsAd,sizeof(lsAd))<0) {
        perror("bind");
        return 1;
    }

    if(listen(lsSkFd,1024)<0){
        perror("listen");
        return 1;
    }

    int epoFd = epoll_create1(0);
    epoll_event ev{};
    ev.data.fd=lsSkFd;
    ev.events=EPOLLIN|EPOLLET;
    epoll_ctl(epoFd,EPOLL_CTL_ADD,lsSkFd,&ev);

    epoll_event events[1024];

    std::cout<<"lauched successfully"<<std::endl;

    while(1) {
        int n=epoll_wait(epoFd,events,1024,-1);
        for(int i=0;i<n;i++) {
            int fd=events[i].data.fd;
            if(fd==lsSkFd){
                while(true) {
                    sockaddr_in clAd{};
                    socklen_t clLn=sizeof(clAd);
                    int clSkFd=accept(lsSkFd,(sockaddr*)&clAd,&clLn);
                    if(clSkFd>=0) {
                        setNonBlocking(clSkFd);
                        epoll_event clEv{};
                        clEv.data.fd=clSkFd;
                        clEv.events=EPOLLET|EPOLLIN;
                        epoll_ctl(epoFd,EPOLL_CTL_ADD,clSkFd,&clEv);
                        dataReceiver::addChat(clSkFd);
                        std::cout<<"New client:"<<clSkFd<<'\n';
                    }else{
                        if(errno==EAGAIN||errno==EWOULDBLOCK){
                            break;
                        } else {
                            perror("accept");
                            break;
                        }
                    }
                }
            } else {
                char buffer[1024];
                while(true) {
                    int len=recv(fd,buffer,1024,0);
                    if(len>0) {
                        dataReceiver::rcvMsg(fd,buffer,len);
                    } else if(len==0) {
                        dataReceiver::rmChat(fd);
                        break;
                    } else {
                        if(errno == EAGAIN||errno==EWOULDBLOCK) {
                            break;
                        } else {
                            perror("recv");
                            dataReceiver::rmChat(fd);
                            break;
                        }
                    }
                }
            }
        }
    }

    close(epoFd);
    close(lsSkFd);
}