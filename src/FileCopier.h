#ifndef FILECOPIER_H
#define FILECOPIER_H

#include <string>
#include <fstream>
#include <vector>
#include <cstdint>

#define CHUNK_BYTES (1024 * 1024)// 1 MB Chunk

class FileCopier {
private:
    std::string iFileName;
    std::string oFileName;
    std::ifstream iFile;
    std::ofstream oFile;
    uint64_t fileSize;
    uint32_t chunkSize;
    uint64_t numChunks;
    bool oneChunk;

public:
    FileCopier();
    ~FileCopier();

    void setIFileName();
    void setOFileName(std::vector<char> extension); // Для клиента (если нужно)
    void setOFileName(char* extension, std::string fName); // Для клиента из вашего кода
    
    void findFileSize();
    void findNumChunks();
    void doWork(); // Локальное копирование (не используется в сети, но оставим)

    uint64_t getSize();
    const char* getExtension(); // Возвращает new char[], нужно удалять вручную!
    
    uint64_t getNumChunks();
    bool getOneChunk();
    uint32_t getChunkSize();

    void readChunk(char* chunk, uint32_t sizeToRead);
    void writeChunk(char* chunk, uint32_t sizeToWrite);
};

#endif
