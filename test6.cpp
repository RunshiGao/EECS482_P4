// test6 continue tests some error handling
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

    // create a file into a file
    status = fs_create("user1", "/apple/file", 'f');
    status = fs_create("user1", "/apple/file/file2", 'f');
    assert(status);

    // read a file that does not exist
    char readdata[FS_BLOCKSIZE];
    status = fs_readblock("user1", "/apple/file3", 0, readdata);
    assert(status);

    // try to read a dir
    status = fs_readblock("user1", "/apple", 0, readdata);
    assert(status);

    // normal write
    const char *writedata = "We hold these truths to be self-evident, that all men are created equal, that they are endowed by their Creator with certain unalienable Rights, that among these are Life, Liberty and the pursuit of Happiness. -- That to secure these rights, Governments are instituted among Men, deriving their just powers from the consent of the governed, -- That whenever any Form of Government becomes destructive of these ends, it is the Right of the People to alter or to abolish it, and to institute new Government, laying its foundation on such principles and organizing its powers in such form, as to them shall seem most likely to effect their Safety and Happiness.";
    status = fs_writeblock("user1", "/apple/file", 0, writedata);
    assert(!status);

    // read exceeds the size of the file
    status = fs_readblock("user1", "/apple/file", 1, readdata);
    assert(status);

    // normal write
    status = fs_writeblock("user1", "/apple/file", 1, writedata);
    assert(!status);

    // write the block that exceeds the file size
    status = fs_writeblock("user1", "/apple/file", 3, writedata);
    assert(status);

    // delete not exist file
    status = fs_delete("user1", "/apple/file4");
    assert(status);

    // delete no empty directory
    status = fs_delete("user1", "/apple");
    assert(status);

    // write to a directory
    status = fs_writeblock("user1", "/apple", 0, writedata);
    assert(status);

    // have no right to read a dir
    status = fs_writeblock("-", "/apple/file", 0, writedata);
    assert(status);

    // have no right to delete a dir
    status = fs_delete("-", "/apple/file");
    assert(status);

    status = fs_delete("user1", "/apple/file");
    assert(!status);

    status = fs_delete("user1", "/apple/file");
    assert(status);

    // write the block that is deleted
    status = fs_writeblock("user1", "/apple/file", 0, writedata);
    assert(status);

    status = fs_delete("user2", "/apple");
    assert(status);

    status = fs_delete("user1", "/apple");
    assert(!status);

    status = fs_create("u", "/apple", 'd');
    assert(!status);

    status = fs_create("u", "/apple/pear", 'd');
    assert(!status);

    status = fs_create("u", "/apple/pear/1", 'f');
    assert(!status);

    status = fs_create("u", "/apple/pear/2", 'f');
    assert(!status);

    status = fs_create("9", "/apple/pear/3", 'f');
    assert(status);

    status = fs_create("u", "/apple/pear/3", 'f');
    assert(!status);

    status = fs_create("u", "/apple/pear/4", 'f');
    assert(!status);

    status = fs_create("u", "/apple/pear/5", 'f');
    assert(!status);

    status = fs_create("u", "/apple/pear/6", 'f');
    assert(!status);

    status = fs_create("u", "/apple/pear/7", 'f');
    assert(!status);

    status = fs_create("u", "/apple/pear/8", 'f');
    assert(!status);

    status = fs_create("u", "/apple/pear/9", 'f');
    assert(!status);

    status = fs_delete("u1", "/apple/pear/3");
    assert(status);

    status = fs_create("u", "/apple/pear/new3", 'f');
    assert(!status);

    status = fs_delete("u", "/apple/pear/3");
    assert(!status);

    status = fs_create("u", "/apple/pear/3", 'f');
    assert(!status);

    status = fs_create("u", "/apple/pear/2", 'd');
    assert(status);

    status = fs_create("u", "/apple/pear/pear", 'd');
    assert(!status);

    status = fs_delete("u1", "/apple/pear/pear");
    assert(status);

    status = fs_writeblock("user1", "/apple/pear/pear", 0, writedata);
    assert(status);

    status = fs_delete("u", "/apple/pear/pear");
    assert(!status);

    status = fs_create("u", "/apple/pear/pear", 'f');
    assert(!status);

    status = fs_writeblock("u", "/apple/pear/pear", 0, writedata);
    assert(!status);

}