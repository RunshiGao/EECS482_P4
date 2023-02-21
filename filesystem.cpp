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

mutex free_block_m;
mutex mutex_map_m;
vector<bool> free_block_map(FS_DISKSIZE, true);
unordered_map<int, shared_mutex> mutex_map;
vector<int> lock_count(FS_DISKSIZE, 0);

// RAII for read lock
class read_lock {
public:
    read_lock(int block): index(block), isdeleted(false) {
        mutex_map_m.lock();
        if (mutex_map.find(block) == mutex_map.end()) {
            mutex_map[index];
        }
        shared_mutex &mut = mutex_map[index];
        lock_count[index]++;
        mutex_map_m.unlock();
        mut.lock_shared();
    }

    ~read_lock() {
        if(!isdeleted) {
            mutex_map_m.lock();
            lock_count[index]--;
            mutex_map[index].unlock_shared();
            if(lock_count[index] == 0) {
                mutex_map.erase(index);
            }
            mutex_map_m.unlock();
        }
    }

    void unlock() {
        mutex_map_m.lock();
        lock_count[index]--;
        mutex_map[index].unlock_shared();
        if(lock_count[index] == 0) {
            mutex_map.erase(index);
        }
        mutex_map_m.unlock();
        isdeleted = true;
    }

    void swap_lock(read_lock &other) {
        swap(index, other.index);
    }
    int index;
    bool isdeleted;
};

// RAII for write lock
class write_lock {
public:
    write_lock(int block): index(block), isdeleted(false) {
        mutex_map_m.lock();
        if (mutex_map.find(block) == mutex_map.end()) {
            mutex_map[index];
        }
        shared_mutex &mut = mutex_map[index];
        lock_count[index]++;
        mutex_map_m.unlock();
        mut.lock();
    }

    ~write_lock() {
        if(!isdeleted) {
            mutex_map_m.lock();
            lock_count[index]--;
            mutex_map[index].unlock();
            if(lock_count[index] == 0) {
                mutex_map.erase(index);
            }
            mutex_map_m.unlock();
        }
    }

    void unlock() {
        mutex_map_m.lock();
        lock_count[index]--;
        mutex_map[index].unlock();
        if(lock_count[index] == 0) {
            mutex_map.erase(index);
        }
        mutex_map_m.unlock();
        isdeleted = true;
    }

    void swap_lock(read_lock &other) {
        swap(index, other.index);
    }

    int index;
    bool isdeleted;
};


// RAII for free disk block
class free_block {
public:
    free_block(): success(false) {
        unique_lock<mutex> l(free_block_m);
        for(size_t i = 0; i < FS_DISKSIZE; i++){
            if(free_block_map[i] == true){
                free_block_map[i] = false;
                index = i;
                return;
            }
        }
        throw runtime_error("disk is full");
    }
    ~free_block() {
        if(!success) {
            free_block_m.lock();
            free_block_map[index] = true;
            free_block_m.unlock();
        }
    }
    void set_success() {
        success = true;
    }
    int index;
    bool success;
};

void init() {
    queue<int> block_queue;
    block_queue.push(0);
    free_block_map[0] = false;
    while (!block_queue.empty()) {
        int curr_block = block_queue.front();
        block_queue.pop();
        fs_inode curr_inode;
        disk_readblock(curr_block, &curr_inode);
        for (size_t i = 0; i < curr_inode.size; i++) {
            free_block_map[curr_inode.blocks[i]] = false;
        }
        if (curr_inode.type == 'd') {
            // read every direntry in 
            for (size_t i = 0; i < curr_inode.size; i++) {
                fs_direntry dir_block[FS_DIRENTRIES];
                disk_readblock(curr_inode.blocks[i], dir_block);
                for(size_t j = 0; j < FS_DIRENTRIES; j++) {
                    if (dir_block[j].inode_block != 0) {
                        free_block_map[dir_block[j].inode_block] = false;
                        block_queue.push(dir_block[j].inode_block);
                    }
                }
            }
        }
    }
}


// return index to inode block
int find_direntry(int cur, string name, string username) {
    fs_inode cur_inode;
    disk_readblock(cur,&cur_inode);
    if (cur != 0 && strcmp(cur_inode.owner, username.c_str())) {
        throw runtime_error("can not access this directory");
    }
    if(cur_inode.type == 'f'){
        throw runtime_error("it is a file instead of a directory");
    }
    for(unsigned int i = 0; i < cur_inode.size; i++){
        fs_direntry dir_block[FS_DIRENTRIES];
        disk_readblock(cur_inode.blocks[i], dir_block);
        for (unsigned int j = 0; j < FS_DIRENTRIES; j++){
            if (dir_block[j].inode_block != 0 && !strcmp(dir_block[j].name, name.c_str())){
                return dir_block[j].inode_block;
            }
        } 
    }
    throw runtime_error("cannot find this direntry");
}


