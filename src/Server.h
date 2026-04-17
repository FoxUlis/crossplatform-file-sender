#ifndef SERVER_H
#define SERVER_H

#include "Network.h"
#include "FileCopier.h"
#include <string>

class Server {
private:
    SocketType Socket;
    struct sockaddr_in serverInfo;
    
    int initWinsock();
    int initSocket();
    void sendData(FileCopier& f);
    void cleanup(); // Renamed from close

public:
    Server();
    ~Server();
    void start();
};

#endif
