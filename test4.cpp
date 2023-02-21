// test4 stress test
#include <iostream>
#include <cassert>
#include <cstdlib>
#include "fs_client.h"
#include <string>

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

    for (int i = 0; i < 10; i++) {
        std::string folder_name = "/" + std::to_string(i);
        status = fs_create("user1", folder_name.c_str(), 'd');
        assert(!status);
        for (int j = 0; j < 10; j++) {
            std::string file_name = folder_name + "/" + std::to_string(j);
            status = fs_create("user1", file_name.c_str(), 'f');
            assert(!status);
        }
    }

    for (int i = 0; i < 10; i++) {
        std::string folder_name = "/" + std::to_string(i);
        for (int j = 0; j < 10; j++) {
            std::string file_name = folder_name + "/" + std::to_string(j);
            status = fs_delete("user1", file_name.c_str());
            assert(!status);
        }
        status = fs_delete("user1", folder_name.c_str());
        assert(!status);
    }
}