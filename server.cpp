#include "fs_server.h"
#include <unistd.h>
#include <sys/socket.h>
#include <iostream>
#include <sstream>
#include <sys/types.h>
#include <netinet/in.h>
#include <unordered_map>
#include <cstring>
#include <vector>
#include <queue>
#include <shared_mutex>
#include <cassert>
using namespace std;

extern void init();
extern void fs_create(string username, vector<string> &path, string type);
extern void fs_read_block(int sockfd, string username, vector<string> &path, int block, string command_str);
extern void fs_write_block(string username, vector<string> &path, int block, string data);
extern void fs_delete(string username, vector<string> &path);

void make_server_sockaddr(struct sockaddr_in *addr, int port) {
	addr->sin_family = AF_INET;
	addr->sin_addr.s_addr = INADDR_ANY;
	addr->sin_port = htons(port);
}

int get_port_number(int sockfd) {
 	struct sockaddr_in addr;
	socklen_t length = sizeof(addr);
	if (getsockname(sockfd, (sockaddr *) &addr, &length) == -1) {
		throw runtime_error("Error getting port of socket");
	}
	return ntohs(addr.sin_port);
}

// get block number
int get_number(string str) {
    if(str.size() > 9) {
        throw runtime_error("too large number");
    }
    for (char const &c : str) {
        if (std::isdigit(c) == 0) {
            throw runtime_error("incorrect number");
        }
    }
    int number = stoi(str);
    if (number >= (int)FS_MAXFILEBLOCKS) {
        throw runtime_error("exceeds the maximum # of data blocks in a file or directory");
    }
    return number;
}

// read command from socket
pair<string, int> read_command(int connectionfd) {
    string data;
    char buf[64];
    bool find = false;
    int index = 0;
    int n = 0;
    do {
        n = recv(connectionfd, buf, sizeof(buf), 0);
        if(n <= 0) {
            break;
        }
        data += string(buf,n);
        index = 0;
        for(auto c:data) {
            if (c == '\0') {
                find = true;
                break;
            }
            index++;
        }
    } while (!find && data.size() < FS_MAXPATHNAME + FS_MAXUSERNAME + 30);
    if (!find) {
        throw runtime_error("no none character");
    }
    return {data, index};
}

// handle one connection per thread
void handle_connection(void* conn) {
    int *conn_ptr = (int*)conn;
    int connectionfd = *conn_ptr;
    delete conn_ptr;
    try {
        auto res = read_command(connectionfd);
        string data = res.first;
        int index = res.second;
        string command_str = data.substr(0,index);
        istringstream ss(command_str);
        vector<string> commands;
        string s;
        while(ss >> s) {
            commands.push_back(s);
        }
        if (commands.size() < 3) {
            throw runtime_error("incorrect argument number");
        }
        string command = commands[0];
        string username = commands[1];
        string pathname = commands[2];
        if (username.size() > FS_MAXUSERNAME) {
            throw runtime_error("username too long");
        }
        if (pathname.size() > FS_MAXPATHNAME) {
            throw runtime_error("pathname too long");
        }
        if (pathname[0] != '/') {
            throw runtime_error("incorrect pathname");
        }
        if (pathname[pathname.size()-1] == '/') {
            throw runtime_error("pathname ended with /");
        }
        string l;
        pathname = pathname.substr(1);
        vector<string> path;
        istringstream path_ss(pathname);
        while(getline(path_ss,l,'/')) {
            if(l.size()==0 || l.size() > FS_MAXFILENAME) {
                throw runtime_error("too long filename in pathname or empty filename");
            }
            path.push_back(l);
        }
        if (command == "FS_READBLOCK") {
            if (commands.size() != 4) {
                throw runtime_error("incorrect argument number for read block");
            }
            int block = get_number(commands[3]);
            string obj = string("FS_READBLOCK") + " " + commands[1] + " " + commands[2] + " " + to_string(block);
            if (obj != command_str) {
                throw runtime_error("incorrect read format");
            }
            fs_read_block(connectionfd, username, path, block, command_str);
        } else if (command == "FS_WRITEBLOCK") {
            if (commands.size() != 4) {
                throw runtime_error("incorrect argument number for write block");
            }
            int block = get_number(commands[3]);
            string block_data = data.substr(index+1);
            int remain_size = FS_BLOCKSIZE - block_data.size();
            if (remain_size != 0) {
                char buf[remain_size];
                int ret = recv(connectionfd, buf, remain_size, MSG_WAITALL);
                if(ret != remain_size) {
                    throw runtime_error("Can not receive enough bytes");
                }
                block_data += string(buf,remain_size);
            }
            string obj = string("FS_WRITEBLOCK") + " " + commands[1] + " " + commands[2] + " " + to_string(block);
            if (obj != command_str) {
                throw runtime_error("incorrect write format");
            }
            fs_write_block(username, path, block, block_data);
            send(connectionfd, command_str.c_str(), command_str.size()+1, MSG_NOSIGNAL);
        } else if (command == "FS_CREATE") {
            if (commands.size() != 4) {
                throw runtime_error("incorrect argument number for create");
            }
            string type = commands[3];
            if (type != "d" && type != "f") {
                throw runtime_error("incorrect file type");
            }
            string obj = string("FS_CREATE") + " " + commands[1] + " " + commands[2] + " " + type;
            if (obj != command_str) {
                throw runtime_error("incorrect create format");
            }
            fs_create(username, path, type);
            send(connectionfd, command_str.c_str(), command_str.size()+1, MSG_NOSIGNAL);
        } else if (command == "FS_DELETE") {
            if (commands.size() != 3) {
                throw runtime_error("incorrect argument number for delete");
            }
            string obj = string("FS_DELETE") + " " + commands[1] + " " + commands[2];
            if (obj != command_str) {
                throw runtime_error("incorrect delete format");
            }
            fs_delete(username, path);
            send(connectionfd, command_str.c_str(), command_str.size()+1, MSG_NOSIGNAL);
        } else {
            throw runtime_error("invalid command");
        }
    } catch (runtime_error & e) {
        cout_lock.lock();
        cout<<e.what()<<endl;
        cout_lock.unlock();
    } 
    close(connectionfd);
}

void run_server(int port) {
    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd == -1) {
		throw runtime_error("Error opening stream socket");
	}
    int yesval = 1;
    if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &yesval, sizeof(yesval)) == -1) {
        throw runtime_error("Error setting socket options");
    }
    struct sockaddr_in addr;
	make_server_sockaddr(&addr, port);
    if (bind(sockfd, (sockaddr *) &addr, sizeof(addr)) == -1) {
		throw runtime_error("Error binding stream socket");
	}
    listen(sockfd, 30);
    port = get_port_number(sockfd);
    std::cout << "\n@@@ port " << port << std::endl;
    while (true) {
		int connectionfd = accept(sockfd, 0, 0);
		if (connectionfd == -1) {
			throw runtime_error("Error accepting connection");
		}
        int *conn = new int(connectionfd);
        thread handler(handle_connection, (void*)conn);
        handler.detach();
	}
}

int main(int argc, char** argv){
    int port_number = 0;
    if (argc > 1) {
        port_number  = atoi(argv[1]);
    }
    init();
    run_server(port_number);
    return 0;
}