#include <iostream>
#include <cassert>
#include <cstdlib>
#include "fs_client.h"
#include <string>

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



    cout<<"create a /2/file"<<endl;
    status = fs_create("user1", "/2/file", 'f');
    assert(status);
    cout<<"delete a /1"<<endl;
    status = fs_delete("user1", "/1");
    assert(!status);
    cout<<"delete a /2"<<endl;
    status = fs_delete("user1", "/2");
    assert(!status);
    cout<<"delete a /3"<<endl;
    status = fs_delete("user1", "/3");
    assert(!status);

    cout<<"create a /2/file"<<endl;
    status = fs_create("user1", "/2/file", 'f');
    assert(status);

    cout<<"create a /2/file2"<<endl;
    status = fs_create("user1", "/4/file2", 'f');
    assert(!status);
}