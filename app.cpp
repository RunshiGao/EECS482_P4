#include <iostream>
#include <cassert>
#include <cstdlib>
#include <string>
#include "fs_client.h"

using std::cout;
using std::string;


int main(int argc, char *argv[]) {
    char *server;
    int server_port;

    const char *writedata1 = "Wehat hts, that amopiness. -- That to secure these rights, Governments are instituted among Men, deriving their just powers from the consent of the governed, -- That whenever any Form of Government becomes destructive of these ends, it is the Right of the People to alter or to abolish it, and to institute new Government, laying its foundation on such principles and organizing its powers in such form, as to them shall seem most likely to effect their Safety and Happiness.";

    string writedata2 = "equal, that they ble Rights, that among these are Life, Liberty and the pursuit of Happiness. -- That to secure these rights, Governments are instituted among Men, deriving their just powers from the consent of the governed, -- That whenever any Form of Government becomes destructive of these ends, it is the Right of the People to alter or to abolish it, and to institute new Government, laying its foundation on such principles and organizing its powers in such form, as to them shall seem most likely to effect their Safety and HappineSS!";
    
    char readdata[FS_BLOCKSIZE];

    if (argc != 3) {
        cout << "error: usage: " << argv[0] << " <server> <serverPort>\n";
        exit(1);
    }
    server = argv[1];
    server_port = atoi(argv[2]);

    fs_clientinit(server, server_port);

    fs_create("USR", "/CS", 'd');
    fs_create("USR", "/CS/P1", 'd');


    fs_create("USR", "/CS/autotag.sh", 'f');

    fs_create("USR", "/CS/P1/cpu.h", 'f');
    fs_writeblock("USR", "/CS/P1/cpu.h", 0, writedata1);
    fs_create("USR", "/CS/P1/p1.pdf", 'f');
    for(int i = 0; i < 9; i++){
        writedata2[0] = i + '0';
        fs_writeblock("USR", "/CS/P1/p1.pdf", i, writedata2.c_str());
        fs_readblock("USR", "/CS/P1/p1.pdf", i, readdata);
        fs_writeblock("USR", "/CS/P1/cpu.h", i, readdata);
    }
    fs_delete("USR", "/CS/P1");
    fs_create("USR", "/CS/project2", 'd');

    fs_delete("USR", "/CS/P1/cpu.h");
    fs_writeblock("USR", "/CS/autotag.sh", 1, writedata2.c_str());
    fs_readblock("USR", "/CS/P1/p1.pdf", 2, readdata);

    fs_delete("USR", "/CS/P1/p1.pdf");

    fs_create("USR", "/CS/project2/p2.pdf", 'f');
    fs_delete("USR", "/CS/P1");
    fs_writeblock("USR", "/CS/project2/p1.pdf", 0, writedata2.c_str());
}
