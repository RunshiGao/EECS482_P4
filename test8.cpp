// test5 tests some error handling
#include <iostream>
#include <cassert>
#include <cstdlib>
#include "fs_client.h"

using std::cout;

int main(int argc, char *argv[]) {
    char *server;
    int server_port;

    int status;

    if (argc != 3) {
        cout << "error: usage: " << argv[0] << " <server> <serverPort>\n";
        exit(1);
    }
    server = argv[1];
    server_port = atoi(argv[2]);

    fs_clientinit(server, server_port);

    // incorrect pathname
    status = fs_create("user1", "apple", 'd');
    assert(status);

    status = fs_create("user1", "/apple", 'd');
    assert(!status);
    status = fs_create("user1", "/apple/file", 'd');
    assert(!status);
    char readdata[FS_BLOCKSIZE];
    status = fs_readblock("user1", "/apple/file", -5, readdata);
    assert(status);

}