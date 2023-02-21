// test2 tests lowest inode
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

    status = fs_create("user1", "/apple", 'd');
    assert(!status);

    status = fs_create("user1", "/apple/pear", 'd');
    assert(!status);

    status = fs_create("user1", "/apple/pear/1", 'f');
    assert(!status);

    status = fs_create("user1", "/apple/pear/2", 'f');
    assert(!status);

    status = fs_create("user1", "/apple/pear/3", 'f');
    assert(!status);

    status = fs_create("user1", "/apple/pear/4", 'f');
    assert(!status);

    status = fs_create("user1", "/apple/pear/5", 'f');
    assert(!status);

    status = fs_create("user1", "/apple/pear/6", 'f');
    assert(!status);

    status = fs_create("user1", "/apple/pear/7", 'f');
    assert(!status);

    status = fs_create("user1", "/apple/pear/8", 'f');
    assert(!status);

    status = fs_create("user1", "/apple/pear/9", 'f');
    assert(!status);

    status = fs_delete("user1", "/apple/pear/1");
    assert(!status);

    status = fs_create("user1", "/apple/pear/10", 'f');
    assert(!status);
}