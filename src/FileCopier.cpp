#include "FileCopier.h"
#include <iostream>
#include <cstring>
#pragma warning(disable : 4996) // For strcpy in MSVC
using namespace std;

FileCopier::FileCopier(){
   chunkSize = CHUNK_BYTES;
   oneChunk = true;
   fileSize = 0;
   numChunks = 0;
}
FileCopier::~FileCopier(){
    if(iFile.is_open()) iFile.close();
    if(oFile.is_open()) oFile.close();
}

void FileCopier::setIFileName(){
   cout << "File name to send: ";
   cin >> iFileName;
}

void FileCopier::setOFileName(std::vector<char> extension){
   cout << "Save as filename: ";
   cin >> oFileName;
   for (char c : extension){
      oFileName.push_back(c);
   }
   oFile.open(oFileName, ios::out | ios::binary);
}

void FileCopier::setOFileName(char* extension, std::string fName){
   oFileName = fName + extension;
   oFile.open(oFileName, ios::out | ios::binary);
}

void FileCopier::findFileSize(){
   iFile.open(iFileName, ios::in | ios::binary | ios::ate);
   if (iFile.is_open()){
      fileSize = (uint64_t)iFile.tellg();
      iFile.seekg(0, ios::beg);
   }
   else {
	   cerr << "File could not be found: " << iFileName << endl;
       fileSize = 0;
   }
}

void FileCopier::findNumChunks(){
   if (fileSize > chunkSize){
      oneChunk = false;
      numChunks = fileSize / chunkSize;
      if (fileSize % chunkSize != 0) numChunks++;
   }
   else {
      numChunks = (fileSize > 0) ? 1 : 0;
   }
}

void FileCopier::doWork(){
   // Local copy logic (unused in network transfer but kept for completeness)
   if(!iFile.is_open()) findFileSize(); 
   
   oFile.open(oFileName, ios::out | ios::binary);
   if (!oFile.is_open()) {
       cerr << "Cannot create output file!" << endl;
       return;
   }

   char* buffer = new char[chunkSize];
   uint32_t bytesLeft = fileSize;
   
   while(bytesLeft > 0) {
       uint32_t toRead = (bytesLeft < chunkSize) ? bytesLeft : chunkSize;
       iFile.read(buffer, toRead);
       std::streamsize bytesRead = iFile.gcount();
       if(bytesRead > 0) {
           oFile.write(buffer, bytesRead);
           bytesLeft -= (uint32_t)bytesRead;
       } else {
           break;
       }
   }
   delete[] buffer;
}

uint64_t FileCopier::getSize(){
   return fileSize;
}

const char* FileCopier::getExtension(){
   size_t indexOf = iFileName.find_last_of(".");
   if (indexOf == std::string::npos) {
       char* ret = new char[1];
       ret[0] = '\0';
       return ret;
   }
   
   string temp = iFileName.substr(indexOf);
   
   // FIX: Allocate length + 1 for null terminator
   char* ret = new char[temp.length() + 1];
   strcpy(ret, temp.c_str());
   return ret;
}

uint64_t FileCopier::getNumChunks(){
   return numChunks;
}

bool FileCopier::getOneChunk(){
   return oneChunk;
}

uint32_t FileCopier::getChunkSize(){
   return chunkSize;
}

void FileCopier::readChunk(char* chunk, uint32_t sizeToRead){
   if(iFile.is_open()) {
       iFile.read(chunk, sizeToRead);
   }
}

void FileCopier::writeChunk(char* chunk, uint32_t sizeToWrite){
   if(oFile.is_open()) {
       oFile.write(chunk, sizeToWrite);
   }
}