// traverse down the directory
int traverse_down(string username, vector<string> &path, unsigned int len, read_lock& rl) {
    int cur = 0;
    for(unsigned int i = 0; i < len; i++) {
        cur = find_direntry(cur, path[i], username);
        read_lock next_rl(cur);
        rl.swap_lock(next_rl);
    }
    return cur;
}

// create a direntry into a directory
void add_direntry(int block, string username, string filename, string type, free_block &fb) {
    // create new inode
    fs_inode new_inode;
    new_inode.type = type[0];
    strcpy(new_inode.owner,username.c_str());
    new_inode.size = 0;
    fs_inode dir_inode;
    disk_readblock(block,&dir_inode);
    if (block != 0 && strcmp(dir_inode.owner, username.c_str())) {
        throw runtime_error("no authority to access this directory");
    }
    if(dir_inode.type == 'f'){
        throw runtime_error("it is a file instead of a directory");
    }
    fs_direntry free_dir[FS_DIRENTRIES];
    int free_dir_index = -1;
    for(size_t i = 0;i < dir_inode.size; i++){
        fs_direntry dir_block[FS_DIRENTRIES];
        disk_readblock(dir_inode.blocks[i], dir_block);
        for(size_t j = 0;j < FS_DIRENTRIES; j++){
            if(dir_block[j].inode_block == 0 && free_dir_index == -1){
                dir_block[j].inode_block = fb.index;
                strcpy(dir_block[j].name, filename.c_str());
                free_dir_index = i;
                memcpy(free_dir, dir_block, FS_BLOCKSIZE);
            } else {
                if (dir_block[j].inode_block != 0 && !strcmp(dir_block[j].name, filename.c_str())) {
                    throw runtime_error("file already existed");
                }
            }
        }
    }
    if(free_dir_index == -1) {
        // check that whether disk blocks is full
        if(dir_inode.size == FS_MAXFILEBLOCKS) {
            throw runtime_error("reach to the maximum file blocks");
        }
        free_block free_dir_block;
        disk_writeblock(fb.index, &new_inode);
        // add a new block for the directory
        fs_direntry dir_block[FS_DIRENTRIES];
        memset(dir_block, 0, FS_BLOCKSIZE);
        dir_block[0].inode_block = fb.index;
        strcpy(dir_block[0].name, filename.c_str());
        dir_inode.blocks[dir_inode.size] = free_dir_block.index;
        dir_inode.size++;
        disk_writeblock(free_dir_block.index, dir_block);
        disk_writeblock(block, &dir_inode);
        free_dir_block.set_success();
    } else {
        disk_writeblock(fb.index,&new_inode);
        disk_writeblock(dir_inode.blocks[free_dir_index], free_dir);
    }
    fb.set_success();
}

void fs_create(string username, vector<string> &path, string type) {
    // first try to get the free block
    // if disk is full, we do not need to traverse down
    free_block fb;
    int num = path.size();
    string filename = path[num-1];
    if(num == 1) {
        write_lock wl(0);
        add_direntry(0, username, filename, type, fb);
    } else {
        read_lock rl(0);
        int cur = traverse_down(username, path, num-2, rl);
        cur = find_direntry(cur, path[num-2], username);
        write_lock wl(cur);
        rl.unlock();
        add_direntry(cur, username, filename, type, fb);
    }
}

void fs_read_block(int sockfd, string username, vector<string> &path, int block, string command_str) {
    read_lock rl(0);
    int cur = traverse_down(username, path, path.size(), rl);
    fs_inode res_inode;
    disk_readblock(cur, &res_inode);
    if (res_inode.type != 'f') {
        throw runtime_error("it is a dir instead of a file");
    }
    if (strcmp(res_inode.owner, username.c_str())) {
        throw runtime_error("no authority to access this directory");
    }
    if (block >= (int)res_inode.size) {
        throw runtime_error("read exceeds the size of the file");
    }
    uint32_t index = res_inode.blocks[block];
    char msg[FS_BLOCKSIZE];
    disk_readblock(index, msg);
    rl.unlock();
    char response[command_str.size() + FS_BLOCKSIZE + 1];
    for(unsigned int i = 0; i < command_str.size(); i++) {
        response[i] = command_str[i];
    }
    response[command_str.size()] = '\0';
    for(unsigned int i = 0; i < FS_BLOCKSIZE; i++) {
        response[i+command_str.size()+1] = msg[i];
    }
    send(sockfd, response, command_str.size() + FS_BLOCKSIZE + 1, MSG_NOSIGNAL);
}

