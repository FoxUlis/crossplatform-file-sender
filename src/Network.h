#ifndef NETWORK_H
#define NETWORK_H

#ifdef _WIN64
    #include <winsock2.h>
    #include <ws2tcpip.h>
    #pragma comment(lib, "ws2_64.lib")
    typedef SOCKET SocketType;
    #define INVALID_SOCKET_VAL INVALID_SOCKET
    #define SOCKET_ERROR_VAL SOCKET_ERROR
    
    inline int init_networking() {
        WSADATA wsaData;
        return WSAStartup(MAKEWORD(2, 2), &wsaData);
    }
    inline void cleanup_networking() {
        WSACleanup();
    }
#else
    #include <sys/socket.h>
    #include <netinet/in.h>
    #include <arpa/inet.h>
    #include <unistd.h>
    #include <netdb.h>
    #include <cstring>
    
    typedef int SocketType;
    #define INVALID_SOCKET_VAL -1
    #define SOCKET_ERROR_VAL -1
    #define closesocket close
    #define SD_SEND SHUT_WR
    
    inline int init_networking() { return 0; }
    inline void cleanup_networking() {}
#endif

#include <cstdint>
#include <iostream>

// Добавлено inline для избежания multiple definition
inline bool sendAll(SocketType sock, const char* data, uint64_t length) {
    uint64_t totalSent = 0;
    while (totalSent < length) {
        int n = send(sock, data + totalSent, length - totalSent, 0);
        if (n <= 0) return false;
        totalSent += n;
    }
    return true;
}

// Добавлено inline для избежания multiple definition
inline bool recvAll(SocketType sock, char* data, uint64_t length) {
    uint64_t totalRec = 0;
    while (totalRec < length) {
        int n = recv(sock, data + totalRec, length - totalRec, 0);
        if (n <= 0) return false;
        totalRec += n;
    }
    return true;
}

inline uint64_t htonll(uint64_t value) {
	const int one = 1;
	const bool isLittleEndian = (*(const char*)&one == 1);

	if (isLittleEndian) {
		return ((uint64_t)htonl((uint32_t)(value >> 32))) |
			((uint64_t)htonl((uint32_t)value) << 32);
	} else {
		return value;
	}
}

inline uint64_t ntohll(uint64_t value) {
	return htonll(value);
}

#endif
