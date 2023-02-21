// test5 tests some error handling
#include <iostream>
#include <cassert>
#include <cstdlib>
#include "fs_client.h"

using namespace std;

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
    
    status = fs_create("user1", "/apple\0 banana/app", 'd');
    assert(!status);
    status = fs_create("user1", "/apple\0 banana", 'd');
    assert(status);
    status = fs_create("user1", "/apple//banana", 'd');
    assert(status);
    status = fs_create("user1", "/apple/", 'd');
    assert(status);
    status = fs_create("user1", "apple/", 'd');
    assert(status);
    status = fs_create("user1", "/apple/012345678901234567890123456789012345678901234567890123456789", 'd');
    assert(status);
    status = fs_create("user1", "/apple/01234567890123456789012345678901234567890124567890123456789/01234567890123456789012345678901234567890124567890123456789/01234567890123456789012345678901234567890124567890123456789", 'd');
    assert(status);


    // pathname too long
    status = fs_create("user1", "/abc/abc/abc/abc/abc/abc/abc/abc/abc/abc/abc/abc/abc/abc/abc/abc/abc/abc/abc/abc/abc/abc/abc/abc/abc/abc/abc/abc/abc/abc/abc/abc", 'd');
    assert(status);
    status = fs_create("user1", "/abc/abc/abc/abc/abc/abc/abc/abc/abc/abc/abc/abc/abc/abc/abc/abc/abc/abc/abc/abc/abc/abc/abc/abc/abc/abc/abc/abc/abc/abc/abc/abca", 'd');
    assert(status);


    status = fs_create("12345678901", "/apple", 'd');
    assert(status);
    status = fs_create("", "/apple", 'd');
    assert(status);
    status = fs_create("user1", "/", 'a');
    assert(status);
}