void fs_write_block(string username, vector<string> &path, int block, string data) {
    string filename = path.back();
    read_lock rl(0);
    int cur = traverse_down(username, path, path.size()-1, rl);
    cur = find_direntry(cur, filename, username);
    write_lock wl(cur);
    rl.unlock();
    fs_inode res_inode;
    disk_readblock(cur, &res_inode);
    if (res_inode.type != 'f') {
        throw runtime_error("it is a dir instead of a file");
    }
    if (strcmp(res_inode.owner, username.c_str())) {
        throw runtime_error("no authority to access this directory");
    }
    bool write_inode = false;
    if (block > (int)res_inode.size) {
        // exceeds the file size
        throw runtime_error("Can not write the block that exceeds the file size");
    } else if (block == (int)res_inode.size) {
        free_block fb;
        res_inode.blocks[block] = fb.index;
        res_inode.size++;
        write_inode = true;
        fb.set_success();
    }
    char buffer[FS_BLOCKSIZE];
    for(unsigned int i = 0; i < FS_BLOCKSIZE; i++) {
        buffer[i] = data[i];
    }
    int block_index = res_inode.blocks[block];
    disk_writeblock(block_index, buffer);
    if(write_inode) {
        disk_writeblock(cur, &res_inode);
    }
}

void delete_parent_child(int parent, string child_name, string username) {
    fs_inode parent_inode;
    disk_readblock(parent, &parent_inode);
    if (parent != 0 && strcmp(parent_inode.owner, username.c_str())) {
        throw runtime_error("no authority to access this directory");
    }
    if (parent_inode.type != 'd') {
        throw runtime_error("parent should be a direntry");
    }
    int edit_index = -1;
    int child_block_index = -1;
    fs_direntry edit_dir[FS_DIRENTRIES];
    for (size_t i = 0; i < parent_inode.size; i++){
        fs_direntry dir_block[FS_DIRENTRIES];
        disk_readblock(parent_inode.blocks[i], dir_block);
        for (size_t j = 0; j < FS_DIRENTRIES; j++) {
            if (dir_block[j].inode_block != 0 && !strcmp(dir_block[j].name, child_name.c_str())) {
                child_block_index = dir_block[j].inode_block;
                dir_block[j].inode_block = 0;
                edit_index = i;
                memcpy(edit_dir, dir_block, FS_BLOCKSIZE);
                break;
            }
        }
        if (edit_index != -1) {
            break;
        }
    }
    if (edit_index == -1) {
        throw runtime_error("Can not find child file");
    }
    write_lock wl(child_block_index);
    fs_inode child_inode;
    disk_readblock(child_block_index, &child_inode);
    if (child_inode.type == 'd' && child_inode.size != 0) {
        throw runtime_error("the directory you are trying to delete is not empty");
    }
    if (strcmp(child_inode.owner, username.c_str())) {
        throw runtime_error("no authority to access this directory");
    }
    bool all_zero = true;
    // check whether we need to shrink
    for (size_t k = 0; k < FS_DIRENTRIES; k++) {
        if (edit_dir[k].inode_block != 0) {
            all_zero = false;
            break;
        }
    }
    if (all_zero) {
        // need to shrink
        int ready2free = parent_inode.blocks[edit_index];
        for (size_t k = edit_index; k < parent_inode.size - 1; k++) {
            parent_inode.blocks[k] = parent_inode.blocks[k + 1];
        }
        parent_inode.size--;
        disk_writeblock(parent, &parent_inode);
        free_block_m.lock();
        free_block_map[ready2free] = true;
        free_block_m.unlock();
    } else {
        disk_writeblock(parent_inode.blocks[edit_index], &edit_dir);
    }
    free_block_m.lock();
    free_block_map[child_block_index] = true;
    if (child_inode.type == 'f') {
        // for every block in it, delete it
        for (size_t i = 0; i < child_inode.size; i++) {
            // each block should point to a file block
            free_block_map[child_inode.blocks[i]] = true;
        }
    }
    free_block_m.unlock();
}

void fs_delete(string username, vector<string> &path) {
    if (path.size() == 1) {
        write_lock wl(0);
        delete_parent_child(0, path[0], username);
    } else {
        read_lock rl(0);
        int cur = traverse_down(username, path, path.size()-2, rl);
        int parent = find_direntry(cur, path[path.size()-2], username);
        write_lock wl(parent);
        rl.unlock();
        delete_parent_child(parent, path[path.size()-1], username);
    }
}