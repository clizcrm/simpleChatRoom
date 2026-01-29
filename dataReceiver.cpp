#include <cstring>
#include <unistd.h>
#include <iostream>
#include "dataReceiver.h"


parser::parser():type('0'),isHd(true),avSz(sizeof(dataHead)) {}

void parser::append(char*str,int n,dataReceiver&dtr,int fd) {
    std::cout<<"msg length"<<n<<std::endl;
    buf.append(str,n);
    while(buf.length()>=avSz) {
        if(isHd) {
            dataHead tp{};
            std::string gt=buf.substr(0,avSz);
            std::memcpy(&tp,gt.c_str(),sizeof(tp));
            std::cout<<"head received"<<tp.type<<std::endl<<tp.len<<std::endl;
            buf=buf.substr(avSz);
            isHd=false;
            avSz=tp.len;
            type=tp.type;
        }else{
            std::string str=buf.substr(0,avSz); 
            buf=buf.substr(avSz);
            isHd=1;
            avSz=sizeof(dataHead);
            if(type=='R') {
                dtr.fdName[fd]=str;
            }else{
                dtr.broadCast(str,fd);
            }
        }
    }
}

dataReceiver& dataReceiver::getDtRc(){
    static dataReceiver hd{};
    return hd;
}

void dataReceiver::addChat(int fd) {
    auto&p=getDtRc().ps;
    p.insert({fd,parser()});
}

void dataReceiver::rmChat(int fd) {
    auto&t=getDtRc().fdName;
    auto&p=getDtRc().ps;
    if(t.count(fd)) t.erase(fd);
    if(p.count(fd)) p.erase(fd);
    close(fd);
}

void dataReceiver::broadCast(std::string msg,int fd) {
    msg=fdName[fd]+msg;
    for(auto&[Fd,name]:fdName) {
        if(Fd==fd)continue;
        send(Fd,msg.c_str(),msg.length(),0);
    }
}

void dataReceiver::rcvMsg(int fd,char*msg,int len) {
    auto&dr=getDtRc();
    dr.ps[fd].append(msg,len,dr,fd);
}