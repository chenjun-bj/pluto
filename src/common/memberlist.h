/**
 *******************************************************************************
 * memberlist.h                                                                *
 *                                                                             *
 * Member list                                                                 *
 *   -- Shared memory layout                                                   *
 *******************************************************************************
 */

#ifndef _MEMBER_LIST_H_
#define _MEMBER_LIST_H_

/*
 *******************************************************************************
 *  Headers                                                                    *
 *******************************************************************************
 */
#include <unistd.h>
#include <sys/ipc.h>
#include <pthread.h>

#include "pltypes.h"
/*
 *******************************************************************************
 *  Macros                                                                     *
 *******************************************************************************
 */

#define PL_SHM_MAGIC    0x504C55544F303031l

#define PL_RUNNING_ST_RUN    1
#define PL_RUNNING_ST_STOP   0
 
/*
 *******************************************************************************
 *  Forward declaraction                                                       *
 *******************************************************************************
 */


/*
 *******************************************************************************
 *  Class declaraction                                                         *
 *******************************************************************************
 */

/**
   Shared memory layout :
    +--------+--------+--------+--------+--------+--------+--------+--------+
    |        8        16       24       32       40       48       56       |
    +--------+--------+--------+--------+--------+--------+--------+--------+
    |                              MAGIC NUMBER                             |
    +--------+--------+--------+--------+--------+--------+--------+--------+
    |              IPCKEY               |         Running status            |
    +--------+--------+--------+--------+--------+--------+--------+--------+
    |  PID of memebership process       |  PID of store process             |
    +--------+--------+--------+--------+--------+--------+--------+--------+
    |                                 LOCK                                  |
    +--------+--------+--------+--------+--------+--------+--------+--------+
    |                                 LOCK                                  |
    +--------+--------+--------+--------+--------+--------+--------+--------+
    |                                 LOCK                                  |
    +--------+--------+--------+--------+--------+--------+--------+--------+
    |                                 LOCK                                  |
    +--------+--------+--------+--------+--------+--------+--------+--------+
    |                                 LOCK                                  |
    +--------+--------+--------+--------+--------+--------+--------+--------+
    |                              RING  LENGTH                             |
    +--------+--------+--------+--------+--------+--------+--------+--------+
    |     Addr1 address family          |             Addr1 type            |
    +--------+--------+--------+--------+--------+--------+--------+--------+
    |                       Default  join address 1                         |
    +--------+--------+--------+--------+--------+--------+--------+--------+
    |                       Default  join address 1                         |
    +--------+--------+--------+--------+--------+--------+--------+--------+
    | Addr1 portnumber|    Addr2 flag   |             Reserved              |
    +--------+--------+--------+--------+--------+--------+--------+--------+
    |     Addr2 address family          |             Addr2 type            |
    +--------+--------+--------+--------+--------+--------+--------+--------+
    |                       Default  join address 2                         |
    +--------+--------+--------+--------+--------+--------+--------+--------+
    |                       Default  join address 2                         |
    +--------+--------+--------+--------+--------+--------+--------+--------+
    | Addr2 portnumber|                     Reserved                        |
    +--------+--------+--------+--------+--------+--------+--------+--------+
    |                              Member size                              |
    +--------+--------+--------+--------+--------+--------+--------+--------+
    |                             Member Entry  1                           |
    |                                ... ...                                |
    +--------+--------+--------+--------+--------+--------+--------+--------+
    |                                ... ...                                |
    +--------+--------+--------+--------+--------+--------+--------+--------+
  
   Member entry layout: 
    +--------+--------+--------+--------+--------+--------+--------+--------+
    |        8        16       24       32       40       48       56       |
    +--------+--------+--------+--------+--------+--------+--------+--------+
    |                                  ID                                   |
    +--------+--------+--------+--------+--------+--------+--------+--------+
    |         Address family            |           Address type            |
    +--------+--------+--------+--------+--------+--------+--------+--------+
    |                                Address                                |
    +--------+--------+--------+--------+--------+--------+--------+--------+
    |                                Address                                |
    +--------+--------+--------+--------+--------+--------+--------+--------+
    |   Portnumber    |                       Reserved                      |
    +--------+--------+--------+--------+--------+--------+--------+--------+
    |                               HashCode                                |
    +--------+--------+--------+--------+--------+--------+--------+--------+
 */

struct MemberEntry {
    uint64  id;
    uint32  af;
    uint32  type;
    uint8   address[16];
    uint16  portnumber;
    uint16  reserved1;
    uint32  reserved2;
    uint64  hashcode;
};

struct Membership {
    uint64  magic_number;
    key_t   ipckey;
    uint32  running_status;
    pid_t   pid_member;
    pid_t   pid_store;
    pthread_mutex_t mutex;
    uint64  ring_size;
    uint32  join_add1_af;   // AF_INET or AF_INET6
    uint32  join_add1_type; // SOCK_STREAM or SOCK_DGRAM
    uint8   join_addr1[16];
    uint16  join_addr1_port;
    uint16  join_addr2_exist;
    uint32  reserved1;
    uint32  join_add2_af;   
    uint32  join_add2_type; 
    uint8   join_addr2[16];
    uint16  join_addr2_port;
    uint16  reserved2;
    uint32  reserved3;
    uint64  member_cnt;
    struct MemberEntry members[1]; // members[0] is ourselves
};

class MemberList {
public:
    MemberList(void* addr);
    ~MemberList();

    bool attach(void* addr);
    void detach();

    uint64 get_magic_number() const;

    bool set_ipckey(key_t);
    key_t  get_ipckey() const;

    bool set_running_status(uint32 status);
    uint32 get_running_status() const;

    bool set_pid_member(pid_t pid);
    pid_t  get_pid_member() const;

    bool set_pid_store(pid_t pid);
    pid_t  get_pid_store()  const;

    bool set_ring_size(uint64 size);
    uint64  get_ring_size()  const;

    pthread_mutex_t* get_mutex() const;

private:
    struct Membership * m_paddr;
};

/*
 *******************************************************************************
 *  Inline functions                                                           *
 *******************************************************************************
 */


#endif // _MEMBER_LIST_H

