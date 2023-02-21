// test5 tests some error handling
#include <iostream>
#include <cassert>
#include <cstdlib>
#include <cstring>
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

    status = fs_create("user1", "/a", 'f');
    assert(!status);

    char buf[FS_BLOCKSIZE];

    const char* write_data = "Hello world";

    status = fs_writeblock("user1", "/a", 0, write_data);
    assert(!status);

    status = fs_readblock("user1", "/a",0,buf);
    printf("%s\n",buf);
    assert(!status);
    

    status = fs_readblock("user1", "/a",0,buf);
    printf("%s\n",buf);
    assert(!status);

    assert(!strcmp(write_data,buf));

    status = fs_delete("user1", "/a");
    assert(!status);

    status = fs_readblock("user1", "/a",0,buf);
    assert(status);

    status = fs_create("user2","/dir/eecs",'d');
    assert(status);

    status = fs_create("user2","/dir",'d');
    assert(!status);

    status = fs_create("user2","/dir/eecs",'d');
    assert(!status);

    status = fs_delete("user2","/eecs");
    assert(status);

    status = fs_delete("user2","/dir/eecs");
    assert(!status);
}