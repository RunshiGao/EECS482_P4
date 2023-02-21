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

    status = fs_create("user1", "/apple", 'd');
    assert(!status);

    // create duplicated directory
    status = fs_create("user1", "/apple", 'd');
    assert(status);

    status = fs_create("user1", "/apple", 'f');
    assert(status);
    
    // delete root
    status = fs_delete("user1", "/");
    assert(status);

    // delete folder that doesn't exist
    status = fs_delete("user1", "/pear");
    assert(status);

    // delete folder as wrong user
    status = fs_delete("user2", "/apple");
    cout << status << std::endl;
    assert(status);

    status = fs_create("user1", "/file", 'f');
    // write file as wrong user
    const char *writedata = "We hold these truths to be self-evident, that all men are created equal, that they are endowed by their Creator with certain unalienable Rights, that among these are Life, Liberty and the pursuit of Happiness. -- That to secure these rights, Governments are instituted among Men, deriving their just powers from the consent of the governed, -- That whenever any Form of Government becomes destructive of these ends, it is the Right of the People to alter or to abolish it, and to institute new Government, laying its foundation on such principles and organizing its powers in such form, as to them shall seem most likely to effect their Safety and Happiness.";
    status = fs_writeblock("user2", "/dir/file", 0, writedata);
    assert(status);

    status = fs_create("user1", "/dir", 'd');
    status = fs_create("user1", "/dir/file", 'f');
    status = fs_writeblock("user1", "/dir/file", 0, writedata);
    assert(!status);

    // read file as wrong user
    char readdata[FS_BLOCKSIZE];
    status = fs_readblock("user2", "/dir/file", 0, readdata);
    assert(status);
}