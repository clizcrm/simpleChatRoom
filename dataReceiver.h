#pragma once
#include <vector>
#include <string>
#include <map>
#include <optional>
#include <sys/socket.h>



#pragma pack(push,1)
struct dataHead{
    int version;
    char type;
    int len;
};
#pragma pack(pop)
//REG register name
//MSG broadcast the message
struct dataReceiver;
struct parser{
    int avSz;
    char type;
    bool isHd;
    std::string buf;
    parser();
    void append(char*,int,dataReceiver&,int);
};

struct dataReceiver{
private:
    friend struct parser;
    std::map<int,std::string> fdName;
    std::map<int,parser> ps;
    
    static dataReceiver&getDtRc();
    void broadCast(std::string,int);
public:
    static void addChat(int);
    static void rmChat(int);
    static void rcvMsg(int,char*,int);
};
