#include "Server.h"
#include <iostream>
#include <cstring>
#include <chrono>
#include <iomanip>


Server::Server(){
   Socket = INVALID_SOCKET_VAL;
   initWinsock();
   initSocket();
   start();
}
Server::~Server(){
    cleanup();
}

int Server::initWinsock(){
   if (init_networking() != 0){
      std::cout << "Networking initialization failed!\n";
      return 1;
   }
   return 0;
}

int Server::initSocket(){
   Socket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
   if (Socket == INVALID_SOCKET_VAL){
      std::cout << "Socket creation failed!\n";
      cleanup_networking();
      return 1;
   }

   int opt = 1;
   setsockopt(Socket, SOL_SOCKET, SO_REUSEADDR, (const char*)&opt, sizeof(opt));

   serverInfo.sin_family = AF_INET;
   serverInfo.sin_addr.s_addr = INADDR_ANY;
   serverInfo.sin_port = htons(4444);

   if (bind(Socket, (struct sockaddr*)(&serverInfo), sizeof(serverInfo)) == SOCKET_ERROR_VAL){
      std::cout << "Was unable to bind socket!\n";
      cleanup();
      return 2;
   }
   return 0;
}

void Server::start(){
   FileCopier f;
   f.setIFileName();
   f.findFileSize();
   f.findNumChunks();

   if (f.getSize() == 0) {
       std::cout << "File is empty or not found. Exiting.\n";
       cleanup();
       return;
   }

   listen(Socket, 1);
   std::cout << "Waiting for incoming connections...\n";
   
   SocketType clientSocket = accept(Socket, NULL, NULL);
   if (clientSocket == INVALID_SOCKET_VAL) {
       std::cout << "Accept failed!\n";
       cleanup();
       return;
   }
   
   closesocket(Socket); 
   Socket = clientSocket;

   std::cout << "Client has connected!\n";
   sendData(f);
   cleanup();
}

void Server::cleanup(){
   if (Socket != INVALID_SOCKET_VAL) {
       shutdown(Socket, SD_SEND);
       closesocket(Socket);
       Socket = INVALID_SOCKET_VAL;
   }
   cleanup_networking();
}

void Server::sendData(FileCopier& f){
   //1. Send file size (64-bit)
   uint64_t fileSize = f.getSize();
   uint64_t netFileSize = htonll(fileSize);
   
   std::cout << "Sending filesize... (" << fileSize << "bytes)...";
   if (!sendAll(Socket, (char*)&netFileSize, sizeof(netFileSize))) {
       std::cout << "Failed.\n"; return;
   }
   std::cout << "OK\n";

   //2. Get extension
   const char* extension = f.getExtension();
   uint32_t extensionLength = (uint32_t)strlen(extension);
   uint32_t netExtLen = htonl(extensionLength);

   //3. Send extension length (32-bit is enough for extension)
   std::cout << "Sending extension length... ";
   if (!sendAll(Socket, (char*)&netExtLen, sizeof(netExtLen))) {
       delete[] extension; return;
   }
   std::cout << "OK\n";

   //4. Send extension str
   std::cout << "Sending extension... ";
   if (extensionLength > 0) {
       if (!sendAll(Socket, extension, extensionLength)) {
           delete[] extension; return;
       }
   }
   std::cout << "OK\n";
   delete[] extension;

   //5. Send data chunks
   uint64_t chunkSize = f.getChunkSize();
   char* currentChunk = new char[chunkSize];
   
   uint64_t totalBytesSent = 0;
   uint64_t bytesLeft = fileSize;

   //Start timer
   auto start = std::chrono::high_resolution_clock::now();

   std::cout << "Sending data...\n";

   while (bytesLeft > 0) {
       uint64_t toRead = (bytesLeft < chunkSize) ? bytesLeft : chunkSize;
       f.readChunk(currentChunk, toRead);
       
       if (!sendAll(Socket, currentChunk, toRead)) {
           std::cout << "Send error.\n";
           break;
       }
       
       totalBytesSent += toRead;
       bytesLeft -= toRead;

	   //Calculate speed and progress
	   auto end = std::chrono::high_resolution_clock::now();
	   double second = std::chrono::duration<double>(end - start).count();

	   //Avoid division by zero
	   double mbps = (second > 0) ? (totalBytesSent / (1024.0 * 1024.0)) / second : 0;

	   double sentMB = totalBytesSent / (1024.0 * 1024.0);
	   double totalMB = fileSize / (1024.0 * 1024.0);

	   std::cout << "\rSent: " << std::fixed << std::setprecision(2)
		   		 << sentMB << "/" << totalMB << "MB"
				 << " (" << std::setprecision(2) << mbps << " MB/s)";
       std::cout.flush();
   }
   
   std::cout << "\n--->File successfully sent!\n";
   delete[] currentChunk;
}
