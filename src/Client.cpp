#include "Client.h"
#include <iostream>
#include <string>
#include <chrono>
#include <iomanip>
#pragma warning(disable : 4996)

Client::Client(){
   recSizeInBytes = 0;
   Socket = INVALID_SOCKET_VAL;
   host = nullptr;
   initWinsock();
   initSocket();
   start();
}
Client::~Client(){
    cleanup();
}

int Client::initWinsock(){
   if (init_networking() != 0){
      std::cout << "Networking initialization failed!\n";
      return 1;
   }
   return 0;
}

int Client::initSocket(){
   Socket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
   if (Socket == INVALID_SOCKET_VAL){
      std::cout << "Socket creation failed!\n";
      cleanup_networking();
      return 1;
   }

   std::cout << "Please enter new filename (to save as): ";
   std::cin >> newFileName;

   std::cout << "Please enter server's hostname or IP address: ";
   std::string s;
   std::cin >> s;
   
   struct hostent* h = gethostbyname(s.c_str());
   if (h == NULL){
      std::cout << "Failed to resolve hostname!\n";
      cleanup();
      return 2;
   }
   host = h;

   std::cout << "Please enter server's port number: ";
   int portNum;
   std::cin >> portNum;

   SockAddress.sin_port = htons(portNum);
   SockAddress.sin_family = AF_INET;
   memcpy(&SockAddress.sin_addr.s_addr, host->h_addr, host->h_length);

   if (connect(Socket, (struct sockaddr*)(&SockAddress), sizeof(SockAddress)) != 0){
      std::cout << "Failed to connect with server!\n";
      cleanup();
      return 3;
   }

   std::cout << "\nSuccessfully connected to server at: " << s << " : " << portNum << "\n";
   return 0;
}

void Client::start(){
   receiveData();
   cleanup();
}

void Client::cleanup(){
   if (Socket != INVALID_SOCKET_VAL) {
       shutdown(Socket, SD_SEND);
       closesocket(Socket);
       Socket = INVALID_SOCKET_VAL;
   }
   cleanup_networking();
}

uint32_t Client::getSizeInBytes(){
   return recSizeInBytes;
}
std::vector<char> Client::getExtension(){
   return eVec;
}

void Client::receiveData(){
   //1. Receive file size (64-bit)
   uint64_t netFileSize = 0;
   std::cout << "Receiving filesize... ";
   if (!recvAll(Socket, (char*)&netFileSize, sizeof(netFileSize))) {
       std::cout << "Failed.\n"; return;
   }
   uint64_t fileSize = ntohl(netFileSize);
   std::cout << "---> " << fileSize << " bytes\n";

   //2. Receive extension length (32-bit)
   uint32_t netExtLen = 0;
   std::cout << "Receiving extension length... ";
   if (!recvAll(Socket, (char*)&netExtLen, sizeof(netExtLen))) {
       std::cout << "Failed.\n"; return;
   }
   uint32_t extensionLength = ntohl(netExtLen);
   std::cout << "---> " << extensionLength << " bytes\n";

   //3. Recieve extension
   char* extension = new char[extensionLength + 1];
   if (extensionLength > 0) {
       std::cout << "Receiving extension... ";
       if (!recvAll(Socket, extension, extensionLength)) {
           delete[] extension;
           std::cout << "Failed.\n"; return;
       }
   }
   extension[extensionLength] = '\0';
   std::cout << "---> ." << extension << "\n";
   
   //4. Receive data
   FileCopier f;
   f.setOFileName(extension, newFileName);
   delete[] extension;

   auto start = std::chrono::high_resolution_clock::now();
   uint64_t allBytesRec = 0;
   uint32_t chunkSize = 1024 * 1024; //1MB
   char* currentChunk = new char[chunkSize];
   
   uint64_t bytesLeft = fileSize;

   std::cout << "Receiving data...\n";

   while (bytesLeft > 0) {
       uint32_t toRead = (bytesLeft < chunkSize) ? bytesLeft : chunkSize;
       
       if (!recvAll(Socket, currentChunk, toRead)) {
           std::cout << "Receive error.\n";
           break;
       }
       
       f.writeChunk(currentChunk, toRead);

       allBytesRec += toRead;
       bytesLeft -= toRead;

       auto end = std::chrono::high_resolution_clock::now();
       double seconds = std::chrono::duration<double>(end - start).count();
       double mbps = (seconds > 0) ? (allBytesRec / 1000000.0) / seconds : 0;

       std::cout << "\rReceiving: " << allBytesRec / (1024.0*1024.0)<< "/" << fileSize / (1024*1024)
                 << " MB (" << std::fixed << std::setprecision(2) << mbps << " MB/s)";
       std::cout.flush();
   }

   delete[] currentChunk;
   std::cout << "\n--->File successfully received!\n";
}
