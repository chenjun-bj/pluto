
#include <cstdio>
#include <cstdarg>
#include <cerrno>
#include <cstring>

#include <iostream>
#include <string>
#include <numeric>

#include <sys/ipc.h>
#include <sys/shm.h>

#include "stdinclude.h"
#include "memberlist.h"

using namespace std;

int shmid = -1;
void * shmaddr = nullptr;
MemberList memlst;

void dest_shm();

void err_exit(const char* fmt, ...)
{
    va_list args;
    va_start(args, fmt);
    vprintf(fmt, args);
    va_end(args);

    dest_shm();
    exit(0);
}

#define  NODE_CNT 5
void init_shm()
{
    key_t ipckey = GetConfigPortal()->get_ipckey();
    long ring_size = GetConfigPortal()->get_ringsize();
    ring_size = NODE_CNT * 2;
    if (ring_size < KV_RING_MIN_SIZE || ring_size > KV_RING_MAX_SIZE) {
        err_exit("Invalid ring size '%d', valid range [%d, %d]\n", 
                 ring_size, KV_RING_MIN_SIZE, KV_RING_MAX_SIZE);
    }

    size_t size = MemberList::get_required_size(ring_size);

    cout << "ring size '" << ring_size << "' required size '" << size << "'" << endl;

#define SHM_MODE  0600  /* User read/write */
    shmid = shmget(ipckey, size, IPC_CREAT|IPC_EXCL|SHM_MODE);
    if (shmid == -1) {
        err_exit("shmget failed, errno=%d:%s\n",
                 errno, strerror(errno));
    }
   
    shmaddr = shmat(shmid, nullptr, 0);
    if (shmaddr == (void*)-1) {
        err_exit("shmat failed, errno=%d:%s\n",
                 errno, strerror(errno));
    }

    long *magic = (long*)shmaddr;
    *magic = PL_SHM_MAGIC;

    if (!memlst.create(shmaddr, ipckey, ring_size)) {
        err_exit("shmat failed, errno=%d:%s\n",
                 errno, strerror(errno));
    }

}

void dest_shm()
{
    if (shmaddr) {
        shmdt(shmaddr);
    }
    if (shmid != -1) {  
        shmctl(shmid, IPC_RMID, NULL);
    }
}

void traverse()
{
    cout << "------------------------------------------------------------------" << endl;
    for (auto&& it : memlst) {
        cout << "port: " << it.portnumber << ", heartbeat: " << it.heartbeat << 
                ", hash: " << it.hashcode << endl;
    }
    cout << "------------------------------------------------------------------" << endl;
}

void add_node(int cnt)
{
    int af = AF_INET;
    int addr[NODE_CNT];
    unsigned short port[NODE_CNT];
    std::iota(addr, addr+NODE_CNT, 1);
    std::iota(port, port+NODE_CNT, 10001);

    for (int i=0; i<NODE_CNT; i++) {
        memlst.add_node(af, (unsigned char*)(addr+i), port[i], -1);
    }

    traverse();

    memlst.del_node(af, (unsigned char*)(addr+1), port[1]);
    memlst.update_node_heartbeat(af, (unsigned char*)(addr+3), port[3], 11);

    traverse();
}


int main(int argc, char* argv[])
{
    if (argc < 2) {
        err_exit("Usage: %s config\n", argv[0]);
    }

    if (!GetConfigPortal()->load(argv[1])) {
        err_exit("Load config file failed\n");
    }

    cout << "sizof(struct Membership)=" << sizeof(struct Membership) << endl;
    cout << "sizof(struct st_entry)=" << sizeof(struct st_entry) << endl;
    cout << "sizof(struct MemberEntry)=" << sizeof(struct MemberEntry) << endl;
    init_shm();
    add_node(2);
    dest_shm();

    return 0;
}

