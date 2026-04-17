#include <iostream>
#include "Server.h"
#include "Client.h"
#include <cstdio>

int main() {
	std::setvbuf(stdout, NULL, _IONBF, 0);

    std::cout << "Press 1 for client, 2 for server: ";
    int x;
    if (!(std::cin >> x)) return 1;

    if (x == 1) {
        Client c;
    }
    else {
        Server s;
    }
    
    return 0;
}
