#ifndef CLIENT_H
#define CLIENT_H

#include "Network.h"
#include "FileCopier.h"
#include <string>
#include <vector>

class Client {
private:
    SocketType Socket;
    struct sockaddr_in SockAddress;
    struct hostent* host;
    
    // WSADATA убран отсюда, так как он нужен только внутри функции init_networking() в Network.h
    
    std::string newFileName;
    uint32_t recSizeInBytes;
    std::vector<char> eVec;

    int initWinsock();
    int initSocket();
    void receiveData();
    void cleanup(); 

public:
    Client();
    ~Client();
    void start();
    uint32_t getSizeInBytes();
    std::vector<char> getExtension();
};

#endif
